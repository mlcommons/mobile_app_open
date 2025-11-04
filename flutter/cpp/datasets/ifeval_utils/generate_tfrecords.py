#!/usr/bin/env python3

import argparse
import json
import sys
from typing import Any, Dict, Iterable, List, Tuple

import tensorflow as tf


def parse_args():
    p = argparse.ArgumentParser(
        description="Convert a JSONL of IFEval prompts to TFRecord (Example) with key, prompt, instruction list, and kwargs/* features."
    )
    p.add_argument("--input_file", type=str, required=True, help="Path to input JSONL file.")
    p.add_argument("--output_file", type=str, required=True, help="Path to output TFRecord file.")
    return p.parse_args()


def iter_jsonl(path: str) -> Iterable[Tuple[int, Dict[str, Any]]]:
    with open(path, "r", encoding="utf-8") as f:
        for lineno, line in enumerate(f, 1):
            line = line.strip()
            if not line:
                continue
            try:
                yield lineno, json.loads(line)
            except json.JSONDecodeError as e:
                print(f"[warn] {path}:{lineno}: JSON decode error: {e}", file=sys.stderr)


def _bytes_feature(values: List[bytes]) -> tf.train.Feature:
    return tf.train.Feature(bytes_list=tf.train.BytesList(value=values))


def _int64_feature(values: List[int]) -> tf.train.Feature:
    return tf.train.Feature(int64_list=tf.train.Int64List(value=values))


def _to_bytes(s: str) -> bytes:
    return s.encode("utf-8")


def to_feature(value: Any) -> tf.train.Feature:
    """
    Map a python value into a tf.train.Feature following supported types.
    """
    if isinstance(value, bool):  # check before int
        return _int64_feature([int(value)])
    if isinstance(value, int):
        return _int64_feature([value])
    if isinstance(value, str):
        return _bytes_feature([_to_bytes(value)])

    if isinstance(value, list):
        if all(isinstance(x, str) for x in value):
            return _bytes_feature([_to_bytes(x) for x in value])
        if all(isinstance(x, bool) for x in value):
            return _int64_feature([int(x) for x in value])
        if all(isinstance(x, int) for x in value):
            return _int64_feature([int(x) for x in value])

    # Fallback: JSON-serialize and store as bytes
    try:
        s = json.dumps(value, ensure_ascii=False)
    except Exception:
        s = str(value)
    return _bytes_feature([_to_bytes(s)])


def build_example(record: Dict[str, Any]) -> tf.train.Example:
    """
    Build a tf.train.Example with:
      - "key": int64 (identifier)
      - "prompt": bytes (UTF-8)
      - "instruction_id_list": bytes_list of strings
      - "kwargs/<i>/<k>": features from kwargs[i][k]
    """
    feats: Dict[str, tf.train.Feature] = {}

    # key (identifier) — ensure we write feature named "key"
    key_val = record.get("key", None)
    if isinstance(key_val, (int, bool)):
        feats["key"] = _int64_feature([int(key_val)])
    else:
        try:
            feats["key"] = _int64_feature([int(key_val)])
        except Exception:
            print(f"[warn] record has non-int key={key_val!r}; writing 0", file=sys.stderr)
            feats["key"] = _int64_feature([0])

    # prompt (optional but recommended)
    prompt = record.get("prompt", "")
    if not isinstance(prompt, str):
        prompt = str(prompt) if prompt is not None else ""
    feats["prompt"] = _bytes_feature([_to_bytes(prompt)])

    # instruction_id_list
    ids = record.get("instruction_id_list", [])
    if not isinstance(ids, list) or not all(isinstance(x, str) for x in ids):
        raise ValueError("Each record must contain 'instruction_id_list' as a list of strings.")
    feats["instruction_id_list"] = _bytes_feature([_to_bytes(x) for x in ids])

    # kwargs aligned by index
    kwargs_list = record.get("kwargs", [])
    if not isinstance(kwargs_list, list):
        kwargs_list = []

    for i, _id in enumerate(ids):
        if i >= len(kwargs_list):
            continue
        entry = kwargs_list[i]
        if not isinstance(entry, dict) or not entry:
            continue
        for k, v in entry.items():
            fname = f"kwargs/{i}/{k}"
            feats[fname] = to_feature(v)

    return tf.train.Example(features=tf.train.Features(feature=feats))


def main():
    args = parse_args()
    options = tf.io.TFRecordOptions(compression_type="ZLIB")

    written = 0
    with tf.io.TFRecordWriter(args.output_file, options=options) as w:
        for lineno, rec in iter_jsonl(args.input_file):
            try:
                ex = build_example(rec)
                w.write(ex.SerializeToString())
                written += 1
            except Exception as e:
                print(f"[warn] skipping line {lineno}: {e}", file=sys.stderr)

    print(f"[done] wrote {written} Example(s) -> {args.output_file}", file=sys.stderr)


if __name__ == "__main__":
    main()
