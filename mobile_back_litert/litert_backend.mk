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

ifeq (${WITH_LITERT},1)
  $(info WITH_LITERT=1)
  backend_litert_bins_dir=output/litert-bins
  backend_litert_bin_filename=libLiteRtClGlAccelerator.so
  backend_litert_bins_url=http://storage.googleapis.com/litert/binaries/2.1.5/android_arm64/${backend_litert_bin_filename}
  backend_litert_lib_deps= mkdir -p ${backend_litert_bins_dir} && \
                           curl -fSL -o ${backend_litert_bins_dir}/${backend_litert_bin_filename} ${backend_litert_bins_url}

  backend_litert_windows_files=${BAZEL_LINKS_PREFIX}bin/mobile_back_litert/cpp/backend_litert/liblitertbackend.dll
  backend_litert_windows_target=//mobile_back_litert/cpp/backend_litert:liblitertbackend.dll
  backend_litert_android_files=${BAZEL_LINKS_PREFIX}bin/mobile_back_litert/cpp/backend_litert/liblitertbackend.so \
			       ${backend_litert_bins_dir}/libLiteRtClGlAccelerator.so
  backend_litert_android_target=//mobile_back_litert/cpp/backend_litert:liblitertbackend.so
  backend_litert_ios_target=//mobile_back_litert/cpp/backend_litert/ios:liblitertbackend
  backend_litert_ios_zip=${BAZEL_LINKS_PREFIX}bin/mobile_back_litert/cpp/backend_litert/ios/liblitertbackend.xcframework.zip
  backend_litert_filename=liblitertbackend
else
  # xcode will give you an error if a backend is specified in xcode config but the file is missing
  backend_litert_ios_target=//mobile_back_litert/cpp/backend_dummy/ios:liblitertbackend
  backend_litert_ios_zip=${BAZEL_LINKS_PREFIX}bin/mobile_back_litert/cpp/backend_dummy/ios/liblitertbackend.xcframework.zip
endif
