#!/bin/bash

##########################################################################
# Copyright (c) 2020-2025 Qualcomm Innovation Center, Inc. All rights reserved.
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


export min_query=1000
export min_duration_ms=60000
export results_suffix=.txt
export dataset_path=""
export models_path=""
export usecase_name=""
export mode=""
export LD_LIBRARY_PATH=.

# Below are the arguments and values to be passed to script in any order
# use --dataset argument to pass dataset path as value
# use --models argument to pass models path as value
# use --mode argument to run in performance or accuracy mode. Defaults to performance mode.
# valid values for --mode argument: performance, accuracy.
# use --usecase argument to pass name of usecase to run as value (if not mentioned, by default runs all 8 usecases)
# valid values for --usecase argument: image_classification_v2, object_detection, image_segmentation, language_understanding, super_resolution, image_classification_offline_v2

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
  if [[ "$1" == "--usecase" ]]
  then
    export usecase_name=$2
    shift 1
  fi
  if [[ "$1" == "--mode" ]]
  then
    export mode=$2
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

if [[ "$mode" == "performance" || "$mode" == "" ]]
then
    echo "Running in Performance (default) mode. Switch to accuracy mode using --mode accuracy"
    export results_prefix=performance_results_
    export test_case_suffix=_performance_logs
    export results_file=performance_results.txt
    export cooldown_period=300
fi

if [[ "$mode" == "accuracy" ]]
then
    echo "Running in Accuracy mode"
    export results_prefix=accuracy_results_
    export test_case_suffix=_accuracy_logs
    export results_file=accuracy_results.txt
    export cooldown_period=5
fi

rm $results_file

####### Performance usecase functions #######

image_classification_v2_performance(){
echo "####### Performance:: Image classification V2 in progress #######"
export test_case=image_classification_v2
mkdir -p $test_case$test_case_suffix
export use_case_results_file=$results_prefix$test_case$results_suffix
./main EXTERNAL $test_case --mode=PerformanceOnly --images_directory=$dataset_path/imagenet/img --offset=0 --output_dir=$test_case$test_case_suffix --min_query_count=$min_query --min_duration_ms=$min_duration_ms --single_stream_expected_latency_ns=1000000 --groundtruth_file="" --model_file=$models_path/mobilenet_v4_htp.dlc --lib_path=libqtibackend.so --native_lib_path=. > $use_case_results_file 2>&1
echo "#######$test_case######" >> $results_file
grep "90th percentile latency (ns)" $use_case_results_file >> $results_file
grep "Result is" $use_case_results_file >> $results_file
grep "QPS w/o loadgen overhead" $use_case_results_file >> $results_file
echo "####### Image classification V2 is complete #######"
}

object_detection_performance(){
echo "####### Performance:: Object detection in progress #######"
export test_case=object_detection
mkdir -p $test_case$test_case_suffix
export use_case_results_file=$results_prefix$test_case$results_suffix
./main EXTERNAL $test_case --mode=PerformanceOnly --images_directory=$dataset_path/coco/img --offset=1 --output_dir=$test_case$test_case_suffix --min_query_count=$min_query --min_duration_ms=$min_duration_ms --single_stream_expected_latency_ns=1000000 --groundtruth_file="" --model_file=$models_path/ssd_mobiledet_qat_htp.dlc --lib_path=libqtibackend.so --native_lib_path=. --num_classes=91 > $use_case_results_file 2>&1
echo "#######$test_case######" >> $results_file
grep "90th percentile latency (ns)" $use_case_results_file >> $results_file
grep "Result is" $use_case_results_file >> $results_file
grep "QPS w/o loadgen overhead" $use_case_results_file >> $results_file
echo "####### Object detection is complete #######"
}

image_segmentation_performance(){
echo "####### Performance:: Image segmentation in progress #######"
export test_case=image_segmentation_v2
mkdir -p $test_case$test_case_suffix
export use_case_results_file=$results_prefix$test_case$results_suffix
./main EXTERNAL $test_case --mode=PerformanceOnly --images_directory=$dataset_path/ade20k/images --num_class=31 --output_dir=$test_case$test_case_suffix --min_query_count=$min_query --min_duration_ms=$min_duration_ms --single_stream_expected_latency_ns=1000000 --ground_truth_directory= --model_file=$models_path/mobile_mosaic_htp.dlc --lib_path=libqtibackend.so --native_lib_path=. > $use_case_results_file 2>&1
echo "#######$test_case######" >> $results_file
grep "90th percentile latency (ns)" $use_case_results_file >> $results_file
grep "Result is" $use_case_results_file >> $results_file
grep "QPS w/o loadgen overhead" $use_case_results_file >> $results_file
echo "####### Image segmentation is complete #######"
}

language_understanding_performance(){
echo "####### Performance:: Natural language processing in progress #######"
export test_case=natural_language_processing
mkdir -p $test_case$test_case_suffix
export use_case_results_file=$results_prefix$test_case$results_suffix
./main EXTERNAL $test_case --mode=PerformanceOnly --input_file=$dataset_path/squad/squad_eval_mini.tfrecord --output_dir=$test_case$test_case_suffix --min_query_count=$min_query --min_duration_ms=$min_duration_ms --single_stream_expected_latency_ns=1000000 --groundtruth_file= --model_file=$models_path/mobilebert_quantized_htp.dlc --lib_path=libqtibackend.so --native_lib_path=. > $use_case_results_file 2>&1
echo "#######$test_case######" >> $results_file
grep "90th percentile latency (ns)" $use_case_results_file >> $results_file
grep "Result is" $use_case_results_file >> $results_file
grep "QPS w/o loadgen overhead" $use_case_results_file >> $results_file
echo "####### Natural language processing is complete #######"
}

super_resolution_performance(){
echo "####### Performance:: Super resolution in progress #######"
export test_case=super_resolution
mkdir -p $test_case$test_case_suffix
export use_case_results_file=$results_prefix$test_case$results_suffix
./main EXTERNAL $test_case --mode=PerformanceOnly --images_directory=$dataset_path/snusr/lr --output_dir=$test_case$test_case_suffix --min_query_count=$min_query --min_duration_ms=$min_duration_ms --single_stream_expected_latency_ns=1000000 --ground_truth_directory= --model_file=$models_path/snusr_htp.dlc --lib_path=libqtibackend.so --native_lib_path=. > $use_case_results_file 2>&1
echo "#######$test_case######" >> $results_file
grep "90th percentile latency (ns)" $use_case_results_file >> $results_file
grep "Result is" $use_case_results_file >> $results_file
grep "QPS w/o loadgen overhead" $use_case_results_file >> $results_file
echo "####### Super resolution is complete #######"
}

image_classification_offline_v2_performance(){
echo "####### Performance:: Image classification offline V2 in progress #######"
export test_case=image_classification_offline_v2
mkdir -p $test_case$test_case_suffix
export use_case_results_file=$results_prefix$test_case$results_suffix
./main EXTERNAL $test_case --mode=PerformanceOnly --scenario=Offline --batch_size=12288 --images_directory=$dataset_path/imagenet/img --offset=0 --output_dir=$test_case$test_case_suffix --min_query_count=24576 --min_duration_ms=0 --single_stream_expected_latency_ns=1000000 --groundtruth_file= --model_file=$models_path/mobilenet_v4_htp_batched_4.dlc --lib_path=libqtibackend.so --native_lib_path=. > $use_case_results_file 2>&1
echo "#######$test_case######" >> $results_file
grep "Result is" $use_case_results_file >> $results_file
grep "Samples per second" $use_case_results_file >> $results_file
echo "####### Image classification offline V2 is complete #######"
}

stable_diffusion_performance(){
echo "####### Performance:: Stable diffusion in progress #######"
export test_case=stable_diffusion
mkdir -p $test_case$test_case_suffix
export use_case_results_file=$results_prefix$test_case$results_suffix
./main EXTERNAL $test_case --mode=PerformanceOnly --input_tfrecord=$dataset_path/stable_diffusion/coco_gen_full.tfrecord --output_dir=$test_case$test_case_suffix --min_query_count=1024 --min_duration_ms=60000 --max_duration_ms=300000 --single_stream_expected_latency_ns=1000000  --model_file=$models_path/stable_diffusion --lib_path=libqtibackend.so --native_lib_path=. > $use_case_results_file 2>&1
echo "#######$test_case######" >> $results_file
grep "90th percentile latency (ns)" $use_case_results_file >> $results_file
grep "Result is" $use_case_results_file >> $results_file
grep "QPS w/o loadgen overhead" $use_case_results_file >> $results_file
echo "####### Stable Diffusion is complete #######"
}

####### Accuracy usecase functions #######

image_classification_v2_accuracy(){
echo "####### Accuracy:: Image classification V2 in progress #######"
export test_case=image_classification_v2
mkdir -p $test_case$test_case_suffix
export use_case_results_file=$results_prefix$test_case$results_suffix
./main EXTERNAL $test_case --mode=AccuracyOnly --images_directory=$dataset_path/imagenet/img --offset=0 --output_dir=$test_case$test_case_suffix --min_query_count=$min_query --min_duration_ms=$min_duration_ms --single_stream_expected_latency_ns=1000000 --groundtruth_file=$dataset_path/imagenet/imagenet_val_full.txt --model_file=$models_path/mobilenet_v4_htp.dlc --lib_path=libqtibackend.so --native_lib_path=. > $use_case_results_file 2>&1
echo "#######$test_case######" >> $results_file
grep "Accuracy" $use_case_results_file >> $results_file
echo "####### Image classification V2 is complete #######"
}

object_detection_accuracy(){
echo "####### Accuracy:: Object detection in progress #######"
export test_case=object_detection
mkdir -p $test_case$test_case_suffix
export use_case_results_file=$results_prefix$test_case$results_suffix
./main EXTERNAL $test_case --mode=AccuracyOnly --images_directory=$dataset_path/coco/img --offset=1 --output_dir=$test_case$test_case_suffix --min_query_count=$min_query --min_duration_ms=$min_duration_ms --single_stream_expected_latency_ns=1000000 --groundtruth_file=$dataset_path/coco/coco_val_full.pbtxt --model_file=$models_path/ssd_mobiledet_qat_htp.dlc --lib_path=libqtibackend.so --native_lib_path=. --num_classes=91 > $use_case_results_file 2>&1
echo "#######$test_case######" >> $results_file
grep "Accuracy" $use_case_results_file >> $results_file
echo "####### Object detection is complete #######"
}

image_segmentation_accuracy(){
echo "####### Accuracy:: Image segmentation in progress #######"
export test_case=image_segmentation_v2
mkdir -p $test_case$test_case_suffix
export use_case_results_file=$results_prefix$test_case$results_suffix
./main EXTERNAL $test_case --mode=AccuracyOnly --images_directory=$dataset_path/ade20k/images --num_class=31 --output_dir=$test_case$test_case_suffix --min_query_count=$min_query --min_duration_ms=$min_duration_ms --single_stream_expected_latency_ns=1000000 --ground_truth_directory=$dataset_path/ade20k/annotations --model_file=$models_path/mobile_mosaic_htp.dlc --lib_path=libqtibackend.so --native_lib_path=. > $use_case_results_file 2>&1
echo "#######$test_case######" >> $results_file
grep "Accuracy" $use_case_results_file >> $results_file
echo "####### Image segmentation is complete #######"
}

language_understanding_accuracy(){
echo "####### Accuracy:: Natural language processing in progress #######"
export test_case=natural_language_processing
mkdir -p $test_case$test_case_suffix
export use_case_results_file=$results_prefix$test_case$results_suffix
./main EXTERNAL $test_case --mode=AccuracyOnly --input_file=$dataset_path/squad/squad_eval_mini.tfrecord --output_dir=$test_case$test_case_suffix --min_query_count=$min_query --min_duration_ms=$min_duration_ms --single_stream_expected_latency_ns=1000000 --groundtruth_file=$dataset_path/squad/squad_groundtruth.tfrecord --model_file=$models_path/mobilebert_quantized_htp.dlc --lib_path=libqtibackend.so --native_lib_path=. > $use_case_results_file 2>&1
echo "#######$test_case######" >> $results_file
grep "Accuracy" $use_case_results_file >> $results_file
echo "####### Natural language processing is complete #######"
}

super_resolution_accuracy(){
echo "####### Accuracy:: Super resolution in progress #######"
export test_case=super_resolution
mkdir -p $test_case$test_case_suffix
export use_case_results_file=$results_prefix$test_case$results_suffix
./main EXTERNAL $test_case --mode=AccuracyOnly --images_directory=$dataset_path/snusr/lr --output_dir=$test_case$test_case_suffix --min_query_count=$min_query --min_duration_ms=$min_duration_ms --single_stream_expected_latency_ns=1000000 --ground_truth_directory=$dataset_path/snusr/hr --model_file=$models_path/snusr_htp.dlc --lib_path=libqtibackend.so --native_lib_path=. > $use_case_results_file 2>&1
echo "#######$test_case######" >> $results_file
grep "Accuracy" $use_case_results_file >> $results_file
echo "####### Super resolution is complete #######"
}

image_classification_offline_v2_accuracy(){
echo "####### Accuracy:: Image classification offline V2 in progress #######"
export test_case=image_classification_offline_v2
mkdir -p $test_case$test_case_suffix
export use_case_results_file=$results_prefix$test_case$results_suffix
./main EXTERNAL $test_case --mode=AccuracyOnly --scenario=Offline --batch_size=12288 --images_directory=$dataset_path/imagenet/img --offset=0 --output_dir=$test_case$test_case_suffix --min_query_count=24576 --min_duration_ms=0 --single_stream_expected_latency_ns=1000000 --groundtruth_file=$dataset_path/imagenet/imagenet_val_full.txt --model_file=$models_path/mobilenet_v4_htp_batched_4.dlc --lib_path=libqtibackend.so --native_lib_path=. > $use_case_results_file 2>&1
echo "#######$test_case######" >> $results_file
grep "Accuracy" $use_case_results_file >> $results_file
echo "####### Image classification offline V2 is complete #######"
}

stable_diffusion_accuracy(){
echo "####### Accuracy:: Stable diffusion in progress #######"
export test_case=stable_diffusion
mkdir -p $test_case$test_case_suffix
export use_case_results_file=$results_prefix$test_case$results_suffix
./main EXTERNAL $test_case --mode=AccuracyOnly --input_tfrecord=$dataset_path/stable_diffusion/coco_gen_test.tfrecord --input_clip_model=$models_path/stable_diffusion/clip_model_512x512.tflite --output_dir=$test_case$test_case_suffix --min_query_count=100 --min_duration_ms=0 --single_stream_expected_latency_ns=1000000  --model_file=$models_path/stable_diffusion --lib_path=libqtibackend.so --native_lib_path=. > $use_case_results_file 2>&1
echo "#######$test_case######" >> $results_file
grep "Accuracy" $use_case_results_file >> $results_file
echo "####### Stable Diffusion is complete #######"
}

if [[ "$mode" == "performance" || "$mode" == "" ]]
then
case $usecase_name in
  "image_classification_v2")
    image_classification_v2_performance
    ;;
  "object_detection")
    object_detection_performance
    ;;
  "image_segmentation")
    image_segmentation_performance
    ;;
  "language_understanding")
    language_understanding_performance
    ;;
  "super_resolution")
    super_resolution_performance
    ;;
  "image_classification_offline_v2")
    image_classification_offline_v2_performance
    ;;
  "stable_diffusion")
    stable_diffusion_performance
    ;;
  *)
    image_classification_v2_performance
    echo "## cooldown intitated ##"
    sleep $cooldown_period
    object_detection_performance
    echo "## cooldown intitated ##"
    sleep $cooldown_period
    image_segmentation_performance
    echo "## cooldown intitated ##"
    sleep $cooldown_period
    language_understanding_performance
    echo "## cooldown intitated ##"
    sleep $cooldown_period
    super_resolution_performance
    echo "## cooldown intitated ##"
    sleep $cooldown_period
    image_classification_offline_v2_performance
    echo "## cooldown intitated ##"
    sleep $cooldown_period
    stable_diffusion_performance
    ;;
esac
fi

if [[ "$mode" == "accuracy" ]]
then
case $usecase_name in
  "image_classification_v2")
    image_classification_v2_accuracy
    ;;
  "object_detection")
    object_detection_accuracy
    ;;
  "image_segmentation")
    image_segmentation_accuracy
    ;;
  "language_understanding")
    language_understanding_accuracy
    ;;
  "super_resolution")
    super_resolution_accuracy
    ;;
  "image_classification_offline_v2")
    image_classification_offline_v2_accuracy
    ;;
  "stable_diffusion")
    stable_diffusion_accuracy
    ;;
  *)
    image_classification_v2_accuracy
    echo "## cooldown intitated ##"
    sleep $cooldown_period
    object_detection_accuracy
    echo "## cooldown intitated ##"
    sleep $cooldown_period
    image_segmentation_accuracy
    echo "## cooldown intitated ##"
    sleep $cooldown_period
    language_understanding_accuracy
    echo "## cooldown intitated ##"
    sleep $cooldown_period
    super_resolution_accuracy
    echo "## cooldown intitated ##"
    sleep $cooldown_period
    image_classification_offline_v2_accuracy
    echo "## cooldown intitated ##"
    sleep $cooldown_period
    stable_diffusion_accuracy
    ;;
esac
fi

