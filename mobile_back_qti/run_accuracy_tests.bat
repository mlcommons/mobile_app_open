@echo off
REM ##########################################################################
REM # Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
REM #
REM # Licensed under the Apache License, Version 2.0 (the "License");
REM # you may not use this file except in compliance with the License.
REM # You may obtain a copy of the License at
REM #
REM #     http://www.apache.org/licenses/LICENSE-2.0
REM #
REM # Unless required by applicable law or agreed to in writing, software
REM # distributed under the License is distributed on an "AS IS" BASIS,
REM # WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
REM # See the License for the specific language governing permissions and
REM # limitations under the License.
REM ##########################################################################
set /A cooldown_period=5
set /A min_query=1000
set /A min_duration_ms=60000
set results_prefix=accuracy_results_
set results_suffix=.txt
set test_case_suffix=_accuracy_logs
set results_file=accuracy_results.txt
del %results_file%
set dataset_path=""
set models_path=""

:loop
IF NOT "%1"=="" (
    IF "%1"=="--dataset" (
        SET dataset_path=%2
        SHIFT
    )
    IF "%1"=="--models" (
        SET models_path=%2
        SHIFT
    )
    SHIFT
    GOTO :loop
)

if %dataset_path%=="" (
    GOTO :dataset_end
)

if %models_path%=="" (
    GOTO :models_end
)

set test_case=image_classification
mkdir %test_case%%test_case_suffix%
set use_case_results_file=%results_prefix%%test_case%%results_suffix%
.\main.exe EXTERNAL imagenet --mode=AccuracyOnly --images_directory=%dataset_path%\imagenet\img --offset=1 --output_dir=%test_case%%test_case_suffix% --min_query_count=%min_query% --min_duration_ms=%min_duration_ms% --single_stream_expected_latency_ns=1000000 --groundtruth_file=%dataset_path%\imagenet\imagenet_val_full.txt --model_file=%models_path%\mobilenet_edgetpu_224_1.0_htp.dlc --lib_path=libqtibackend.dll --native_lib_path=. > %use_case_results_file% 2>&1
echo #######%test_case%###### >> %results_file%
findstr "Accuracy" %use_case_results_file% >> %results_file%
timeout /t %cooldown_period% /nobreak

set test_case=object_detection
mkdir %test_case%%test_case_suffix%
set use_case_results_file=%results_prefix%%test_case%%results_suffix%
.\main.exe EXTERNAL coco --mode=AccuracyOnly --images_directory=%dataset_path%\coco\img --offset=1   --output_dir=%test_case%%test_case_suffix% --min_query_count=%min_query% --min_duration_ms=%min_duration_ms% --single_stream_expected_latency_ns=1000000 --groundtruth_file=%dataset_path%\coco\coco_val_full.pbtxt --model_file=%models_path%\ssd_mobiledet_qat_htp.dlc --lib_path=libqtibackend.dll --native_lib_path=. --num_classes=91 > %use_case_results_file% 2>&1
echo #######%test_case%###### >> %results_file%
findstr "Accuracy" %use_case_results_file% >> %results_file%
timeout /t %cooldown_period% /nobreak

set test_case=image_segmentation
mkdir %test_case%%test_case_suffix%
set use_case_results_file=%results_prefix%%test_case%%results_suffix%
.\main.exe EXTERNAL ade20k --mode=AccuracyOnly --images_directory=%dataset_path%\ade20k\images --num_class=31 --output_dir=%test_case%%test_case_suffix% --min_query_count=%min_query% --min_duration_ms=%min_duration_ms% --single_stream_expected_latency_ns=1000000 --ground_truth_directory=%dataset_path%\ade20k\annotations --model_file=%models_path%\mobile_mosaic_htp.dlc --lib_path=libqtibackend.dll --native_lib_path=. > %use_case_results_file% 2>&1
echo #######%test_case%###### >> %results_file%
findstr "Accuracy" %use_case_results_file% >> %results_file%
timeout /t %cooldown_period% /nobreak

set test_case=language_understanding
mkdir %test_case%%test_case_suffix%
set use_case_results_file=%results_prefix%%test_case%%results_suffix%
.\main.exe EXTERNAL squad --mode=AccuracyOnly --input_file=%dataset_path%\squad\squad_eval.tfrecord --output_dir=%test_case%%test_case_suffix% --min_query_count=%min_query% --min_duration_ms=%min_duration_ms% --single_stream_expected_latency_ns=1000000 --groundtruth_file=%dataset_path%\squad\squad_groundtruth.tfrecord --model_file=%models_path%\mobilebert_quantized_htp.dlc --lib_path=libqtibackend.dll --native_lib_path=. > %use_case_results_file% 2>&1
echo #######%test_case%###### >> %results_file%
findstr "Accuracy" %use_case_results_file% >> %results_file%
timeout /t %cooldown_period% /nobreak

set test_case=super_resolution
mkdir %test_case%%test_case_suffix%
set use_case_results_file=%results_prefix%%test_case%%results_suffix%
.\main.exe EXTERNAL SNUSR --mode=AccuracyOnly --images_directory=%dataset_path%\snusr\lr --output_dir=%test_case%%test_case_suffix% --min_query_count=%min_query% --min_duration_ms=%min_duration_ms% --single_stream_expected_latency_ns=1000000 --ground_truth_directory=%dataset_path%\snusr\hr --model_file=%models_path%\snusr_htp.dlc --lib_path=libqtibackend.dll --native_lib_path=. > %use_case_results_file% 2>&1
echo #######%test_case%###### >> %results_file%
findstr "Accuracy" %use_case_results_file% >> %results_file%
timeout /t %cooldown_period% /nobreak

set test_case=image_classification_offline
mkdir %test_case%%test_case_suffix%
set use_case_results_file=%results_prefix%%test_case%%results_suffix%
.\main.exe EXTERNAL imagenet --mode=AccuracyOnly --scenario=Offline --batch_size=12288 --images_directory=%dataset_path%\imagenet\img --offset=1 --output_dir=%test_case%%test_case_suffix% --min_query_count=24576 --min_duration_ms=0 --single_stream_expected_latency_ns=1000000 --groundtruth_file=%dataset_path%\imagenet\imagenet_val_full.txt --model_file=%models_path%\mobilenet_edgetpu_224_1.0_htp_batched_8.dlc --lib_path=libqtibackend.dll --native_lib_path=. > %use_case_results_file%  2>&1
echo #######%test_case%###### >> %results_file%
findstr "Accuracy" %use_case_results_file% >> %results_file%

:dataset_end
echo "set dataset path using --dataset"

:models_end
echo "set models path using --models"
