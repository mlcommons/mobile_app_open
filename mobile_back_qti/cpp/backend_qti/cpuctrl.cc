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

#include "cpuctrl.h"

#include <sched.h>
#include <sys/types.h>
#include <unistd.h>

#include <chrono>
#include <fstream>
#include <thread>
#include <vector>

#include "tensorflow/core/platform/logging.h"

using namespace std::chrono;

#define SET_AFFINITY(a, b) sched_setaffinity(gettid(), a, b)
#define GET_AFFINITY(a, b) sched_getaffinity(gettid(), a, b)

#define SLEEPMS 2

static uint32_t soc_id_g = 0;

static bool active_g = false;
static std::thread *thread_ = nullptr;
static cpu_set_t cpusetLow_g;
static cpu_set_t cpusetHigh_g;
static cpu_set_t cpusetall_g;

static void loop(void *unused) {
  (void)unused;
  active_g = true;
  while (active_g) {
    auto now =
        duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    if (now.count() % 100 == 0 && active_g) {
      usleep(SLEEPMS * 1000);
    } else {
      for (int i = 0; i < 100; i++) {
        // Prevent compiler from optimizing away busy loop
        __asm__ __volatile__("" : "+g"(i) : :);
      }
    }
  }
}

void CpuCtrl::startLoad() {
  if (active_g) {
    return;
  }
  thread_ = new std::thread(loop, nullptr);
}

void CpuCtrl::stopLoad() {
  active_g = false;
  if (thread_) {
    thread_->join();
  }
  delete thread_;
  thread_ = nullptr;
}

void CpuCtrl::lowLatency() { SET_AFFINITY(sizeof(cpu_set_t), &cpusetLow_g); }

void CpuCtrl::normalLatency() { SET_AFFINITY(sizeof(cpu_set_t), &cpusetall_g); }

void CpuCtrl::highLatency() { SET_AFFINITY(sizeof(cpu_set_t), &cpusetHigh_g); }

bool CpuCtrl::isSnapDragon(const char *manufacturer) {
  bool is_qcom = false;

  if (strncmp("QUALCOMM", manufacturer, 7) == 0) {
    // This is a test device
    LOG(INFO) << "QTI test device detected";
    is_qcom = true;
  } else {
    static EGLint const attribute_list[] = {EGL_RED_SIZE,  1, EGL_GREEN_SIZE, 1,
                                            EGL_BLUE_SIZE, 1, EGL_NONE};

    EGLDisplay display;
    EGLConfig config;
    EGLContext context;
    EGLSurface surface;
    EGLint num_config;

    /* get an EGL display connection */
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    /* initialize the EGL display connection */
    eglInitialize(display, NULL, NULL);
    /* get an appropriate EGL frame buffer configuration */
    eglChooseConfig(display, attribute_list, &config, 1, &num_config);
    /* create an EGL rendering context */
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, NULL);
    /* connect the context to the surface */
    eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, context);

    const unsigned char *vendor = glGetString(GL_VENDOR);

    if (strcmp("Qualcomm", (const char *)vendor) == 0) {
      is_qcom = true;
    }

    LOG(INFO) << "vendor: " << vendor;
  }
  return is_qcom;
}

uint32_t CpuCtrl::getSocId() {
  if (soc_id_g == 0) {
    std::ifstream in_file;
    std::vector<char> line(5);
    in_file.open("/sys/devices/soc0/soc_id");
    if (in_file.fail()) {
      in_file.open("/sys/devices/system/soc/soc0/id");
    }
    if (in_file.fail()) {
      return 0;
    }

    in_file.read(line.data(), 5);
    in_file.close();
    soc_id_g = (uint32_t)std::atoi(line.data());

    std::vector<uint32_t> allcores;
    std::vector<uint32_t> low_latency_cores;
    std::vector<uint32_t> high_latency_cores;
    int maxcores = 0;
    if (soc_id_g == SDM888 || soc_id_g == SDM865) {
      high_latency_cores.emplace_back(0);
      high_latency_cores.emplace_back(1);
      high_latency_cores.emplace_back(2);
      high_latency_cores.emplace_back(3);
      maxcores = 8;
    }

    if (soc_id_g == SDM888 || soc_id_g == SDM865) {
      low_latency_cores.emplace_back(4);
      low_latency_cores.emplace_back(5);
      low_latency_cores.emplace_back(6);
      low_latency_cores.emplace_back(7);
      maxcores = 8;
    }

    for (auto i = 0; i < maxcores; i++) {
      allcores.emplace_back(i);
    }

    CPU_ZERO(&cpusetLow_g);
    for (auto core : low_latency_cores) {
      CPU_SET(core, &cpusetLow_g);
    }
    CPU_ZERO(&cpusetHigh_g);
    for (auto core : high_latency_cores) {
      CPU_SET(core, &cpusetHigh_g);
    }
    CPU_ZERO(&cpusetall_g);
    for (auto core : allcores) {
      CPU_SET(core, &cpusetall_g);
    }
  }
  LOG(INFO) << "SOC ID is " << soc_id_g;
  return soc_id_g;
}
