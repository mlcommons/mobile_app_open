# Copyright 2026 The MLPerf Authors. All Rights Reserved.
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

# LiteRT backend (Android and iOS): every benchmark runs on the LiteRT
# CompiledModel API -- the llm-* benchmarks on the LLM pipeline, the vision/NLP
# benchmarks on the single-model pipeline.
backend_litert_bins_dir=output/litert-bins

# The Metal accelerator is a prebuilt dylib that the Xcode project embeds
# unconditionally (it is dlopened at runtime, never linked), so it has to be
# downloaded for every iOS build, even with WITH_LITERT=0 where only the dummy
# backend is bundled.
backend_litert_ios_bin_filename=libLiteRtMetalAccelerator.dylib
backend_litert_ios_bins_url=https://storage.googleapis.com/litert/binaries/2.1.5/ios_arm64/${backend_litert_ios_bin_filename}
backend_litert_ios_file=${backend_litert_bins_dir}/${backend_litert_ios_bin_filename}
backend_litert_ios_lib_deps= mkdir -p ${backend_litert_bins_dir} && \
                             { [ -s ${backend_litert_ios_file} ] || \
                               curl -fSL --proto '=https' --retry 3 --retry-delay 5 \
                                    -o ${backend_litert_ios_file} ${backend_litert_ios_bins_url}; }

ifeq (${WITH_LITERT},1)
  $(info WITH_LITERT=1)
  backend_litert_bin_filename=libLiteRtClGlAccelerator.so
  backend_litert_bins_url=https://storage.googleapis.com/litert/binaries/2.1.5/android_arm64/${backend_litert_bin_filename}
  backend_litert_lib_deps= mkdir -p ${backend_litert_bins_dir} && \
                           curl -fSL --proto '=https' -o ${backend_litert_bins_dir}/${backend_litert_bin_filename} ${backend_litert_bins_url}

  backend_litert_android_files=${BAZEL_LINKS_PREFIX}bin/mobile_back_litert/cpp/backend_litert/liblitertbackend.so \
			       ${backend_litert_bins_dir}/${backend_litert_bin_filename}
  backend_litert_android_target=//mobile_back_litert/cpp/backend_litert:liblitertbackend.so
  backend_litert_ios_target=//mobile_back_litert/cpp/backend_litert/ios:liblitertbackend
  backend_litert_ios_zip=${BAZEL_LINKS_PREFIX}bin/mobile_back_litert/cpp/backend_litert/ios/liblitertbackend.xcframework.zip
  backend_litert_filename=liblitertbackend
else
  # xcode will give you an error if a backend is specified in xcode config but the file is missing
  backend_litert_ios_target=//mobile_back_tflite/cpp/backend_dummy/ios:liblitertbackend
  backend_litert_ios_zip=${BAZEL_LINKS_PREFIX}bin/mobile_back_tflite/cpp/backend_dummy/ios/liblitertbackend.xcframework.zip
endif
