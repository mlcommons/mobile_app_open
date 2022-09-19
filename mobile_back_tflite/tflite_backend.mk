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
  backend_tflite_windows_files=${BAZEL_LINKS_PREFIX}bin/mobile_back_tflite/cpp/backend_tflite/libtflitebackend.dll
  backend_tflite_windows_target=//mobile_back_tflite/cpp/backend_tflite:libtflitebackend.dll
  backend_tflite_android_files=${BAZEL_LINKS_PREFIX}bin/mobile_back_tflite/cpp/backend_tflite/libtflitebackend.so
  backend_tflite_android_target=//mobile_back_tflite/cpp/backend_tflite:libtflitebackend.so
  backend_tflite_ios_target=//mobile_back_tflite/cpp/backend_tflite/ios:libtflitebackend
  backend_tflite_ios_zip=${BAZEL_LINKS_PREFIX}bin/mobile_back_tflite/cpp/backend_tflite/ios/libtflitebackend.xcframework.zip
  backend_tflite_filename=libtflitebackend
else
  # tflite is enabled by default, so print log message only if someone disabled it
  $(info WITH_TFLITE=0)
  # xcode will give you an error if a backend is specified in xcode config but the file is missing
  backend_tflite_ios_target=//mobile_back_tflite/cpp/backend_fake/ios:libtflitebackend
  backend_tflite_ios_zip=${BAZEL_LINKS_PREFIX}bin/mobile_back_tflite/cpp/backend_fake/ios/libtflitebackend.xcframework.zip
endif

ifeq (${WITH_MEDIATEK},1)
  $(info WITH_MEDIATEK=1)
  backend_mediatek_android_files= \
    ${BAZEL_LINKS_PREFIX}bin/mobile_back_tflite/cpp/backend_tflite/neuron/libtfliteneuronbackend.so \
    bazel-bin/external/neuron_delegate/neuron/java/libtensorflowlite_neuron_jni.so
  backend_mediatek_android_target=//mobile_back_tflite/cpp/backend_tflite/neuron:libtfliteneuronbackend.so
  backend_mediatek_filename=libtfliteneuronbackend
endif
