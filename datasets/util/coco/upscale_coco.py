# Copyright (c) 2020-2021 The MLPerf Authors. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################

import argparse
import json
import os
import time

import cv2
import numpy as np
#from pycocotools.coco import COCO


def parse_args():
    parser = argparse.ArgumentParser(description="Upscale COCO dataset")
    parser.add_argument('--inputs', '-i', type=str, default='/coco',
                        help='input directory for coco dataset')
    parser.add_argument('--outputs', '-o', required=True, type=str,
                        help='output directory for upscaled coco dataset')
    parser.add_argument('--size', required=True, type=int, nargs='+',
                        help='upscaled image sizes (e.g 300 300, 1200 1200')
    parser.add_argument('--format', '-f', type=str, default='jpg',
                        help='image format')
    return parser.parse_args()


def upscale_coco(indir, outdir, size, fmt):
    # Build directories.
    print('Building directories...')
    size = tuple(size)
    image_in_path = indir
    image_out_path = os.path.join(outdir)
    if not os.path.exists(image_out_path):
        os.makedirs(image_out_path)

    # Read images.
    print('Reading COCO images...')
    images = open("/util/coco/coco_calibration_files.txt").read().split("\n")

    # Upscale images.
    print('Upscaling images...')
    for img in images:
        if img == "":
            continue
        file_name = img;
        # Load, upscale, and save image.
        print("Loading %s/%s" % (image_in_path, file_name))
        image = cv2.imread(os.path.join(image_in_path, file_name))
        if len(image.shape) < 3 or image.shape[2] != 3:
            # some images might be grayscale
            image = cv2.cvtColor(image, cv2.COLOR_GRAY2RGB)
        image = cv2.resize(image, size, interpolation=cv2.INTER_LINEAR)
        cv2.imwrite(os.path.join(image_out_path, img.split(".")[0] + "." + fmt), image)

    print('Done.')


def main():
    # Get arguments.
    args = parse_args()
    # Upscale coco.
    upscale_coco(args.inputs, args.outputs, args.size, args.format)

main()
