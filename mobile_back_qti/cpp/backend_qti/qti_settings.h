/* Copyright (c) 2020-2025 Qualcomm Innovation Center, Inc. All rights reserved.

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

#include INCLUDE_SETTINGS(sd8cxg3)
#include INCLUDE_SETTINGS(sd8sG4)
#include INCLUDE_SETTINGS(sd7G4)
#include INCLUDE_SETTINGS(sd8elite)
#include INCLUDE_SETTINGS(sd8eliteG5)
#include INCLUDE_SETTINGS(sd8G5)
#include INCLUDE_SETTINGS(gpufp16)
#include INCLUDE_SETTINGS(default_dsp)
#include INCLUDE_SETTINGS(default_cpu)
#include INCLUDE_SETTINGS(default_gpu)
#include INCLUDE_SETTINGS(stablediffusion)
#include INCLUDE_SETTINGS(stablediffusion_v75)
#include INCLUDE_SETTINGS(stablediffusion_v79)
#include INCLUDE_SETTINGS(stablediffusion_v73_sd8sG4)
#include INCLUDE_SETTINGS(stablediffusion_v73_sd7G4)
#include INCLUDE_SETTINGS(stablediffusion_v81_sd8eliteG5)
#include INCLUDE_SETTINGS(stablediffusion_v81_sd8G5)

STRING_SETTINGS(sd8cxg3)
STRING_SETTINGS(sd8sG4)
STRING_SETTINGS(sd7G4)
STRING_SETTINGS(sd8elite)
STRING_SETTINGS(sd8eliteG5)
STRING_SETTINGS(sd8G5)
STRING_SETTINGS(gpufp16)
STRING_SETTINGS(default_dsp)
STRING_SETTINGS(default_cpu)
STRING_SETTINGS(default_gpu)
STRING_SETTINGS(stablediffusion)
STRING_SETTINGS(stablediffusion_v75)
STRING_SETTINGS(stablediffusion_v79)
STRING_SETTINGS(stablediffusion_v73_sd8sG4)
STRING_SETTINGS(stablediffusion_v73_sd7G4)
STRING_SETTINGS(stablediffusion_v81_sd8eliteG5)
STRING_SETTINGS(stablediffusion_v81_sd8G5)

#endif
