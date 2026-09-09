#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.11"
# dependencies = [
#     "ai-edge-litert==2.2.0",
#     "numpy",
#     "flatbuffers",
# ]
# ///
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
"""Rewrite the Stable Diffusion exports so the LiteRT GPU delegate accepts them.

The v5_0 SD exports run only on CPU under LiteRT. Four separate things stop the
GPU delegate from taking them; this script fixes all four and checks that the
result still computes exactly the same function.

1. Dynamic shapes. Every input declares -1 on the batch dimension and the
   graphs recompute their own shapes at run time (SHAPE -> REDUCE_PROD / GATHER
   / PACK / CONCATENATION / BROADCAST_ARGS -> RESHAPE / BROADCAST_TO). A RESHAPE
   whose shape operand is computed has a dynamic output, and the delegate
   refuses the whole graph:

       Attempting to use a delegate that only supports static-sized tensors
       with a graph that has dynamic-sized tensors

   The pipeline only ever runs batch 1, so those shapes are constants. Pin the
   inputs and constant-fold the shape cone.

2. BROADCAST_TO is not implemented by the GPU delegate. The group-norm blocks
   use it to materialise both operands of a MUL/SUB to a common shape, which
   TFLite's binary kernels already do implicitly, so it can be deleted.

3. Rank-5 tensors. The group-norm blocks work in [1, H, W, groups, ch/group]
   and the delegate refuses anything above rank 4. Every rank-5 tensor here has
   a leading dimension of 1, so dropping it is a shape-only change.

4. A stale SUB version. TFLite raised SUB to version 3 because of those rank-5
   operands; the GPU delegate supports up to version 2. Once step 3 has run
   nothing needs version 3, but the declared version stays and splits the graph
   into partitions too small to delegate. This is the step that takes the
   diffusion and decoder graphs from ~7% delegated to fully accelerated.

Measured on an M-series Mac (LiteRT CompiledModel, ms per invocation):

    model           original CPU   rewritten CPU   rewritten Metal
    text encoder            12.4            13.1              14.1
    diffusion model       1494.0           977.0             331.8
    decoder               7702.3          2000.5             463.1

The diffusion and decoder graphs come out fully GPU-accelerated; the text
encoder keeps two GATHERs on CPU because its token/position indices are 2-D and
the delegate only takes 1-D indices there. All three outputs are bit-identical
to the originals.

Usage:
    uv run convert.py --in-dir <dir with the v5_0 exports> --out-dir <dir>

The dependency pins live in the PEP 723 header above, so uv builds the
environment itself and nothing has to be installed into the repo's Python. The
ai-edge-litert pin matters: it is the runtime whose Metal accelerator these
models are being made compatible with.

The rewritten files are named *_litert.tflite, matching the convention already
used by llama_q8_ekv3072_litert.tflite.
"""
import argparse
import collections
import os
import sys

import flatbuffers
import numpy as np
from ai_edge_litert import schema_py_generated as schema
from ai_edge_litert.interpreter import Interpreter

import shape_eval

# Ops whose own kernel broadcasts its operands, so an explicit BROADCAST_TO in
# front of them buys nothing.
BROADCASTING_CONSUMERS = {
    "ADD", "SUB", "MUL", "DIV", "MAXIMUM", "MINIMUM", "POW",
    "SQUARED_DIFFERENCE", "FLOOR_DIV", "FLOOR_MOD", "SELECT_V2",
}

# Operator versions we may lower once the rank-5 tensors are gone, with the
# element types the older version covers.
VERSION_RULES = {
    "SUB": (2, {schema.TensorType.FLOAT32, schema.TensorType.INT8,
                schema.TensorType.UINT8, schema.TensorType.INT32}),
}

SD_MODELS = [
    ("sd_text_encoder_dynamic_int8.tflite", "sd_text_encoder_litert.tflite"),
    ("sd_diffusion_model_dynamic_int8.tflite",
     "sd_diffusion_model_litert.tflite"),
    ("sd_decoder_dynamic_fp16.tflite", "sd_decoder_litert.tflite"),
]


def ins_of(op):
    return [] if op.inputs is None else [int(x) for x in op.inputs]


def outs_of(op):
    return [] if op.outputs is None else [int(x) for x in op.outputs]


def serialize(model):
    b = flatbuffers.Builder(1024)
    b.Finish(model.Pack(b), file_identifier=b"TFL3")
    return bytes(b.Output())


def probe(model_bytes, batch=1):
    """Resize inputs to `batch`, allocate, run once, and report every shape.

    The run matters. TFLite only fills in a dynamic tensor's dims while
    executing, so without it anything downstream of a computed RESHAPE reports a
    stale placeholder -- the text encoder's attention really produces
    [12,77,64] but advertises [1,77,64]. experimental_preserve_all_tensors would
    also give correct shapes but disables memory planning, which costs many
    gigabytes on these graphs.
    """
    it = Interpreter(model_content=model_bytes)
    want = {}
    for d in it.get_input_details():
        want[d["index"]] = [batch if v == -1 else int(v)
                            for v in d["shape_signature"]]
        it.resize_tensor_input(d["index"], want[d["index"]], strict=False)
    it.allocate_tensors()
    for d in it.get_input_details():
        it.set_tensor(d["index"],
                      np.zeros(want[d["index"]], dtype=d["dtype"]))
    it.invoke()
    shapes = {d["index"]: [int(x) for x in d["shape"]]
              for d in it.get_tensor_details()}
    names = {d["index"]: d["name"] for d in it.get_tensor_details()}
    del it
    return shapes, names, want


def fold_shapes(data, batch=1, rounds=4):
    """Pin the inputs and constant-fold the shape-computation cone."""
    folded = dead = 0
    for _ in range(rounds):
        model = schema.ModelT.InitFromPackedBuf(data, 0)
        sub = model.subgraphs[0]
        shapes, names, want = probe(data, batch)

        # The interpreter appends temporaries past the flatbuffer's tensor
        # count; everything below it must still line up by name or the rewrite
        # would silently retarget the wrong tensors.
        for j, t in enumerate(sub.tensors):
            fb = "" if t.name is None else bytes(t.name).decode()
            if j in names and names[j] != fb:
                raise RuntimeError(
                    "interpreter/flatbuffer tensor mismatch at %d" % j)

        for idx, dims in want.items():
            t = sub.tensors[idx]
            t.shape = list(dims)
            t.shapeSignature = None
            shapes[idx] = list(dims)

        values, dead_ops, unhandled = shape_eval.evaluate(model, sub, shapes)
        if not dead_ops:
            if folded == 0:
                data = serialize(model)  # still need the pinned inputs
            break

        for j in sorted(values):
            t = sub.tensors[j]
            v = np.asarray(values[j]).astype(shape_eval.NP_TYPE[t.type])
            buf = schema.BufferT()
            buf.data = np.frombuffer(v.tobytes(), dtype=np.uint8)
            model.buffers.append(buf)
            t.buffer = len(model.buffers) - 1
            t.shape = list(v.shape)
            t.shapeSignature = None

        counts = collections.Counter(
            shape_eval.opcode_name(model, sub.operators[i]) for i in dead_ops)
        sub.operators = [op for i, op in enumerate(sub.operators)
                         if i not in dead_ops]
        for t in sub.tensors:
            t.shapeSignature = None

        folded += len(values)
        dead += len(dead_ops)
        print("    folded %d tensors, dropped %d ops %s%s"
              % (len(values), len(dead_ops), dict(counts.most_common(8)),
                 (" UNHANDLED=%s" % unhandled) if unhandled else ""))
        data = serialize(model)
    return data, folded, dead


def drop_broadcast_to(model, sub, shapes):
    """Delete BROADCAST_TO ops whose consumers broadcast on their own."""
    consumers = collections.defaultdict(list)
    for oi, op in enumerate(sub.operators):
        for i in ins_of(op):
            if i >= 0:
                consumers[i].append(oi)
    graph_outputs = set(int(x) for x in (sub.outputs or []))

    remap, dead = {}, set()
    for oi, op in enumerate(sub.operators):
        if shape_eval.opcode_name(model, op) != "BROADCAST_TO":
            continue
        src, out = ins_of(op)[0], outs_of(op)[0]
        if out in graph_outputs:
            continue
        cons = [shape_eval.opcode_name(model, sub.operators[c])
                for c in consumers[out]]
        if not cons:
            dead.add(oi)
            continue
        # An identity broadcast is always safe to drop; a real one only if
        # every consumer broadcasts its own operands.
        if shapes.get(src) != shapes.get(out):
            if not all(c in BROADCASTING_CONSUMERS for c in cons):
                continue
        remap[out] = src
        dead.add(oi)

    def resolve(t):  # a chain of broadcasts can remap onto another removed one
        seen = set()
        while t in remap and t not in seen:
            seen.add(t)
            t = remap[t]
        return t

    for oi, op in enumerate(sub.operators):
        if oi in dead or op.inputs is None:
            continue
        op.inputs = [resolve(int(x)) if int(x) >= 0 else int(x)
                     for x in op.inputs]
    sub.operators = [op for i, op in enumerate(sub.operators) if i not in dead]
    return len(dead)


def squeeze_rank5(model, sub, shapes):
    """Drop the leading 1 from every rank>=5 tensor and fix the ops around it."""
    targets = {i for i, s in shapes.items() if len(s) >= 5}
    if not targets:
        return 0, {}
    if any(shapes[i][0] != 1 for i in targets):
        raise RuntimeError("a rank-5 tensor does not have a leading 1")

    notes = collections.Counter()
    # TFLite shares identical constants between ops, so the same axes or shape
    # buffer can be reached from several operators -- all 60 group norms in the
    # decoder point at one axes tensor. Adjust each buffer once; decrementing a
    # reduce axis twice silently produces wrong results.
    touched = set()
    for op in sub.operators:
        name = shape_eval.opcode_name(model, op)
        ins, outs = ins_of(op), outs_of(op)
        if not [i for i in ins + outs if i in targets]:
            continue

        if name == "RESHAPE" and outs[0] in targets and len(ins) > 1:
            t = sub.tensors[ins[1]]
            if t.buffer in touched:
                continue
            buf = model.buffers[t.buffer]
            v = np.frombuffer(bytes(buf.data), dtype=np.int32).copy()
            if len(v) >= 5 and v[0] == 1:
                touched.add(t.buffer)
                buf.data = np.frombuffer(v[1:].tobytes(), dtype=np.uint8)
                t.shape = [len(v) - 1]
                notes["reshape-target"] += 1
        elif name in ("MEAN", "SUM", "REDUCE_MAX", "REDUCE_MIN", "REDUCE_PROD"):
            t = sub.tensors[ins[1]]
            if t.buffer in touched:
                continue
            touched.add(t.buffer)
            buf = model.buffers[t.buffer]
            v = np.frombuffer(bytes(buf.data), dtype=np.int32).copy()
            nv = np.array([a - 1 if a > 0 else a for a in v], dtype=np.int32)
            buf.data = np.frombuffer(nv.tobytes(), dtype=np.uint8)
            notes["reduce-axes"] += 1

    # Constant data is unchanged: dropping a leading 1 moves no element.
    for i in sorted(targets):
        t = sub.tensors[i]
        cur = [] if t.shape is None else [int(x) for x in t.shape]
        t.shape = cur[1:] if len(cur) >= 5 and cur[0] == 1 else shapes[i][1:]
        t.shapeSignature = None
        q = t.quantization
        if q is not None and getattr(q, "quantizedDimension", 0):
            q.quantizedDimension = max(0, int(q.quantizedDimension) - 1)
            notes["quant-dim"] += 1
    return len(targets), dict(notes)


def lower_op_versions(model, sub, shapes):
    """Lower operator versions the graph no longer needs (see module docstring)."""
    users = collections.defaultdict(list)
    for op in sub.operators:
        users[int(op.opcodeIndex)].append(op)

    lowered = {}
    for ci, oc in enumerate(model.operatorCodes):
        code = max(oc.builtinCode, oc.deprecatedBuiltinCode)
        rule = VERSION_RULES.get(shape_eval.OPNAME.get(code))
        if rule is None or oc.version <= rule[0]:
            continue
        target, ok_types = rule
        safe = all(
            len(shapes.get(i, [])) <= 4 and sub.tensors[i].type in ok_types
            for op in users[ci] for i in ins_of(op) + outs_of(op) if i >= 0)
        if safe:
            lowered[shape_eval.OPNAME.get(code)] = (oc.version, target)
            oc.version = target
    return lowered


def optimize(data):
    """Steps 2-4: the op-level rewrites, after the shapes are static."""
    shapes, _, _ = probe(data)
    model = schema.ModelT.InitFromPackedBuf(data, 0)
    sub = model.subgraphs[0]

    n_bcast = drop_broadcast_to(model, sub, shapes)
    n_sq, notes = squeeze_rank5(model, sub, shapes)
    squeezed = {i: (v[1:] if len(v) >= 5 and v[0] == 1 else v)
                for i, v in shapes.items()}
    lowered = lower_op_versions(model, sub, squeezed)
    print("    dropped %d BROADCAST_TO, squeezed %d rank-5 tensors %s, "
          "lowered %s" % (n_bcast, n_sq, notes, lowered or "nothing"))
    return serialize(model)


def run(path_or_bytes, feeds):
    if isinstance(path_or_bytes, bytes):
        it = Interpreter(model_content=path_or_bytes)
    else:
        it = Interpreter(model_path=path_or_bytes)
    for d in it.get_input_details():
        want = [1 if v == -1 else int(v) for v in d["shape_signature"]]
        it.resize_tensor_input(d["index"], want, strict=False)
    it.allocate_tensors()
    for d in it.get_input_details():
        it.set_tensor(d["index"], feeds[d["name"]].astype(d["dtype"]))
    it.invoke()
    out = [np.array(it.get_tensor(d["index"])) for d in it.get_output_details()]
    del it
    return out


def verify(original_path, new_bytes, seed=0):
    """The rewrite must not change what the model computes."""
    rng = np.random.default_rng(seed)
    it = Interpreter(model_path=original_path)
    feeds = {}
    for d in it.get_input_details():
        shape = [1 if v == -1 else int(v) for v in d["shape_signature"]]
        if np.issubdtype(d["dtype"], np.integer):
            feeds[d["name"]] = rng.integers(0, 77, size=shape).astype(d["dtype"])
        else:
            feeds[d["name"]] = rng.standard_normal(shape).astype(d["dtype"])
    del it

    a, b = run(original_path, feeds), run(new_bytes, feeds)
    if len(a) != len(b):
        print("    output count changed: %d -> %d" % (len(a), len(b)))
        return False

    ok, worst = True, 0.0
    for x, y in zip(a, b):
        if x.shape != y.shape:
            print("    output shape changed: %s -> %s" % (x.shape, y.shape))
            ok = False
            continue
        # Compare the values themselves rather than reducing to a max
        # difference. max(0.0, nan) is 0.0 in Python, so a NaN difference
        # would otherwise report a clean 0.000e+00 and pass.
        if not np.array_equal(x, y, equal_nan=True):
            ok = False
        d = np.abs(x.astype(np.float64) - y.astype(np.float64))
        finite = d[np.isfinite(d)]
        if finite.size:
            worst = max(worst, float(finite.max()))
        if finite.size != d.size:
            print("    non-finite values present in %d of %d elements"
                  % (d.size - finite.size, d.size))
    print("    verified against the original: max_abs_diff=%.3e%s"
          % (worst, "" if ok else "  MISMATCH"))
    return ok


def convert_one(src, dst):
    print("  %s" % os.path.basename(src))
    data = open(src, "rb").read()
    data, folded, dead = fold_shapes(data)
    data = optimize(data)
    if not verify(src, data):
        print("    REFUSING to write: output changed")
        return False
    with open(dst, "wb") as f:
        f.write(data)
    print("    wrote %s (%.1f MB)" % (dst, os.path.getsize(dst) / 1048576))
    return True


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--in-dir", required=True,
                    help="directory holding the v5_0 SD exports")
    ap.add_argument("--out-dir", required=True)
    args = ap.parse_args()

    os.makedirs(args.out_dir, exist_ok=True)
    ok = True
    for src_name, dst_name in SD_MODELS:
        src = os.path.join(args.in_dir, src_name)
        if not os.path.exists(src):
            print("  missing %s -- skipped" % src)
            ok = False
            continue
        ok &= convert_one(src, os.path.join(args.out_dir, dst_name))
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
