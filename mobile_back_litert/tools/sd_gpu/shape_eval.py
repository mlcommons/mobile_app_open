# Copyright 2025 The MLPerf Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ==============================================================================
"""Evaluate the shape-computation cone of a TFLite graph with numpy.

`experimental_preserve_all_tensors` is the obvious way to learn what the shape
tensors hold, but it disables memory planning, so on the SD decoder and
diffusion exports it needs many gigabytes. Everything in the cone is small
integer arithmetic over tensor shapes, so evaluate it directly instead: SHAPE is
seeded from the (now fixed) shapes the interpreter reports, and the handful of
ops downstream of it are plain numpy.

Ops that are not implemented here are simply not folded -- the graph stays
correct, it just stays dynamic, and the caller can see what was left behind.
"""
import numpy as np
from ai_edge_litert import schema_py_generated as schema

OPNAME = {v: k for k, v in vars(schema.BuiltinOperator).items()
          if isinstance(v, int)}

NP_TYPE = {
    schema.TensorType.FLOAT32: np.float32,
    schema.TensorType.FLOAT16: np.float16,
    schema.TensorType.INT32: np.int32,
    schema.TensorType.INT64: np.int64,
    schema.TensorType.INT8: np.int8,
    schema.TensorType.UINT8: np.uint8,
    schema.TensorType.INT16: np.int16,
    schema.TensorType.BOOL: np.bool_,
}


def opcode_name(model, op):
    oc = model.operatorCodes[op.opcodeIndex]
    return OPNAME.get(max(oc.builtinCode, oc.deprecatedBuiltinCode), "?")


def const_value(model, sub, i):
    """Return the constant value of tensor i, or None if it is not constant."""
    t = sub.tensors[i]
    if not t.buffer:
        return None
    buf = model.buffers[t.buffer]
    if buf.data is None or len(buf.data) == 0:
        return None
    dt = NP_TYPE.get(t.type)
    if dt is None:
        return None
    shape = [] if t.shape is None else [int(x) for x in t.shape]
    raw = np.frombuffer(bytes(buf.data), dtype=dt)
    n = int(np.prod(shape)) if shape else 1
    if raw.size < n:
        return None
    return raw[:n].reshape(shape) if shape else raw[:1].reshape(())


def _opts(op):
    o = op.builtinOptions
    return o if o is not None else None


def evaluate(model, sub, shapes, max_elements=4096):
    """Return (values, dead_ops, unhandled) for the shape cone.

    values: {tensor index: np.ndarray} for tensors proven constant
    dead_ops: indices of ops whose every output is in values
    unhandled: Counter-ish dict of op names that blocked a fold
    """
    values = {}
    unhandled = {}
    dead = set()

    def get(i):
        if i in values:
            return values[i]
        return const_value(model, sub, i)

    for oi, op in enumerate(sub.operators):
        name = opcode_name(model, op)
        ins = [x for x in (op.inputs if op.inputs is not None else []) if x >= 0]
        outs = [x for x in (op.outputs if op.outputs is not None else []) if x >= 0]
        if not outs:
            continue

        if name == "SHAPE":
            if ins[0] not in shapes:
                continue
            dt = NP_TYPE.get(sub.tensors[outs[0]].type)
            if dt is None:
                unhandled["SHAPE(dtype)"] = unhandled.get("SHAPE(dtype)", 0) + 1
                continue
            # Must match the tensor's declared type: writing int32 bytes into a
            # tensor the schema calls int64 yields half the required bytes and
            # TFLite rejects the model with "invalidly specified in schema".
            values[outs[0]] = np.asarray(shapes[ins[0]], dtype=dt)
            dead.add(oi)
            continue

        argv = [get(i) for i in ins]
        if any(a is None for a in argv):
            continue
        # Only fold small integer results: this pass exists to freeze shape
        # vectors, not to bake activations into the file.
        if any(sub.tensors[o].type not in (schema.TensorType.INT32,
                                           schema.TensorType.INT64)
               for o in outs):
            continue
        if any(int(np.prod(shapes.get(o, [1]) or [1])) > max_elements for o in outs):
            continue

        o = _opts(op)
        try:
            if name == "GATHER":
                axis = getattr(o, "axis", 0) or 0
                res = np.take(argv[0], argv[1].astype(np.int64), axis=axis)
            elif name == "REDUCE_PROD":
                axes = tuple(np.atleast_1d(argv[1]).astype(int).tolist())
                res = np.prod(argv[0], axis=axes,
                              keepdims=bool(getattr(o, "keepDims", False)))
            elif name == "PACK":
                axis = getattr(o, "axis", 0) or 0
                res = np.stack(argv, axis=axis)
            elif name == "CONCATENATION":
                axis = getattr(o, "axis", 0) or 0
                res = np.concatenate([np.atleast_1d(a) for a in argv], axis=axis)
            elif name == "RESHAPE":
                if len(argv) > 1:
                    res = argv[0].reshape([int(x) for x in np.atleast_1d(argv[1])])
                else:
                    res = argv[0].reshape([int(x) for x in (o.newShape or [])])
            elif name == "BROADCAST_ARGS":
                res = np.asarray(np.broadcast_shapes(
                    tuple(int(x) for x in np.atleast_1d(argv[0])),
                    tuple(int(x) for x in np.atleast_1d(argv[1]))))
            elif name == "CAST":
                res = argv[0]
            elif name in ("ADD",):
                res = argv[0] + argv[1]
            elif name in ("SUB",):
                res = argv[0] - argv[1]
            elif name in ("MUL",):
                res = argv[0] * argv[1]
            elif name in ("DIV", "FLOOR_DIV"):
                res = argv[0] // argv[1]
            elif name == "MAXIMUM":
                res = np.maximum(argv[0], argv[1])
            elif name == "MINIMUM":
                res = np.minimum(argv[0], argv[1])
            elif name == "EXPAND_DIMS":
                res = np.expand_dims(argv[0], int(np.asarray(argv[1]).reshape(-1)[0]))
            elif name == "SQUEEZE":
                res = np.squeeze(argv[0])
            elif name == "RANGE":
                res = np.arange(argv[0], argv[1], argv[2])
            elif name == "FILL":
                res = np.full([int(x) for x in np.atleast_1d(argv[0])], argv[1])
            elif name == "TILE":
                res = np.tile(argv[0], [int(x) for x in np.atleast_1d(argv[1])])
            elif name == "SLICE":
                begin = [int(x) for x in np.atleast_1d(argv[1])]
                size = [int(x) for x in np.atleast_1d(argv[2])]
                sl = tuple(slice(b, None if s < 0 else b + s)
                           for b, s in zip(begin, size))
                res = argv[0][sl]
            elif name == "STRIDED_SLICE":
                begin = [int(x) for x in np.atleast_1d(argv[1])]
                end = [int(x) for x in np.atleast_1d(argv[2])]
                stride = [int(x) for x in np.atleast_1d(argv[3])]
                bm = getattr(o, "beginMask", 0) or 0
                em = getattr(o, "endMask", 0) or 0
                sm = getattr(o, "shrinkAxisMask", 0) or 0
                if getattr(o, "ellipsisMask", 0) or getattr(o, "newAxisMask", 0):
                    raise NotImplementedError("strided_slice masks")
                sl = []
                for d in range(len(begin)):
                    b = None if bm & (1 << d) else begin[d]
                    e = None if em & (1 << d) else end[d]
                    sl.append(slice(b, e, stride[d]))
                res = argv[0][tuple(sl)]
                for d in range(len(begin) - 1, -1, -1):
                    if sm & (1 << d):
                        res = np.take(res, 0, axis=d)
            else:
                unhandled[name] = unhandled.get(name, 0) + 1
                continue
        except Exception as e:  # noqa: BLE001
            unhandled["%s(%s)" % (name, type(e).__name__)] = \
                unhandled.get("%s(%s)" % (name, type(e).__name__), 0) + 1
            continue

        res = np.asarray(res)
        if res.size > max_elements:
            # Guard on the computed size too: an output whose shape was not in
            # `shapes` skipped the pre-check above.
            unhandled["%s(too-large)" % name] = \
                unhandled.get("%s(too-large)" % name, 0) + 1
            continue
        want = shapes.get(outs[0])
        if want is not None and list(res.shape) != list(want):
            # Our evaluation disagrees with the interpreter's own shape for this
            # tensor -- do not trust it.
            unhandled["%s(shape-mismatch)" % name] = \
                unhandled.get("%s(shape-mismatch)" % name, 0) + 1
            continue
        values[outs[0]] = res.astype(NP_TYPE[sub.tensors[outs[0]].type])
        dead.add(oi)

    return values, dead, unhandled
