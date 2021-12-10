# Copyright 2021 The MLPerf Authors. All Rights Reserved.
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

ifeq (${WITH_TFLITE},1)
  $(info WITH_TFLITE=1)
  # Android app always includes tflite backend
  # ANDROID_TFLITE_BACKEND_BAZEL_FLAG=
  BACKEND_TFLITE_DLL_FILE=mobile_back_tflite/cpp/backend_tflite/libtflitebackend.dll
  BACKEND_TFLITE_DLL_TARGET=//mobile_back_tflite/cpp/backend_tflite:libtflitebackend.dll
  BACKEND_TFLITE_SO_FILE=mobile_back_tflite/cpp/backend_tflite/libtflitebackend.so
  BACKEND_TFLITE_SO_TARGET=//mobile_back_tflite/cpp/backend_tflite:libtflitebackend.so
  BACKEND_TFLITE_FILENAME=libtflitebackend
endif

ifeq (${WITH_MEDIATEK},1)
  $(info WITH_MEDIATEK=1)
  ANDROID_MEDIATEK_BACKEND_BAZEL_FLAG=--//android/java/org/mlperf/inference:with_mediatek="1"
endif
