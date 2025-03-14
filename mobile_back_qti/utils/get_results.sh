#!/bin/bash
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

set -e

if [ ! $# = 1 ]; then
  echo "Usage: $0 assetsdir"
  exit 1
fi

ASSETS=$1

mkdir -p ${ASSETS}

adb pull /sdcard/mlperf_results/results.json ${ASSETS}/results.json
adb pull /sdcard/mlperf_results/log_performance ${ASSETS}/log_performance
adb pull /sdcard/mlperf_results/log_accuracy ${ASSETS}/log_accuracy

