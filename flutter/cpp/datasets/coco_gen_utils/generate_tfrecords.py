# Copyright 2024 The MLPerf Authors. All Rights Reserved.

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

#     http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""
Generate a TFRecord file for the Stable Diffusion benchmark.
captions_source.tsv can be downloaded from:
https://github.com/mlcommons/inference/blob/master/text_to_image/coco2014/captions/captions_source.tsv
Run with the following command:
```shell
python generate_tfrecords.py \
  --input_tsv ./captions_source.tsv \
  --output_tfrecord ./coco_gen.tfrecord \
  --image_dir ./coco2014
```
"""

import os
import tensorflow as tf
import pandas as pd
import requests
from transformers import TFCLIPModel, CLIPProcessor
from PIL import Image
from io import BytesIO
import argparse

MODEL_NAME = "openai/clip-vit-large-patch14"


# Function to parse command line arguments
def parse_args():
  parser = argparse.ArgumentParser(description="Process prompts from TSV file and save the results to TFRecord.")
  parser.add_argument('--input_tsv', type=str, required=True, help="Path to the input TSV file.")
  parser.add_argument('--num_images', type=int, required=False, help="Number of images to use.")
  parser.add_argument('--image_dir', type=str, required=True, help="Path to the image directory.")
  parser.add_argument('--output_tfrecord', type=str, required=True, help="Path to the output TFRecord file.")
  return parser.parse_args()


def download_image(url, file_path):
  """Downloads the image from the URL if it doesn't already exist."""
  if not os.path.exists(file_path):
    response = requests.get(url)
    image = Image.open(BytesIO(response.content))
    image.save(file_path)
    print(f"Downloaded image to {file_path}")


def serialize_example(caption, input_ids, attention_mask, file_name, clip_score):
  """Creates a tf.train.Example message ready to be written to a file."""
  feature = {
    'caption': tf.train.Feature(bytes_list=tf.train.BytesList(value=[caption.encode()])),
    'input_ids': tf.train.Feature(int64_list=tf.train.Int64List(value=input_ids)),
    'attention_mask': tf.train.Feature(int64_list=tf.train.Int64List(value=attention_mask)),
    'file_name': tf.train.Feature(bytes_list=tf.train.BytesList(value=[file_name.encode()])),
    'clip_score': tf.train.Feature(float_list=tf.train.FloatList(value=clip_score)),
  }
  example_proto = tf.train.Example(features=tf.train.Features(feature=feature))
  return example_proto.SerializeToString()


def main():
  args = parse_args()

  # Create the image directory if it doesn't exist
  if not os.path.exists(args.image_dir):
    os.makedirs(args.image_dir)

  # Load the TSV file
  df = pd.read_csv(args.input_tsv, sep='\t')
  print(df.columns)

  # Initialize the CLIP processor and model
  processor = CLIPProcessor.from_pretrained(MODEL_NAME)
  model = TFCLIPModel.from_pretrained(MODEL_NAME)

  with tf.io.TFRecordWriter(args.output_tfrecord, options='ZLIB') as writer:
    total = len(df)
    for idx, row in df.iterrows():
      caption = row['caption']
      file_name = row['file_name']
      coco_url = row['coco_url']

      image_path = os.path.join(args.image_dir, file_name)
      download_image(coco_url, image_path)
      image = Image.open(image_path)

      inputs = processor(text=[caption], images=image, padding="max_length", truncation=True, return_tensors="tf")
      outputs = model(**inputs)

      input_ids = inputs['input_ids'].numpy().flatten().tolist()
      attention_mask = inputs['attention_mask'].numpy().flatten().tolist()
      assert len(input_ids) == len(attention_mask) == 77
      clip_score = outputs.logits_per_image.numpy().flatten().tolist()

      example = serialize_example(
        caption=caption,
        input_ids=input_ids,
        attention_mask=attention_mask,
        file_name=file_name,
        clip_score=clip_score,
      )
      writer.write(example)
      print(f"Processed: {idx + 1}/{total} | Caption: {caption} | CLIP Score: {clip_score}")
      if args.num_images is not None and idx + 1 >= args.num_images:
        print(f"Early stopping at {args.num_images} images")
        break

  print(f"TFRecord file created at {args.output_tfrecord}")


if __name__ == "__main__":
  main()
