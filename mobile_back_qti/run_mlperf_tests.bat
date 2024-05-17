@echo off
REM ##########################################################################
REM # Copyright (c) 2020-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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


set /A min_query=1000
set /A min_duration_ms=60000
set results_suffix=.txt
set dataset_path=""
set models_path=""
set usecase_name=""
set appmode=""
set results_prefix=""
set test_case_suffix=""
set results_file=""
set cooldown_period=""

rem # Below are the arguments and values to be passed to script in any order
rem # use --dataset argument to pass dataset path as value
rem # use --models argument to pass models path as value
rem # use --mode argument to run in performance or accuracy mode. Defaults to performance mode.
rem # valid values for --mode argument: performance, accuracy.
rem # use --usecase argument to pass name of usecase to run as value (if not mentioned, by default runs all 8 usecases)
rem # valid values for --usecase argument: image_classification_v2, image_classification, object_detection, image_segmentation, language_understanding, super_resolution, image_classification_offline_v2, image_classification_offline

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
    IF "%1"=="--usecase" (
        SET usecase_name=%2
        SHIFT
    )
    IF "%1"=="--mode" (
        SET appmode=%2
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

if %appmode%=="" (
    set appmode=performance
)

if %appmode%==performance (
    echo "Running in Performance (default) mode. Switch to accuracy mode using --mode accuracy"
    set results_prefix=performance_results_
    set test_case_suffix=_performance_logs
    set results_file=performance_results.txt
    set cooldown_period=300
)

if %appmode%==accuracy (
    echo "Running in Accuracy mode."
    set results_prefix=accuracy_results_
    set test_case_suffix=_accuracy_logs
    set results_file=accuracy_results.txt
    set cooldown_period=5
)

del %results_file%

rem #### Performance usecase switch ####

if %appmode%==performance (
IF "%usecase_name%"=="image_classification_v2" (
    call :image_classification_v2_performance
    goto :eof
)
IF "%usecase_name%"=="object_detection" (
    call :object_detection_performance
    goto :eof
)
IF "%usecase_name%"=="image_segmentation" (
    call :image_segmentation_performance
    goto :eof
)
IF "%usecase_name%"=="super_resolution" (
    call :super_resolution_performance
    goto :eof
)
IF "%usecase_name%"=="language_understanding" (
    call :language_understanding_performance
    goto :eof
)
IF "%usecase_name%"=="image_classification_offline_v2" (
    call :image_classification_offline_v2_performance
    goto :eof
)
IF "%usecase_name%"=="image_classification" (
    call :image_classification_performance
    goto :eof
)
IF "%usecase_name%"=="image_classification_offline" (
    call :image_classification_offline_performance
    goto :eof
)
IF  %usecase_name%=="" (
    call :image_classification_v2_performance
    echo ## cooldown intitated ##
    timeout /t %cooldown_period% /nobreak
    call :object_detection_performance
    echo ## cooldown intitated ##
    timeout /t %cooldown_period% /nobreak
    call :image_segmentation_performance
    echo ## cooldown intitated ##
    timeout /t %cooldown_period% /nobreak
    call :language_understanding_performance
    echo ## cooldown intitated ##
    timeout /t %cooldown_period% /nobreak
    call :super_resolution_performance
    echo ## cooldown intitated ##
    timeout /t %cooldown_period% /nobreak
    call :image_classification_offline_v2_performance
    echo ## cooldown intitated ##
    timeout /t %cooldown_period% /nobreak
    call :image_classification_performance
    echo ## cooldown intitated ##
    timeout /t %cooldown_period% /nobreak
    call :image_classification_offline_performance
    goto :eof
)
)

rem #### Accuracy usecase switch ####
if %appmode%==accuracy (
IF "%usecase_name%"=="image_classification_v2" (
    call :image_classification_v2_accuracy
    goto :eof
)
IF "%usecase_name%"=="object_detection" (
    call :object_detection_accuracy
    goto :eof
)
IF "%usecase_name%"=="image_segmentation" (
    CALL :image_segmentation_accuracy
    goto :eof
)
IF "%usecase_name%"=="super_resolution" (
    call :super_resolution_accuracy
    goto :eof
)
IF "%usecase_name%"=="language_understanding" (
    call :language_understanding_accuracy
    goto :eof
)
IF "%usecase_name%"=="image_classification_offline_v2" (
    call :image_classification_offline_v2_accuracy
    goto :eof
)
IF "%usecase_name%"=="image_classification" (
    call :image_classification_accuracy
    goto :eof
)
IF "%usecase_name%"=="image_classification_offline" (
    call :image_classification_offline_accuracy
    goto :eof
)
IF  %usecase_name%=="" (
    call :image_classification_v2_accuracy
    echo ## cooldown intitated ##
    timeout /t %cooldown_period% /nobreak
    call :object_detection_accuracy
    echo ## cooldown intitated ##
    timeout /t %cooldown_period% /nobreak
    call :image_segmentation_accuracy
    echo ## cooldown intitated ##
    timeout /t %cooldown_period% /nobreak
    call :language_understanding_accuracy
    echo ## cooldown intitated ##
    timeout /t %cooldown_period% /nobreak
    call :super_resolution_accuracy
    echo ## cooldown intitated ##
    timeout /t %cooldown_period% /nobreak
    call :image_classification_offline_v2_accuracy
    echo ## cooldown intitated ##
    timeout /t %cooldown_period% /nobreak
    call :image_classification_accuracy
    echo ## cooldown intitated ##
    timeout /t %cooldown_period% /nobreak
    call :image_classification_offline_accuracy
    goto :eof
)
)

goto :eof

rem ####### Performance usecase functions #######

:image_classification_v2_performance
echo ####### Performance:: Image classification V2 in progress #######
set test_case=image_classification_v2
mkdir %test_case%%test_case_suffix%
set use_case_results_file=%results_prefix%%test_case%%results_suffix%
.\main.exe EXTERNAL %test_case% --mode=PerformanceOnly --images_directory=%dataset_path%\imagenet\img --offset=0 --output_dir=%test_case%%test_case_suffix% --min_query_count=%min_query% --min_duration_ms=%min_duration_ms% --single_stream_expected_latency_ns=1000000 --groundtruth_file=%dataset_path% --model_file=%models_path%\mobilenet_v4_htp.dlc --lib_path=libqtibackend.dll --native_lib_path=. > %use_case_results_file% 2>&1
echo #######%test_case%###### >> %results_file%
findstr /C:"90th percentile latency (ns)" %use_case_results_file% >> %results_file%
findstr /C:"Result is" %use_case_results_file% >> %results_file%
findstr /C:"QPS w/o loadgen overhead" %use_case_results_file% >> %results_file%
echo ####### Image classification V2 is complete #######
EXIT /B 0

:object_detection_performance
echo ####### Performance:: Object detection in progress #######
set test_case=object_detection
mkdir %test_case%%test_case_suffix%
set use_case_results_file=%results_prefix%%test_case%%results_suffix%
.\main.exe EXTERNAL %test_case% --mode=PerformanceOnly --images_directory=%dataset_path%\coco\img --offset=1   --output_dir=%test_case%%test_case_suffix% --min_query_count=%min_query% --min_duration_ms=%min_duration_ms% --single_stream_expected_latency_ns=1000000 --groundtruth_file=%dataset_path% --model_file=%models_path%\ssd_mobiledet_qat_htp.dlc --lib_path=libqtibackend.dll --native_lib_path=. --num_classes=91 > %use_case_results_file% 2>&1
echo #######%test_case%###### >> %results_file%
findstr /C:"90th percentile latency (ns)" %use_case_results_file% >> %results_file%
findstr /C:"Result is" %use_case_results_file% >> %results_file%
findstr /C:"QPS w/o loadgen overhead" %use_case_results_file% >> %results_file%
echo ####### Object detection is complete #######
EXIT /B 0

:image_segmentation_performance
echo ####### Performance:: Image segmentation in progress #######
set test_case=image_segmentation_v2
mkdir %test_case%%test_case_suffix%
set use_case_results_file=%results_prefix%%test_case%%results_suffix%
.\main.exe EXTERNAL %test_case% --mode=PerformanceOnly --images_directory=%dataset_path%\ade20k\images --num_class=31 --output_dir=%test_case%%test_case_suffix% --min_query_count=%min_query% --min_duration_ms=%min_duration_ms% --single_stream_expected_latency_ns=1000000 --ground_truth_directory= --model_file=%models_path%\mobile_mosaic_htp.dlc --lib_path=libqtibackend.dll --native_lib_path=. > %use_case_results_file% 2>&1
echo #######%test_case%###### >> %results_file%
findstr /C:"90th percentile latency (ns)" %use_case_results_file% >> %results_file%
findstr /C:"Result is" %use_case_results_file% >> %results_file%
findstr /C:"QPS w/o loadgen overhead" %use_case_results_file% >> %results_file%
timeout /t %cooldown_period% /nobreak
echo ####### Image segmentation is complete #######
EXIT /B 0

:language_understanding_performance
echo ####### Performance:: Natural language processing in progress #######
set test_case=natural_language_processing
mkdir %test_case%%test_case_suffix%
set use_case_results_file=%results_prefix%%test_case%%results_suffix%
.\main.exe EXTERNAL %test_case% --mode=PerformanceOnly --input_file=%dataset_path%\squad\squad_eval_mini.tfrecord --output_dir=%test_case%%test_case_suffix% --min_query_count=%min_query% --min_duration_ms=%min_duration_ms% --single_stream_expected_latency_ns=1000000 --groundtruth_file= --model_file=%models_path%\mobilebert_quantized_htp.dlc --lib_path=libqtibackend.dll --native_lib_path=. > %use_case_results_file% 2>&1
echo #######%test_case%###### >> %results_file%
findstr /C:"90th percentile latency (ns)" %use_case_results_file% >> %results_file%
findstr /C:"Result is" %use_case_results_file% >> %results_file%
findstr /C:"QPS w/o loadgen overhead" %use_case_results_file% >> %results_file%
echo ####### Natural language processing is complete #######
EXIT /B 0

:super_resolution_performance
echo ####### Performance:: Super resolution in progress #######
set test_case=super_resolution
mkdir %test_case%%test_case_suffix%
set use_case_results_file=%results_prefix%%test_case%%results_suffix%
.\main.exe EXTERNAL %test_case% --mode=PerformanceOnly --images_directory=%dataset_path%\snusr\lr --output_dir=%test_case%%test_case_suffix% --min_query_count=%min_query% --min_duration_ms=%min_duration_ms% --single_stream_expected_latency_ns=1000000 --ground_truth_directory=. --model_file=%models_path%\snusr_htp.dlc --lib_path=libqtibackend.dll --native_lib_path=. > %use_case_results_file% 2>&1
echo #######%test_case%###### >> %results_file%
findstr /C:"90th percentile latency (ns)" %use_case_results_file% >> %results_file%
findstr /C:"Result is" %use_case_results_file% >> %results_file%
findstr /C:"QPS w/o loadgen overhead" %use_case_results_file% >> %results_file%
echo ####### Super resolution is complete #######
EXIT /B 0

:image_classification_offline_v2_performance
echo ####### Performance:: Image classification offline V2 in progress #######
set test_case=image_classification_offline_v2
mkdir %test_case%%test_case_suffix%
set use_case_results_file=%results_prefix%%test_case%%results_suffix%
.\main.exe EXTERNAL %test_case% --mode=PerformanceOnly --scenario=Offline --batch_size=12288 --images_directory=%dataset_path%\imagenet\img --offset=0 --output_dir=%test_case%%test_case_suffix% --min_query_count=24576 --min_duration_ms=0 --single_stream_expected_latency_ns=1000000 --groundtruth_file=%dataset_path% --model_file=%models_path%\mobilenet_v4_htp_batched_4.dlc --lib_path=libqtibackend.dll --native_lib_path=. > %use_case_results_file% 2>&1
echo #######%test_case%###### >> %results_file%
findstr /C:"Result is" %use_case_results_file% >> %results_file%
findstr /C:"Samples per second" %use_case_results_file% >> %results_file%
echo ####### Image classification offline V2 is complete #######
EXIT /B 0

:image_classification_performance
echo ####### Performance:: Image classification in progress #######
set test_case=image_classification
mkdir %test_case%%test_case_suffix%
set use_case_results_file=%results_prefix%%test_case%%results_suffix%
.\main.exe EXTERNAL %test_case% --mode=PerformanceOnly --images_directory=%dataset_path%\imagenet\img --offset=1 --output_dir=%test_case%%test_case_suffix% --min_query_count=%min_query% --min_duration_ms=%min_duration_ms% --single_stream_expected_latency_ns=600000 --groundtruth_file=%dataset_path% --model_file=%models_path%\mobilenet_edgetpu_224_1.0_htp.dlc --lib_path=libqtibackend.dll --native_lib_path=. > %use_case_results_file% 2>&1
echo #######%test_case%###### >> %results_file%
findstr /C:"90th percentile latency (ns)" %use_case_results_file% >> %results_file%
findstr /C:"Result is" %use_case_results_file% >> %results_file%
findstr /C:"QPS w/o loadgen overhead" %use_case_results_file% >> %results_file%
echo ####### Image classification is complete #######
EXIT /B 0

:image_classification_offline_performance
echo ####### Performance:: Image classification offline in progress #######
set test_case=image_classification_offline
mkdir %test_case%%test_case_suffix%
set use_case_results_file=%results_prefix%%test_case%%results_suffix%
.\main.exe EXTERNAL %test_case% --mode=PerformanceOnly --scenario=Offline --batch_size=12288 --images_directory=%dataset_path%\imagenet\img --offset=1 --output_dir=%test_case%%test_case_suffix% --min_query_count=24576 --min_duration_ms=0 --single_stream_expected_latency_ns=1000000 --groundtruth_file=%dataset_path% --model_file=%models_path%\mobilenet_edgetpu_224_1.0_htp_batched_8.dlc --lib_path=libqtibackend.dll --native_lib_path=. > %use_case_results_file% 2>&1
echo #######%test_case%###### >> %results_file%
findstr /C:"Result is" %use_case_results_file% >> %results_file%
findstr /C:"Samples per second" %use_case_results_file% >> %results_file%
echo ####### Image classification offline is complete #######
EXIT /B 0

rem ####### Accuracy usecase functions #######

:image_classification_v2_accuracy
echo ####### Accuracy:: Image classification V2 in progress #######
set test_case=image_classification_v2
mkdir %test_case%%test_case_suffix%
set use_case_results_file=%results_prefix%%test_case%%results_suffix%
.\main.exe EXTERNAL %test_case% --mode=AccuracyOnly --images_directory=%dataset_path%\imagenet\img --offset=0 --output_dir=%test_case%%test_case_suffix% --min_query_count=%min_query% --min_duration_ms=%min_duration_ms% --single_stream_expected_latency_ns=1000000 --groundtruth_file=%dataset_path%\imagenet\imagenet_val_full.txt --model_file=%models_path%\mobilenet_v4_htp.dlc --lib_path=libqtibackend.dll --native_lib_path=. > %use_case_results_file% 2>&1
echo #######%test_case%###### >> %results_file%
findstr "Accuracy" %use_case_results_file% >> %results_file%
echo ####### Image classification V2 is complete #######
EXIT /B 0

:object_detection_accuracy
echo ####### Accuracy:: Object detection in progress #######
set test_case=object_detection
mkdir %test_case%%test_case_suffix%
set use_case_results_file=%results_prefix%%test_case%%results_suffix%
.\main.exe EXTERNAL %test_case% --mode=AccuracyOnly --images_directory=%dataset_path%\coco\img --offset=1   --output_dir=%test_case%%test_case_suffix% --min_query_count=%min_query% --min_duration_ms=%min_duration_ms% --single_stream_expected_latency_ns=1000000 --groundtruth_file=%dataset_path%\coco\coco_val_full.pbtxt --model_file=%models_path%\ssd_mobiledet_qat_htp.dlc --lib_path=libqtibackend.dll --native_lib_path=. --num_classes=91 > %use_case_results_file% 2>&1
echo #######%test_case%###### >> %results_file%
findstr "Accuracy" %use_case_results_file% >> %results_file%
echo ####### Object detection is complete #######
EXIT /B 0

:image_segmentation_accuracy
echo ####### Accuracy:: Image segmentation in progress #######
set test_case=image_segmentation_v2
mkdir %test_case%%test_case_suffix%
set use_case_results_file=%results_prefix%%test_case%%results_suffix%
.\main.exe EXTERNAL %test_case% --mode=AccuracyOnly --images_directory=%dataset_path%\ade20k\images --num_class=31 --output_dir=%test_case%%test_case_suffix% --min_query_count=%min_query% --min_duration_ms=%min_duration_ms% --single_stream_expected_latency_ns=1000000 --ground_truth_directory=%dataset_path%\ade20k\annotations --model_file=%models_path%\mobile_mosaic_htp.dlc --lib_path=libqtibackend.dll --native_lib_path=. > %use_case_results_file% 2>&1
echo #######%test_case%###### >> %results_file%
findstr "Accuracy" %use_case_results_file% >> %results_file%
echo ####### Image segmentation is complete #######
EXIT /B 0

:language_understanding_accuracy
echo ####### Accuracy:: Natural language processing in progress #######
set test_case=natural_language_processing
mkdir %test_case%%test_case_suffix%
set use_case_results_file=%results_prefix%%test_case%%results_suffix%
.\main.exe EXTERNAL %test_case% --mode=AccuracyOnly --input_file=%dataset_path%\squad\squad_eval.tfrecord --output_dir=%test_case%%test_case_suffix% --min_query_count=%min_query% --min_duration_ms=%min_duration_ms% --single_stream_expected_latency_ns=1000000 --groundtruth_file=%dataset_path%\squad\squad_groundtruth.tfrecord --model_file=%models_path%\mobilebert_quantized_htp.dlc --lib_path=libqtibackend.dll --native_lib_path=. > %use_case_results_file% 2>&1
echo #######%test_case%###### >> %results_file%
findstr "Accuracy" %use_case_results_file% >> %results_file%
echo ####### Natural language processing is complete #######
EXIT /B 0

:super_resolution_accuracy
echo ####### Accuracy:: Super resolution in progress #######
set test_case=super_resolution
mkdir %test_case%%test_case_suffix%
set use_case_results_file=%results_prefix%%test_case%%results_suffix%
.\main.exe EXTERNAL %test_case% --mode=AccuracyOnly --images_directory=%dataset_path%\snusr\lr --output_dir=%test_case%%test_case_suffix% --min_query_count=%min_query% --min_duration_ms=%min_duration_ms% --single_stream_expected_latency_ns=1000000 --ground_truth_directory=%dataset_path%\snusr\hr --model_file=%models_path%\snusr_htp.dlc --lib_path=libqtibackend.dll --native_lib_path=. > %use_case_results_file% 2>&1
echo #######%test_case%###### >> %results_file%
findstr "Accuracy" %use_case_results_file% >> %results_file%
echo ####### Super resolution is complete #######
EXIT /B 0

:image_classification_offline_v2_accuracy
echo ####### Accuracy:: Image classification offline V2 in progress #######
set test_case=image_classification_offline_v2
mkdir %test_case%%test_case_suffix%
set use_case_results_file=%results_prefix%%test_case%%results_suffix%
.\main.exe EXTERNAL %test_case% --mode=AccuracyOnly --scenario=Offline --batch_size=12288 --images_directory=%dataset_path%\imagenet\img --offset=0 --output_dir=%test_case%%test_case_suffix% --min_query_count=24576 --min_duration_ms=0 --single_stream_expected_latency_ns=1000000 --groundtruth_file=%dataset_path%\imagenet\imagenet_val_full.txt --model_file=%models_path%\mobilenet_v4_htp_batched_4.dlc --lib_path=libqtibackend.dll --native_lib_path=. > %use_case_results_file%  2>&1
echo #######%test_case%###### >> %results_file%
findstr "Accuracy" %use_case_results_file% >> %results_file%
echo ####### Image classification offline V2 is complete #######
EXIT /B 0

:image_classification_accuracy
echo ####### Accuracy:: Image classification in progress #######
set test_case=image_classification
mkdir %test_case%%test_case_suffix%
set use_case_results_file=%results_prefix%%test_case%%results_suffix%
.\main.exe EXTERNAL %test_case% --mode=AccuracyOnly --images_directory=%dataset_path%\imagenet\img --offset=1 --output_dir=%test_case%%test_case_suffix% --min_query_count=%min_query% --min_duration_ms=%min_duration_ms% --single_stream_expected_latency_ns=600000 --groundtruth_file=%dataset_path%\imagenet\imagenet_val_full.txt --model_file=%models_path%\mobilenet_edgetpu_224_1.0_htp.dlc --lib_path=libqtibackend.dll --native_lib_path=. > %use_case_results_file% 2>&1
echo #######%test_case%###### >> %results_file%
findstr "Accuracy" %use_case_results_file% >> %results_file%
echo ####### Image classification is complete #######
EXIT /B 0

:image_classification_offline_accuracy
echo ####### Accuracy:: Image classification offline in progress #######
set test_case=image_classification_offline
mkdir %test_case%%test_case_suffix%
set use_case_results_file=%results_prefix%%test_case%%results_suffix%
.\main.exe EXTERNAL %test_case% --mode=AccuracyOnly --scenario=Offline --batch_size=12288 --images_directory=%dataset_path%\imagenet\img --offset=1 --output_dir=%test_case%%test_case_suffix% --min_query_count=24576 --min_duration_ms=0 --single_stream_expected_latency_ns=1000000 --groundtruth_file=%dataset_path%\imagenet\imagenet_val_full.txt --model_file=%models_path%\mobilenet_edgetpu_224_1.0_htp_batched_8.dlc --lib_path=libqtibackend.dll --native_lib_path=. > %use_case_results_file%  2>&1
echo #######%test_case%###### >> %results_file%
findstr "Accuracy" %use_case_results_file% >> %results_file%
echo ####### Image classification offline is complete #######
EXIT /B 0

:dataset_end
echo "set dataset path using --dataset"

:models_end
echo "set models path using --models"
