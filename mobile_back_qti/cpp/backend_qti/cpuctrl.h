/* Copyright (c) 2020-2021 Qualcomm Innovation Center, Inc. All rights reserved.

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
#pragma once

#include <EGL/egl.h>
#include <GLES/gl.h>
#include <stdint.h>

#define SDM865 356
#define SDM888 415

class CpuCtrl {
 public:
  static void startLoad();
  static void stopLoad();
  static void lowLatency();
  static void normalLatency();
  static void highLatency();
  static uint32_t getSocId();
  static bool isSnapDragon(const char *manufacturer);

 private:
  CpuCtrl() = delete;
  ~CpuCtrl() = delete;
};
