/* Copyright (c) 2020-2024 Qualcomm Innovation Center, Inc. All rights reserved.

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
#ifndef QTI_SETTINGS_H
#define QTI_SETTINGS_H

#include <string>

const std::string empty_settings = "";

// Macro for adding quotes
#define STRINGIFY(X) STRINGIFY2(X)
#define STRINGIFY2(X) #X
// clang-format off
#define INCLUDE_SETTINGS(device)                             \
  STRINGIFY(mobile_back_qti/cpp/backend_qti/settings/qti_settings_##device.pbtxt.h)
// clang-format on
#define SETTINGS_LHS(device) const std::string qti_settings_##device
#define SETTINGS_RHS(device) qti_settings_##device##_pbtxt
#define STRING_SETTINGS(device) SETTINGS_LHS(device) = SETTINGS_RHS(device);

#include INCLUDE_SETTINGS(sd7g1)
#include INCLUDE_SETTINGS(sd7pg2)
#include INCLUDE_SETTINGS(sd8cxg3)
#include INCLUDE_SETTINGS(sd7cxg3)
#include INCLUDE_SETTINGS(sd8g1)
#include INCLUDE_SETTINGS(sd8g2)
#include INCLUDE_SETTINGS(gpufp16)
#include INCLUDE_SETTINGS(sd8pg1)
#include INCLUDE_SETTINGS(sdm778)
#include INCLUDE_SETTINGS(sdm888)
#include INCLUDE_SETTINGS(sm4450)
#include INCLUDE_SETTINGS(sd8g3)
#include INCLUDE_SETTINGS(sm8635)
#include INCLUDE_SETTINGS(sm7550)
#include INCLUDE_SETTINGS(default_dsp)
#include INCLUDE_SETTINGS(default_cpu)
#include INCLUDE_SETTINGS(default_gpu)
#include INCLUDE_SETTINGS(stablediffusion)

STRING_SETTINGS(sd7g1)
STRING_SETTINGS(sd7pg2)
STRING_SETTINGS(sd8cxg3)
STRING_SETTINGS(sd7cxg3)
STRING_SETTINGS(sd8g1)
STRING_SETTINGS(sd8g2)
STRING_SETTINGS(gpufp16)
STRING_SETTINGS(sd8pg1)
STRING_SETTINGS(sdm778)
STRING_SETTINGS(sdm888)
STRING_SETTINGS(sm4450)
STRING_SETTINGS(sd8g3)
STRING_SETTINGS(sm8635)
STRING_SETTINGS(sm7550)
STRING_SETTINGS(default_dsp)
STRING_SETTINGS(default_cpu)
STRING_SETTINGS(default_gpu)
STRING_SETTINGS(stablediffusion)

#endif
