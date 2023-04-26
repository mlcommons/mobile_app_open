/* Copyright 2020-2021 The MLPerf Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/
#ifndef TFLITE_SETTINGS_APPLE_H
#define TFLITE_SETTINGS_APPLE_H

#include "mobile_back_tflite/cpp/backend_tflite/backend_settings/tflite_settings_apple_iphone11.pbtxt.h"
#include "mobile_back_tflite/cpp/backend_tflite/backend_settings/tflite_settings_apple_iphone12.pbtxt.h"
#include "mobile_back_tflite/cpp/backend_tflite/backend_settings/tflite_settings_apple_iphoneX.pbtxt.h"
#include "mobile_back_tflite/cpp/backend_tflite/backend_settings/tflite_settings_apple_main.pbtxt.h"

static std::string tflite_settings_apple;

const std::string tflite_settings_apple_main = tflite_settings_apple_main_pbtxt;
const std::string tflite_settings_apple_iphoneX =
    tflite_settings_apple_iphoneX_pbtxt;
const std::string tflite_settings_apple_iphone11 =
    tflite_settings_apple_iphone11_pbtxt;
const std::string tflite_settings_apple_iphone12 =
    tflite_settings_apple_iphone12_pbtxt;

#endif
