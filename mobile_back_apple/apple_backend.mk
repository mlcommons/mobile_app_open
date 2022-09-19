# Copyright 2022 The MLPerf Authors. All Rights Reserved.
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

ifeq (${WITH_APPLE},1)
$(info WITH_APPLE=1)
  backend_coreml_ios_target=//mobile_back_apple/cpp/backend_coreml:libcoremlbackend
  backend_coreml_ios_zip=${BAZEL_LINKS_PREFIX}bin/mobile_back_apple/cpp/backend_coreml/libcoremlbackend.xcframework.zip
  backend_coreml_filename=libcoremlbackend
else 
  # xcode will give you an error if a backend is specified in xcode config but the file is missing
  backend_coreml_ios_target=//mobile_back_tflite/cpp/backend_fake/ios:libcoremlbackend
  backend_coreml_ios_zip=${BAZEL_LINKS_PREFIX}bin/mobile_back_tflite/cpp/backend_fake/ios/libcoremlbackend.xcframework.zip
endif
