# !/usr/bin/env python3
# coding: utf-8

# Copyright 2026 The MLPerf Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Converts quantized INT8/UINT8 TFLite models to Core ML W8A8 .mlpackage."""

import re
import coremltools as ct
from coremltools.converters.mil import Builder as mb
from coremltools.converters.mil.mil import Function, Program, types
import flatbuffers.flexbuffers as flexbuffers
import numpy as np
from tensorflow.lite.python import schema_py_generated as schema

# TFLite TensorType enum
_TFLITE_DTYPE = {
  schema.TensorType.FLOAT32: np.float32,
  schema.TensorType.FLOAT16: np.float16,
  schema.TensorType.INT32: np.int32,
  schema.TensorType.UINT8: np.uint8,
  schema.TensorType.INT64: np.int64,
  schema.TensorType.BOOL: np.bool_,
  schema.TensorType.INT16: np.int16,
  schema.TensorType.INT8: np.int8,
}

_BUILTIN_OP_NAMES = {
  getattr(schema.BuiltinOperator, k): k
  for k in dir(schema.BuiltinOperator)
  if not k.startswith('_')
}


def _sanitize_name(name: str) -> str:
  s = re.sub(r'[^a-zA-Z0-9_]', '_', name)
  if not s or s[0].isdigit():
    s = 't_' + s
  return s


def _get_tensor_quant(tensor):
  q = tensor.Quantization()
  if q is None or q.ScaleLength() == 0:
    return None
  scales = np.array([q.Scale(i) for i in range(q.ScaleLength())], dtype=np.float32)
  zps = np.array([q.ZeroPoint(i) for i in range(q.ZeroPointLength())], dtype=np.int64)
  if len(zps) == 0:
    zps = np.zeros_like(scales, dtype=np.int64)
  if tensor.Type() == schema.TensorType.UINT8:
    zps = zps - 128
  return {
    'scales': scales,
    'zps': zps,
    'qdim': q.QuantizedDimension(),
  }


def _get_buffer_array(model, tensor):
  buf = model.Buffers(tensor.Buffer())
  if buf is None or buf.DataLength() == 0:
    return None
  raw = buf.DataAsNumpy()
  dtype = _TFLITE_DTYPE[tensor.Type()]
  arr = np.frombuffer(raw.tobytes(), dtype=dtype)
  shape = [tensor.Shape(i) for i in range(tensor.ShapeLength())]
  return arr.reshape(shape) if shape else arr.reshape(())


def _apply_activation(x, act_type):
  if act_type == schema.ActivationFunctionType.NONE:
    return x
  if act_type == schema.ActivationFunctionType.RELU:
    return mb.relu(x=x)
  if act_type == schema.ActivationFunctionType.RELU6:
    return mb.clip(x=x, alpha=np.float32(0.0), beta=np.float32(6.0))
  if act_type == schema.ActivationFunctionType.RELU_N1_TO_1:
    return mb.clip(x=x, alpha=np.float32(-1.0), beta=np.float32(1.0))
  if act_type == schema.ActivationFunctionType.TANH:
    return mb.tanh(x=x)
  raise NotImplementedError(f'Unsupported fused activation: {act_type}')


def _nhwc_axis_to_nchw(axis: int, rank: int = 4) -> int:
  if axis < 0:
    axis += rank
  if rank != 4:
    return axis
  # NHWC (0, 1, 2, 3) -> NCHW (0, 2, 3, 1)
  return {0: 0, 1: 2, 2: 3, 3: 1}[axis]


def convert_tflite_to_mlpackage(
  tflite_path: str,
  export_path: str,
  quantize_activations: bool = True,
  symmetric: bool = False,
  compute_precision=ct.precision.FLOAT16,
  minimum_deployment_target=ct.target.iOS18,
  float_io: bool = True,
  short_description: str = '',
):
  """Converts a quantized TFLite flatbuffer to a Core ML W8/W8A8 .mlpackage."""
  with open(tflite_path, 'rb') as f:
    buf = f.read()
  model = schema.Model.GetRootAsModel(buf, 0)
  sg = model.Subgraphs(0)

  op_codes = {}
  for i in range(model.OperatorCodesLength()):
    oc = model.OperatorCodes(i)
    code = max(oc.BuiltinCode(), oc.DeprecatedBuiltinCode())
    custom = oc.CustomCode().decode('utf-8') if oc.CustomCode() else ''
    op_codes[i] = custom if code == schema.BuiltinOperator.CUSTOM else _BUILTIN_OP_NAMES.get(code, str(code))

  num_tensors = sg.TensorsLength()
  tensors = [sg.Tensors(i) for i in range(num_tensors)]
  shapes = [[t.Shape(j) for j in range(t.ShapeLength())] for t in tensors]
  quants = [_get_tensor_quant(t) for t in tensors]
  buffers = [_get_buffer_array(model, t) for t in tensors]

  sg_inputs = [sg.Inputs(i) for i in range(sg.InputsLength())]
  sg_outputs = [sg.Outputs(i) for i in range(sg.OutputsLength())]

  input_placeholders = {}
  input_names = []
  for idx in sg_inputs:
    t = tensors[idx]
    name = _sanitize_name(t.Name().decode('utf-8'))
    input_names.append(name)
    t_dtype = _TFLITE_DTYPE[t.Type()]
    if float_io and t_dtype in (np.int8, np.uint8):
      mil_dtype = types.fp32
    elif t_dtype in (np.int32, np.int64):
      mil_dtype = types.int32
    else:
      mil_dtype = types.fp32
    input_placeholders[name] = mb.placeholder(shape=tuple(shapes[idx]), dtype=mil_dtype)

  env = {}
  layout = {}  # 'NCHW' or 'NHWC'

  def qdq(var, tensor_idx):
    if not quantize_activations:
      return var
    q = quants[tensor_idx]
    t_type = tensors[tensor_idx].Type()
    if q is None or t_type not in (schema.TensorType.INT8, schema.TensorType.UINT8):
      return var
    if len(q['scales']) != 1:
      return var
    scale = float(q['scales'][0])
    zp = int(np.clip(q['zps'][0], -128, 127))
    if symmetric and zp != 0:
      min_val = (-128 - zp) * scale
      max_val = (127 - zp) * scale
      abs_max = max(abs(min_val), abs(max_val))
      scale = max(abs_max / 127.0, 1e-12)
      zp = 0
    scale_val = np.float32(scale)
    if symmetric:
      qv = mb.quantize(input=var, scale=scale_val, output_dtype='int8')
      return mb.dequantize(input=qv, scale=scale_val)
    zp_val = np.int8(zp)
    qv = mb.quantize(input=var, scale=scale_val, zero_point=zp_val, output_dtype='int8')
    return mb.dequantize(input=qv, scale=scale_val, zero_point=zp_val)

  def get_const_fp32(tensor_idx):
    arr = buffers[tensor_idx]
    q = quants[tensor_idx]
    t_type = tensors[tensor_idx].Type()
    if t_type == schema.TensorType.UINT8:
      arr = (arr.astype(np.int16) - 128).astype(np.int8)
    if q is not None and t_type in (schema.TensorType.INT8, schema.TensorType.UINT8):
      scales = q['scales']
      zps = q['zps']
      if len(scales) == 1:
        return (arr.astype(np.float32) - float(zps[0])) * float(scales[0])
      qdim = q['qdim']
      b_shape = [1] * arr.ndim
      b_shape[qdim] = len(scales)
      return (arr.astype(np.float32) - zps.reshape(b_shape)) * scales.reshape(b_shape)
    if q is not None and t_type == schema.TensorType.INT32:
      scales = q['scales']
      if len(scales) == 1:
        return arr.astype(np.float32) * float(scales[0])
      return arr.astype(np.float32) * scales.reshape(arr.shape)
    return arr.astype(np.float32)

  def get_quant_weight(tensor_idx, perm=None, new_qdim=None):
    """Returns a MIL constexpr_affine_dequantize weight var (or float32 const)."""
    arr = buffers[tensor_idx]
    q = quants[tensor_idx]
    t_type = tensors[tensor_idx].Type()
    if perm is not None:
      arr = np.transpose(arr, perm)
    if q is not None and t_type in (schema.TensorType.INT8, schema.TensorType.UINT8):
      if t_type == schema.TensorType.UINT8:
        arr = (arr.astype(np.int16) - 128).astype(np.int8)
      else:
        arr = arr.astype(np.int8)
      scales = q['scales'].astype(np.float32)
      zps = np.clip(q['zps'], -128, 127).astype(np.int8)
      if symmetric and not np.all(zps == 0):
        # Re-quantize asymmetric weights symmetrically around zero_point=0
        if len(scales) == 1:
          w_fp32 = (arr.astype(np.float32) - float(zps[0])) * float(scales[0])
          abs_max = max(float(np.max(np.abs(w_fp32))), 1e-12)
          scales = np.array([abs_max / 127.0], dtype=np.float32)
          arr = np.clip(np.round(w_fp32 / scales[0]), -127, 127).astype(np.int8)
          zps = np.array([0], dtype=np.int8)
        else:
          axis = new_qdim if new_qdim is not None else q['qdim']
          b_shape = [1] * arr.ndim
          b_shape[axis] = len(scales)
          w_fp32 = (arr.astype(np.float32) - zps.reshape(b_shape).astype(np.float32)) * scales.reshape(b_shape)
          reduce_axes = tuple(i for i in range(arr.ndim) if i != axis)
          abs_max = np.maximum(np.max(np.abs(w_fp32), axis=reduce_axes), 1e-12)
          scales = (abs_max / 127.0).astype(np.float32)
          arr = np.clip(np.round(w_fp32 / scales.reshape(b_shape)), -127, 127).astype(np.int8)
          zps = np.zeros_like(zps, dtype=np.int8)
      if len(scales) == 1:
        if symmetric:
          return mb.constexpr_affine_dequantize(
            quantized_data=arr,
            zero_point=np.int8(0),
            scale=np.float32(scales[0]),
            axis=0,
          )
        return mb.constexpr_affine_dequantize(
          quantized_data=arr,
          zero_point=np.int8(zps[0]),
          scale=np.float32(scales[0]),
          axis=0,
        )
      axis = new_qdim if new_qdim is not None else q['qdim']
      zp_arg = np.int8(zps[0]) if np.all(zps == zps[0]) else zps
      return mb.constexpr_affine_dequantize(
        quantized_data=arr,
        zero_point=zp_arg,
        scale=scales,
        axis=axis,
      )
    return arr.astype(np.float32)

  def get_var(tensor_idx, target_layout=None):
    if tensor_idx not in env:
      if buffers[tensor_idx] is not None:
        arr = buffers[tensor_idx]
        t_type = tensors[tensor_idx].Type()
        if t_type in (schema.TensorType.INT32, schema.TensorType.INT64) and quants[tensor_idx] is None:
          env[tensor_idx] = arr.astype(np.int32)
          layout[tensor_idx] = 'OTHER'
        else:
          env[tensor_idx] = get_const_fp32(tensor_idx)
          layout[tensor_idx] = 'NHWC' if len(shapes[tensor_idx]) == 4 else 'OTHER'
      else:
        raise KeyError(f'Tensor {tensor_idx} not yet defined in env')
    v = env[tensor_idx]
    cur = layout.get(tensor_idx, 'OTHER')
    if target_layout == 'NCHW' and cur == 'NHWC':
      v = mb.transpose(x=v, perm=[0, 3, 1, 2])
    elif target_layout == 'NHWC' and cur == 'NCHW':
      v = mb.transpose(x=v, perm=[0, 2, 3, 1])
    return v

  with Function(input_placeholders, opset_version=minimum_deployment_target) as func:
    for i, idx in enumerate(sg_inputs):
      v = func.inputs[input_names[i]]
      if not symmetric and tensors[idx].Type() in (schema.TensorType.INT8, schema.TensorType.UINT8):
        v = qdq(v, idx)
      env[idx] = v
      layout[idx] = 'NHWC' if len(shapes[idx]) == 4 else 'OTHER'

    for op_idx in range(sg.OperatorsLength()):
      op = sg.Operators(op_idx)
      op_name = op_codes[op.OpcodeIndex()]
      ins = [op.Inputs(j) for j in range(op.InputsLength())]
      outs = [op.Outputs(j) for j in range(op.OutputsLength())]

      if op_name == 'CONV_2D':
        opt = schema.Conv2DOptions()
        opt.Init(op.BuiltinOptions().Bytes, op.BuiltinOptions().Pos)
        x = get_var(ins[0], 'NCHW')
        if symmetric and ins[0] in sg_inputs:
          x = qdq(x, ins[0])
        w = get_quant_weight(ins[1], perm=[0, 3, 1, 2], new_qdim=0)
        b = get_const_fp32(ins[2]) if len(ins) > 2 and ins[2] >= 0 else None
        pad = 'same' if opt.Padding() == schema.Padding.SAME else 'valid'
        y = mb.conv(
          x=x,
          weight=w,
          bias=b,
          strides=[opt.StrideH(), opt.StrideW()],
          pad_type=pad,
          dilations=[max(1, opt.DilationHFactor()), max(1, opt.DilationWFactor())],
        )
        y = _apply_activation(y, opt.FusedActivationFunction())
        env[outs[0]] = qdq(y, outs[0])
        layout[outs[0]] = 'NCHW'

      elif op_name == 'DEPTHWISE_CONV_2D':
        opt = schema.DepthwiseConv2DOptions()
        opt.Init(op.BuiltinOptions().Bytes, op.BuiltinOptions().Pos)
        x = get_var(ins[0], 'NCHW')
        # TFLite depthwise weight: [1, kH, kW, C_out] -> CoreML: [C_out, 1, kH, kW]
        c_in = int(x.shape[1])
        w = get_quant_weight(ins[1], perm=[3, 0, 1, 2], new_qdim=0)
        b = get_const_fp32(ins[2]) if len(ins) > 2 and ins[2] >= 0 else None
        pad = 'same' if opt.Padding() == schema.Padding.SAME else 'valid'
        y = mb.conv(
          x=x,
          weight=w,
          bias=b,
          strides=[opt.StrideH(), opt.StrideW()],
          pad_type=pad,
          dilations=[max(1, opt.DilationHFactor()), max(1, opt.DilationWFactor())],
          groups=c_in,
        )
        y = _apply_activation(y, opt.FusedActivationFunction())
        env[outs[0]] = qdq(y, outs[0])
        layout[outs[0]] = 'NCHW'

      elif op_name in ('ADD', 'SUB', 'MUL', 'MINIMUM', 'MAXIMUM'):
        act = schema.ActivationFunctionType.NONE
        if op_name == 'ADD' and op.BuiltinOptions() is not None:
          opt = schema.AddOptions()
          opt.Init(op.BuiltinOptions().Bytes, op.BuiltinOptions().Pos)
          act = opt.FusedActivationFunction()
        elif op_name == 'SUB' and op.BuiltinOptions() is not None:
          opt = schema.SubOptions()
          opt.Init(op.BuiltinOptions().Bytes, op.BuiltinOptions().Pos)
          act = opt.FusedActivationFunction()
        elif op_name == 'MUL' and op.BuiltinOptions() is not None:
          opt = schema.MulOptions()
          opt.Init(op.BuiltinOptions().Bytes, op.BuiltinOptions().Pos)
          act = opt.FusedActivationFunction()

        # Align layouts if both are 4D
        l0 = layout.get(ins[0], 'NHWC' if len(shapes[ins[0]]) == 4 else 'OTHER')
        l1 = layout.get(ins[1], 'NHWC' if len(shapes[ins[1]]) == 4 else 'OTHER')
        target = 'NCHW' if ('NCHW' in (l0, l1) and len(shapes[outs[0]]) == 4) else None
        a = get_var(ins[0], target if len(shapes[ins[0]]) == 4 else None)
        b = get_var(ins[1], target if len(shapes[ins[1]]) == 4 else None)
        if target == 'NCHW':
          if len(shapes[ins[1]]) == 1 and shapes[ins[1]][0] == shapes[outs[0]][3]:
            b = np.reshape(b, (1, -1, 1, 1)) if isinstance(b, np.ndarray) else mb.reshape(x=b, shape=[1, shapes[ins[1]][0], 1, 1])
          elif len(shapes[ins[0]]) == 1 and shapes[ins[0]][0] == shapes[outs[0]][3]:
            a = np.reshape(a, (1, -1, 1, 1)) if isinstance(a, np.ndarray) else mb.reshape(x=a, shape=[1, shapes[ins[0]][0], 1, 1])

        fn_map = {
          'ADD': mb.add,
          'SUB': mb.sub,
          'MUL': mb.mul,
          'MINIMUM': mb.minimum,
          'MAXIMUM': mb.maximum,
        }
        y = fn_map[op_name](x=a, y=b)
        y = _apply_activation(y, act)
        env[outs[0]] = y if (symmetric and op_name in ('MINIMUM', 'MAXIMUM')) else qdq(y, outs[0])
        layout[outs[0]] = target if target else l0

      elif op_name == 'RELU':
        x = get_var(ins[0])
        y = mb.relu(x=x)
        env[outs[0]] = y if symmetric else qdq(y, outs[0])
        layout[outs[0]] = layout.get(ins[0], 'OTHER')

      elif op_name == 'LOGISTIC':
        x = get_var(ins[0])
        y = mb.sigmoid(x=x)
        env[outs[0]] = y if symmetric else qdq(y, outs[0])
        layout[outs[0]] = layout.get(ins[0], 'OTHER')

      elif op_name == 'DEPTH_TO_SPACE':
        opt = schema.DepthToSpaceOptions()
        opt.Init(op.BuiltinOptions().Bytes, op.BuiltinOptions().Pos)
        x = get_var(ins[0], 'NCHW')
        y = mb.depth_to_space(x=x, block_size=opt.BlockSize())
        env[outs[0]] = y if symmetric else qdq(y, outs[0])
        layout[outs[0]] = 'NCHW'

      elif op_name == 'AVERAGE_POOL_2D':
        opt = schema.Pool2DOptions()
        opt.Init(op.BuiltinOptions().Bytes, op.BuiltinOptions().Pos)
        x = get_var(ins[0], 'NCHW')
        pad = 'same' if opt.Padding() == schema.Padding.SAME else 'valid'
        y = mb.avg_pool(
          x=x,
          kernel_sizes=[opt.FilterHeight(), opt.FilterWidth()],
          strides=[opt.StrideH(), opt.StrideW()],
          pad_type=pad,
        )
        y = _apply_activation(y, opt.FusedActivationFunction())
        env[outs[0]] = qdq(y, outs[0])
        layout[outs[0]] = 'NCHW'

      elif op_name == 'RESIZE_BILINEAR':
        opt = schema.ResizeBilinearOptions()
        opt.Init(op.BuiltinOptions().Bytes, op.BuiltinOptions().Pos)
        x = get_var(ins[0], 'NCHW')
        target_hw = get_var(ins[1])
        out_h, out_w = shapes[outs[0]][1], shapes[outs[0]][2]
        mode = 'ALIGN_CORNERS' if opt.AlignCorners() else ('STRICT_ALIGN_CORNERS' if opt.HalfPixelCenters() else 'DEFAULT')
        y = mb.resize_bilinear(
          x=x,
          target_size_height=int(out_h),
          target_size_width=int(out_w),
          sampling_mode=mode,
        )
        env[outs[0]] = y if symmetric else qdq(y, outs[0])
        layout[outs[0]] = 'NCHW'

      elif op_name == 'CONCATENATION':
        opt = schema.ConcatenationOptions()
        opt.Init(op.BuiltinOptions().Bytes, op.BuiltinOptions().Pos)
        axis = opt.Axis()
        all_4d = all(len(shapes[i]) == 4 for i in ins)
        use_nchw = all_4d and any(layout.get(i) == 'NCHW' for i in ins)
        vals = [get_var(i, 'NCHW' if use_nchw else ('NHWC' if all_4d else None)) for i in ins]
        mil_axis = _nhwc_axis_to_nchw(axis, 4) if use_nchw else axis
        y = mb.concat(values=vals, axis=mil_axis)
        y = _apply_activation(y, opt.FusedActivationFunction())
        env[outs[0]] = qdq(y, outs[0])
        layout[outs[0]] = 'NCHW' if use_nchw else ('NHWC' if all_4d else 'OTHER')

      elif op_name == 'SPLIT':
        opt = schema.SplitOptions()
        opt.Init(op.BuiltinOptions().Bytes, op.BuiltinOptions().Pos)
        axis = int(get_var(ins[0]))
        x = get_var(ins[1])
        cur_layout = layout.get(ins[1], 'OTHER')
        mil_axis = _nhwc_axis_to_nchw(axis, 4) if cur_layout == 'NCHW' else axis
        splits = mb.split(x=x, num_splits=opt.NumSplits(), axis=mil_axis)
        if not isinstance(splits, (list, tuple)):
          splits = [splits]
        for out_idx, s in zip(outs, splits):
          env[out_idx] = s if symmetric else qdq(s, out_idx)
          layout[out_idx] = cur_layout

      elif op_name == 'MEAN':
        opt = schema.ReducerOptions()
        opt.Init(op.BuiltinOptions().Bytes, op.BuiltinOptions().Pos)
        axes = [int(a) for a in get_var(ins[1]).flatten()]
        cur_layout = layout.get(ins[0], 'OTHER')
        x = get_var(ins[0])
        if cur_layout == 'NCHW':
          axes = [_nhwc_axis_to_nchw(a, 4) for a in axes]
        y = mb.reduce_mean(x=x, axes=axes, keep_dims=opt.KeepDims())
        env[outs[0]] = qdq(y, outs[0])
        layout[outs[0]] = 'NCHW' if (cur_layout == 'NCHW' and opt.KeepDims()) else 'OTHER'

      elif op_name == 'ARG_MAX':
        opt = schema.ArgMaxOptions()
        opt.Init(op.BuiltinOptions().Bytes, op.BuiltinOptions().Pos)
        axis = int(get_var(ins[1]))
        cur_layout = layout.get(ins[0], 'OTHER')
        x = get_var(ins[0])
        if cur_layout == 'NCHW':
          axis = _nhwc_axis_to_nchw(axis, 4)
        if minimum_deployment_target >= ct.target.iOS18:
          env[outs[0]] = mb.reduce_argmax(
            x=x, axis=axis, keep_dims=False, output_dtype='uint16'
          )
        else:
          y = mb.reduce_argmax(x=x, axis=axis, keep_dims=False)
          env[outs[0]] = mb.cast(x=y, dtype='int32')
        layout[outs[0]] = 'OTHER'

      elif op_name == 'FULLY_CONNECTED':
        opt = schema.FullyConnectedOptions()
        opt.Init(op.BuiltinOptions().Bytes, op.BuiltinOptions().Pos)
        x = get_var(ins[0], 'NHWC' if len(get_var(ins[0]).shape) == 4 else None)
        if len(x.shape) > 2:
          in_features = int(get_var(ins[1]).shape[1]) if buffers[ins[1]] is None else int(shapes[ins[1]][1])
          x = mb.reshape(x=x, shape=[-1, in_features])
        b = get_const_fp32(ins[2]) if len(ins) > 2 and ins[2] >= 0 and buffers[ins[2]] is not None else None
        if buffers[ins[1]] is not None:
          # TFLite FC weight is [out_features, in_features], CoreML linear expects [out_features, in_features]
          w = get_quant_weight(ins[1])
          y = mb.linear(x=x, weight=w, bias=b)
        else:
          w = get_var(ins[1])
          y = mb.matmul(x=x, y=w, transpose_y=True)
          if b is not None:
            y = mb.add(x=y, y=b)
        y = _apply_activation(y, opt.FusedActivationFunction())
        env[outs[0]] = qdq(y, outs[0])
        layout[outs[0]] = 'OTHER'

      elif op_name == 'SOFTMAX':
        opt = schema.SoftmaxOptions()
        opt.Init(op.BuiltinOptions().Bytes, op.BuiltinOptions().Pos)
        x = get_var(ins[0], 'NHWC' if len(shapes[ins[0]]) == 4 else None)
        if opt.Beta() != 1.0:
          x = mb.mul(x=x, y=np.float32(opt.Beta()))
        y = mb.softmax(x=x, axis=-1)
        env[outs[0]] = y if symmetric else qdq(y, outs[0])
        layout[outs[0]] = 'NHWC' if len(shapes[outs[0]]) == 4 else 'OTHER'

      elif op_name == 'RESHAPE':
        x = get_var(ins[0], 'NHWC' if len(get_var(ins[0]).shape) == 4 else None)
        if shapes[outs[0]]:
          target_shape = np.array(shapes[outs[0]], dtype=np.int32)
        elif len(ins) > 1 and buffers[ins[1]] is not None:
          target_shape = buffers[ins[1]].astype(np.int32).flatten()
        else:
          opt = schema.ReshapeOptions()
          opt.Init(op.BuiltinOptions().Bytes, op.BuiltinOptions().Pos)
          target_shape = np.array(
            [opt.NewShape(j) for j in range(opt.NewShapeLength())],
            dtype=np.int32,
          )
        y = mb.reshape(x=x, shape=target_shape)
        env[outs[0]] = y if symmetric else qdq(y, outs[0])
        layout[outs[0]] = 'NHWC' if len(y.shape) == 4 else 'OTHER'

      elif op_name == 'SQUEEZE':
        x = get_var(ins[0], 'NHWC' if len(get_var(ins[0]).shape) == 4 else None)
        opt = schema.SqueezeOptions()
        opt.Init(op.BuiltinOptions().Bytes, op.BuiltinOptions().Pos)
        axes = [opt.SqueezeDims(j) for j in range(opt.SqueezeDimsLength())]
        y = mb.squeeze(x=x, axes=axes if axes else None)
        env[outs[0]] = y if symmetric else qdq(y, outs[0])
        layout[outs[0]] = 'NHWC' if len(y.shape) == 4 else 'OTHER'

      elif op_name == 'QUANTIZE':
        x = get_var(ins[0])
        env[outs[0]] = qdq(x, outs[0])
        layout[outs[0]] = layout.get(ins[0], 'OTHER')

      elif op_name == 'DEQUANTIZE':
        x = get_var(ins[0])
        env[outs[0]] = x
        layout[outs[0]] = layout.get(ins[0], 'OTHER')

      elif op_name == 'TRANSPOSE':
        x = get_var(ins[0], 'NHWC' if len(get_var(ins[0]).shape) == 4 else None)
        perm = [int(p) for p in get_var(ins[1]).flatten()]
        y = mb.transpose(x=x, perm=perm)
        env[outs[0]] = y if symmetric else qdq(y, outs[0])
        layout[outs[0]] = 'OTHER'

      elif op_name == 'CAST':
        x = get_var(ins[0])
        out_t = tensors[outs[0]].Type()
        mil_dt = 'int32' if out_t in (schema.TensorType.INT32, schema.TensorType.INT64) else 'fp32'
        y = mb.cast(x=x, dtype=mil_dt)
        env[outs[0]] = y if symmetric else qdq(y, outs[0])
        layout[outs[0]] = layout.get(ins[0], 'OTHER')

      elif op_name == 'GATHER':
        opt = schema.GatherOptions()
        opt.Init(op.BuiltinOptions().Bytes, op.BuiltinOptions().Pos)
        params = get_var(ins[0])
        indices = get_var(ins[1])
        y = mb.gather(x=params, indices=indices, axis=opt.Axis())
        env[outs[0]] = y if symmetric else qdq(y, outs[0])
        layout[outs[0]] = 'OTHER'

      elif op_name == 'PAD':
        x = get_var(ins[0], 'NHWC' if len(get_var(ins[0]).shape) == 4 else None)
        paddings = buffers[ins[1]].astype(np.int32).flatten()
        y = mb.pad(x=x, pad=paddings, mode='constant', constant_val=np.float32(0.0))
        env[outs[0]] = y if symmetric else qdq(y, outs[0])
        layout[outs[0]] = 'OTHER'

      elif op_name == 'SLICE':
        x = get_var(ins[0], 'NHWC' if len(get_var(ins[0]).shape) == 4 else None)
        begin = buffers[ins[1]].astype(np.int32).flatten()
        size = buffers[ins[2]].astype(np.int32).flatten()
        end = np.where(size < 0, np.array(x.shape, dtype=np.int32), begin + size)
        y = mb.slice_by_index(x=x, begin=begin, end=end)
        env[outs[0]] = y if symmetric else qdq(y, outs[0])
        layout[outs[0]] = 'OTHER'

      elif op_name == 'STRIDED_SLICE':
        opt = schema.StridedSliceOptions()
        opt.Init(op.BuiltinOptions().Bytes, op.BuiltinOptions().Pos)
        x = get_var(ins[0], 'NHWC' if len(get_var(ins[0]).shape) == 4 else None)
        begin = buffers[ins[1]].astype(np.int32).flatten()
        end = buffers[ins[2]].astype(np.int32).flatten()
        strides = buffers[ins[3]].astype(np.int32).flatten()
        begin_mask = [bool((opt.BeginMask() >> i) & 1) for i in range(len(begin))]
        end_mask = [bool((opt.EndMask() >> i) & 1) for i in range(len(end))]
        squeeze_mask = [bool((opt.ShrinkAxisMask() >> i) & 1) for i in range(len(begin))]
        y = mb.slice_by_index(
          x=x,
          begin=begin,
          end=end,
          stride=strides,
          begin_mask=begin_mask,
          end_mask=end_mask,
          squeeze_mask=squeeze_mask,
        )
        env[outs[0]] = y if symmetric else qdq(y, outs[0])
        layout[outs[0]] = 'OTHER'

      elif op_name == 'UNPACK':
        opt = schema.UnpackOptions()
        opt.Init(op.BuiltinOptions().Bytes, op.BuiltinOptions().Pos)
        x = get_var(ins[0], 'NHWC' if len(get_var(ins[0]).shape) == 4 else None)
        splits = mb.split(x=x, num_splits=opt.Num(), axis=opt.Axis())
        if not isinstance(splits, (list, tuple)):
          splits = [splits]
        for out_idx, s in zip(outs, splits):
          sq = mb.squeeze(x=s, axes=[opt.Axis()])
          env[out_idx] = sq if symmetric else qdq(sq, out_idx)
          layout[out_idx] = 'OTHER'

      elif op_name == 'TFLite_Detection_PostProcess':
        opts = flexbuffers.Loads(op.CustomOptionsAsNumpy().tobytes())
        boxes_raw = get_var(ins[0])
        scores_var = get_var(ins[1])
        anchors = get_const_fp32(ins[2])  # [num_anchors, 4] as [yc, xc, h, w]
        num_anchors = anchors.shape[0]
        anchors_yx = anchors[:, :2].reshape(1, num_anchors, 2)
        anchors_hw = anchors[:, 2:].reshape(1, num_anchors, 2)

        yx_raw, hw_raw = mb.split(x=boxes_raw, num_splits=2, axis=2)
        yx = mb.add(
          x=mb.mul(
            x=mb.mul(x=yx_raw, y=np.array([1.0 / opts['y_scale'], 1.0 / opts['x_scale']], dtype=np.float32)),
            y=anchors_hw,
          ),
          y=anchors_yx,
        )
        hw = mb.mul(
          x=mb.exp(
            x=mb.mul(x=hw_raw, y=np.array([1.0 / opts['h_scale'], 1.0 / opts['w_scale']], dtype=np.float32))
          ),
          y=anchors_hw,
        )
        y_c, x_c = mb.split(x=yx, num_splits=2, axis=2)
        h_c, w_c = mb.split(x=hw, num_splits=2, axis=2)
        raw_coords = mb.concat(values=[x_c, y_c, w_c, h_c], axis=2)
        boxes_t = mb.transpose(x=raw_coords, perm=[0, 2, 1])

        scores_t = mb.transpose(x=scores_var, perm=[0, 2, 1])
        score_thresh = np.float32(opts['nms_score_threshold'])
        masked_scores = mb.select(
          cond=mb.greater_equal(x=scores_t, y=score_thresh),
          a=scores_t,
          b=np.float32(0.0),
        )
        coords_t, conf_t, _ = mb.non_maximum_suppression(
          boxes=boxes_t,
          scores=masked_scores,
          iou_threshold=np.float32(opts['nms_iou_threshold']),
          max_boxes=int(opts['max_detections']),
          per_class_suppression=True,
        )
        coords = mb.transpose(x=coords_t, perm=[0, 2, 1])
        conf = mb.transpose(x=conf_t, perm=[0, 2, 1])
        t = np.array(
          [
            [0, 1, 0, 1],
            [1, 0, 1, 0],
            [0, -0.5, 0, 0.5],
            [-0.5, 0, 0.5, 0],
          ],
          dtype=np.float32,
        )
        det_boxes = mb.matmul(x=coords, y=t)
        cls_idx = mb.reduce_argmax(x=conf, axis=2, keep_dims=False)
        det_classes = mb.sub(x=mb.cast(x=cls_idx, dtype='fp32'), y=np.float32(1.0))
        det_scores = mb.reduce_max(x=conf, axes=[2], keep_dims=False)
        valid_boxes = mb.cast(x=mb.greater_equal(x=det_scores, y=score_thresh), dtype='fp32')
        det_num = mb.reduce_sum(x=valid_boxes, axes=[1], keep_dims=False)
        for out_idx, v in zip(outs, [det_boxes, det_classes, det_scores, det_num]):
          env[out_idx] = v
          layout[out_idx] = 'OTHER'

      else:
        raise NotImplementedError(f'Unsupported TFLite op: {op_name} (op {op_idx})')

    out_vars = []
    for idx in sg_outputs:
      v = get_var(idx, 'NHWC' if len(shapes[idx]) == 4 else None)
      out_vars.append(v)
    func.set_outputs(out_vars)

  prog = Program()
  prog.add_function('main', func)

  mlmodel = ct.convert(
    prog,
    convert_to='mlprogram',
    minimum_deployment_target=minimum_deployment_target,
    compute_precision=compute_precision,
  )
  spec = mlmodel.get_spec()
  for i, name in enumerate(input_names):
    old = spec.description.input[i].name
    if old != name:
      ct.utils.rename_feature(spec, old, name)
  for i, idx in enumerate(sg_outputs):
    out_name = _sanitize_name(tensors[idx].Name().decode('utf-8'))
    old = spec.description.output[i].name
    if old != out_name:
      ct.utils.rename_feature(spec, old, out_name)

  final_model = ct.models.MLModel(spec, weights_dir=mlmodel.weights_dir)
  if short_description:
    final_model.short_description = short_description
  final_model.save(export_path)
  print('Done! Quantized TFLite -> Core ML exported to:', export_path)
  return final_model


if __name__ == '__main__':
  import argparse

  parser = argparse.ArgumentParser(
    description='Convert a quantized INT8/UINT8 TFLite model to Core ML .mlpackage.'
  )
  parser.add_argument('tflite_path', help='Path to input quantized .tflite file')
  parser.add_argument('export_path', help='Path to output .mlpackage directory')
  parser.add_argument(
    '--weights-only',
    action='store_true',
    help='Only quantize weights (W8) without inserting activation quantize/dequantize ops.',
  )
  parser.add_argument(
    '--symmetric',
    action='store_true',
    help='Convert asymmetric quantized tensors (zero_point != 0) to symmetric (zero_point = 0).',
  )
  args = parser.parse_args()
  convert_tflite_to_mlpackage(
    args.tflite_path,
    args.export_path,
    quantize_activations=not args.weights_only,
    symmetric=args.symmetric,
  )
