#!/bin/bash
# Copyright (c) 2020-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

import sys
import errno
import json
import os
import numpy as np
from argparse import ArgumentParser
sys.path.insert(1, 'py-bindings')
from squad import SQUADConverter

def get_arguments():
    parser = ArgumentParser()
    parser.add_argument("--test_file", type=str, help="Path to squad test json file", required=True)
    parser.add_argument("--vocab_file", type=str, help="Path to vocab.txt file", required=True)
    parser.add_argument("--max_seq_length", type=int, help="Max sequence length", default=384)
    parser.add_argument("--num_samples", type=int, help="Number of samples to be converted", default=300)
    parser.add_argument("--max_query_length", type=int, help="Max query length", default=64)
    parser.add_argument("--doc_stride", type=int, help="Document stride", default=128)
    parser.add_argument("--lower_case", type=bool, help="Lower case", default=1)
    parser.add_argument("--output_dir", type=str, help="Output directory for saved raw files", default="samples_cache")
    parser.add_argument("--input_list_dir", type=str, help="Output directory for input_list", default="samples_cache")
    
    return parser.parse_args()


def main():

    args = get_arguments()
    if not os.path.isfile(args.test_file):
        raise FileNotFoundError(errno.ENOENT, os.strerror(errno.ENOENT), args.test_file)

    if not os.path.isfile(args.vocab_file):
        raise FileNotFoundError(errno.ENOENT, os.strerror(errno.ENOENT), args.vocab_file)
    
    os.makedirs(args.output_dir, exist_ok=True)
    sqd = SQUADConverter(args.test_file, args.vocab_file, args.max_seq_length, args.max_query_length, args.doc_stride, args.lower_case)
    
    # Convert examples and generate input_list
    input_file_name= "input_list.txt"
    file=open(os.path.join(args.input_list_dir,input_file_name), "w")
    samples = sqd.convert()
    for i in range(args.num_samples):
        single_sample= samples[i]
        input_mask = single_sample["input_mask"]
        input_ids = single_sample["input_ids"]
        segment_ids = single_sample["segment_ids"]
        np_mask = np.asarray(input_mask).astype(np.float32)
        file_name_1_prefix = "input_mask:0:="
        file_name_1 = "input_mask_{}.raw".format(i)
        np_mask.astype(np.float32).tofile(os.path.join(args.output_dir,file_name_1))
        np_ids = np.asarray(input_ids).astype(np.float32)
        file_name_2_prefix = "bert/embeddings/ExpandDims:0:="
        file_name_2 = "input_ids_{}.raw".format(i)
        np_ids.astype(np.float32).tofile(os.path.join(args.output_dir,file_name_2))
        np_segment = np.asarray(segment_ids).astype(np.float32)
        file_name_3_prefix = "segment_ids:0:="
        file_name_3 = "segment_ids_{}.raw".format(i)
        np_segment.astype(np.float32).tofile(os.path.join(args.output_dir,file_name_3))
        file.write("{} {} {}\n".format(file_name_2_prefix + file_name_2,file_name_1_prefix + file_name_1,file_name_3_prefix + file_name_3))
    file.close()

if __name__=="__main__":
    main()
