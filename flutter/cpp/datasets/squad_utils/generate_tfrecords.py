# Copyright 2020 The MLPerf Authors. All Rights Reserved.

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

#     http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


"""Generate tfrcords file to evaluate a model."""
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import argparse
import collections
import os

import tensorflow.compat.v1 as tf

from mobilebert import tokenization
from mobilebert.run_squad import read_squad_examples, convert_examples_to_features

parser = argparse.ArgumentParser()

parser.add_argument(
    "--vocab_file",
    required=True,
    help="The vocabulary file that the BERT model was trained on.")

parser.add_argument(
    "--predict_file",
    required=True,
    help="SQuAD json for predictions. E.g., dev-v1.1.json or test-v1.1.json")

parser.add_argument(
    "--output_dir",
    required=True,
    help="The output directory where the model checkpoints will be written.")

parser.add_argument(
    "--max_seq_length",
    type=int,
    default=384,
    help="The maximum total input sequence length after WordPiece tokenization. "
    "Sequences longer than this will be truncated, and sequences shorter "
    "than this will be padded.")

parser.add_argument(
    "--doc_stride",
    type=int,
    default=128,
    help="When splitting up a long document into chunks, how much stride to "
    "take between chunks.")

parser.add_argument(
    "--max_query_length",
    type=int,
    default=64,
    help="The maximum number of tokens for the question. Questions longer than "
    "this will be truncated to this length.")

args = parser.parse_args()


class GroundTruthWriter(object):
  """Writes GroundTruth data to TF example file."""

  def __init__(self, filename):
    self.filename = filename
    options = tf.python_io.TFRecordOptions(tf.python_io.TFRecordCompressionType.ZLIB)
    self._writer = tf.python_io.TFRecordWriter(filename, options=options)

  def process_feature(self, example):
    """Write a Example to the TFRecordWriter as a tf.train.Example."""

    tokenizer = tokenization.BasicTokenizer(do_lower_case=True)
    examples = collections.OrderedDict()
    token_bytes = [
        " ".join(tokenizer.tokenize(x)).encode("utf-8")
        for x in example.doc_tokens
    ]
    examples["tokens"] = tf.train.Feature(
        bytes_list=tf.train.BytesList(value=token_bytes))

    token_bytes = [x.encode("utf-8") for x in example.doc_tokens]
    examples["words"] = tf.train.Feature(
        bytes_list=tf.train.BytesList(value=token_bytes))

    qas_id_bytes = example.qas_id.encode("utf-8")
    examples["qas_id"] = tf.train.Feature(
        bytes_list=tf.train.BytesList(value=[qas_id_bytes]))

    token_bytes = [x.encode("utf-8") for x in example.answers]
    examples["answers"] = tf.train.Feature(
        bytes_list=tf.train.BytesList(value=token_bytes))

    tf_example = tf.train.Example(features=tf.train.Features(feature=examples))
    self._writer.write(tf_example.SerializeToString())

  def close(self):
    self._writer.close()


class FeatureWriter(object):
  """Writes InputFeature to TF example file."""

  def __init__(self, filename):
    self.filename = filename
    options = tf.python_io.TFRecordOptions(tf.python_io.TFRecordCompressionType.ZLIB)
    self._writer = tf.python_io.TFRecordWriter(filename, options=options)

  def process_feature(self, feature):
    """Write a InputFeature to the TFRecordWriter as a tf.train.Example."""

    def create_int_feature(values):
      feature = tf.train.Feature(
          int64_list=tf.train.Int64List(value=list(values)))
      return feature

    features = collections.OrderedDict()
    qas_id_bytes = feature.qas_id.encode("utf-8")
    features["qas_id"] = tf.train.Feature(
        bytes_list=tf.train.BytesList(value=[qas_id_bytes]))

    token_bytes = [x.encode("utf-8") for x in feature.tokens]
    features["tokens"] = tf.train.Feature(
        bytes_list=tf.train.BytesList(value=token_bytes))

    features["token_to_orig_map"] = create_int_feature(
        feature.token_to_orig_map.values())
    features["token_is_max_context"] = create_int_feature(
        feature.token_is_max_context.values())
    features["unique_ids"] = create_int_feature([feature.unique_id])
    features["input_ids"] = create_int_feature(feature.input_ids)
    features["input_mask"] = create_int_feature(feature.input_mask)
    features["segment_ids"] = create_int_feature(feature.segment_ids)

    tf_example = tf.train.Example(features=tf.train.Features(feature=features))
    self._writer.write(tf_example.SerializeToString())

  def close(self):
    self._writer.close()


def main(_):
  tokenizer = tokenization.FullTokenizer(
      vocab_file=args.vocab_file, do_lower_case=True)
  eval_examples = read_squad_examples(
      input_file=args.predict_file, is_training=False)
  gt_file = os.path.join(args.output_dir, "groundtruth.tfrecord")
  gt_writer = GroundTruthWriter(filename=gt_file)
  for example in eval_examples:
    gt_writer.process_feature(example)
  gt_writer.close()

  eval_file = os.path.join(args.output_dir, "eval.tfrecord")

  eval_writer = FeatureWriter(filename=eval_file)
  eval_features = []

  def append_feature(feature):
    eval_features.append(feature)
    eval_writer.process_feature(feature)

  convert_examples_to_features(
      examples=eval_examples,
      tokenizer=tokenizer,
      max_seq_length=args.max_seq_length,
      doc_stride=args.doc_stride,
      max_query_length=args.max_query_length,
      is_training=False,
      output_fn=append_feature)
  eval_writer.close()
  tf.logging.error("Writing tfrecords file completed.")


if __name__ == "__main__":
  tf.app.run()

