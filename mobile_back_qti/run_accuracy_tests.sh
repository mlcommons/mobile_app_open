#!/bin/bash
##########################################################################
# Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

export cooldown_period=5
export min_query=1000
export min_duration=60000
export results_prefix=accuracy_results_
export results_suffix=.txt
export test_case_suffix=_accuracy_logs
export results_file=accuracy_results.txt
rm $results_file
export dataset_path=""
export models_path=""
export LD_LIBRARY_PATH=.

while [[ $# -gt 0 ]]
do
  if [[ "$1" == "--dataset" ]]
  then
    export dataset_path=$2
    shift 1
  fi
  if [[ "$1" == "--models" ]]
  then
    export models_path=$2
    shift 1
  fi
  shift 1
done

if [[ "$dataset_path" == "" ]]
then
    echo "set dataset path using --dataset"
    exit 1
fi

if [[ "$models_path" == "" ]]
then
    echo "set models path using --models"
    exit 1
fi

export test_case=image_classification
mkdir -p $test_case$test_case_suffix
export use_case_results_file=$results_prefix$test_case$results_suffix
./main EXTERNAL imagenet --mode=AccuracyOnly --images_directory=$dataset_path/imagenet/img --offset=1 --output_dir=$test_case$test_case_suffix --min_query_count=$min_query --min_duration=$min_duration --single_stream_expected_latency_ns=1000000 --groundtruth_file=$dataset_path/imagenet/imagenet_val_full.txt --model_file=$models_path/mobilenet_edgetpu_224_1.0_htp.dlc --lib_path=libqtibackend.so --native_lib_path=. > $use_case_results_file 2>&1
echo "#######$test_case######" >> $results_file
grep "Accuracy" $use_case_results_file >> $results_file
sleep $cooldown_period

export test_case=object_detection
mkdir -p $test_case$test_case_suffix
export use_case_results_file=$results_prefix$test_case$results_suffix
./main EXTERNAL coco --mode=AccuracyOnly --images_directory=$dataset_path/coco/img --offset=1 --output_dir=$test_case$test_case_suffix --min_query_count=$min_query --min_duration=$min_duration --single_stream_expected_latency_ns=1000000 --groundtruth_file=$dataset_path/coco/coco_val_full.pbtxt --model_file=$models_path/ssd_mobiledet_qat_htp.dlc --lib_path=libqtibackend.so --native_lib_path=. --num_classes=91 > $use_case_results_file 2>&1
echo "#######$test_case######" >> $results_file
grep "Accuracy" $use_case_results_file >> $results_file
sleep $cooldown_period

export test_case=image_segmentation
mkdir -p $test_case$test_case_suffix
export use_case_results_file=$results_prefix$test_case$results_suffix
./main EXTERNAL ade20k --mode=AccuracyOnly --images_directory=$dataset_path/ade20k/images --num_class=31 --output_dir=$test_case$test_case_suffix --min_query_count=$min_query --min_duration=$min_duration --single_stream_expected_latency_ns=1000000 --ground_truth_directory=$dataset_path/ade20k/annotations --model_file=$models_path/mobile_mosaic_htp.dlc --lib_path=libqtibackend.so --native_lib_path=. > $use_case_results_file 2>&1
echo "#######$test_case######" >> $results_file
grep "Accuracy" $use_case_results_file >> $results_file
sleep $cooldown_period

export test_case=language_understanding
mkdir -p $test_case$test_case_suffix
export use_case_results_file=$results_prefix$test_case$results_suffix
./main EXTERNAL squad --mode=AccuracyOnly --input_file=$dataset_path/squad/squad_eval.tfrecord --output_dir=$test_case$test_case_suffix --min_query_count=$min_query --min_duration=$min_duration --single_stream_expected_latency_ns=1000000 --groundtruth_file=$dataset_path/squad/squad_groundtruth.tfrecord --model_file=$models_path/mobilebert_quantized_htp.dlc --lib_path=libqtibackend.so --native_lib_path=. > $use_case_results_file 2>&1
echo "#######$test_case######" >> $results_file
grep "Accuracy" $use_case_results_file >> $results_file
sleep $cooldown_period

export test_case=super_resolution
mkdir -p $test_case$test_case_suffix
export use_case_results_file=$results_prefix$test_case$results_suffix
./main EXTERNAL SNUSR --mode=AccuracyOnly --images_directory=$dataset_path/snusr/lr --output_dir=$test_case$test_case_suffix --min_query_count=$min_query --min_duration=$min_duration --single_stream_expected_latency_ns=1000000 --ground_truth_directory=$dataset_path/snusr/hr --model_file=$models_path/snusr_htp.dlc --lib_path=libqtibackend.so --native_lib_path=. > $use_case_results_file 2>&1
echo "#######$test_case######" >> $results_file
grep "Accuracy" $use_case_results_file >> $results_file
sleep $cooldown_period

export test_case=image_classification_offline
mkdir -p $test_case$test_case_suffix
export use_case_results_file=$results_prefix$test_case$results_suffix
./main EXTERNAL imagenet --mode=AccuracyOnly --scenario=Offline --batch_size=12288 --images_directory=$dataset_path/imagenet/img --offset=1 --output_dir=$test_case$test_case_suffix --min_query_count=24576 --min_duration=0 --single_stream_expected_latency_ns=1000000 --groundtruth_file=$dataset_path/imagenet/imagenet_val_full.txt --model_file=$models_path/mobilenet_edgetpu_224_1.0_htp_batched_sd8pg1.dlc --lib_path=libqtibackend.so --native_lib_path=. > $use_case_results_file 2>&1
echo "#######$test_case######" >> $results_file
grep "Accuracy" $use_case_results_file >> $results_file
