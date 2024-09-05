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
Convert RGB8 to PNG.
Run with the following command:
```shell
python rbg8_to_png.py --image_dir ./image_dir
```
"""

import os
from PIL import Image
import numpy as np
import argparse


def parse_args():
  parser = argparse.ArgumentParser(description="Convert RGB8 to PNG.")
  parser.add_argument('--image_dir', type=str, required=True, help="Path to the image directory.")
  parser.add_argument('--width', type=int, required=False, default=512, help="Width of the image.")
  parser.add_argument('--height', type=int, required=False, default=512, help="Height of the image.")
  return parser.parse_args()


def convert_rgb8_to_png(rgb8_file, png_file, width, height):
  with open(rgb8_file, 'rb') as f:
    data = f.read()
  image = np.frombuffer(data, dtype=np.uint8).reshape((height, width, 3))
  img = Image.fromarray(image, 'RGB')
  img.save(png_file)


def convert_all_rgb8_in_folder(folder_path, width, height):
  print(f'Converting RGB8 to PNG in {folder_path}')
  for filename in os.listdir(folder_path):
    if filename.endswith('.rgb8'):
      rgb8_file = os.path.join(folder_path, filename)
      png_file = os.path.join(folder_path, os.path.splitext(filename)[0] + '.png')
      convert_rgb8_to_png(rgb8_file, png_file, width, height)
      print(f'Converted {rgb8_file} to {png_file}')


if __name__ == "__main__":
  args = parse_args()
  convert_all_rgb8_in_folder(args.image_dir, args.width, args.height)
