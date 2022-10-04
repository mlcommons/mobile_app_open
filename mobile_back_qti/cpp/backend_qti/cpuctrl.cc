/* Copyright (c) 2020-2022 Qualcomm Innovation Center, Inc. All rights reserved.

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
#include "soc_utility.h"

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

static bool active_ = false;
static uint32_t loadOffTime_ = 2;
static uint32_t loadOnTime_ = 100;
static std::thread *thread_ = nullptr;
static cpu_set_t cpusetLow_;
static cpu_set_t cpusetHigh_;
static cpu_set_t cpusetall_;

static void loop(void *unused) {
  (void)unused;
  active_ = true;
  while (active_) {
    auto now =
        duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    if (now.count() % loadOnTime_ == 0 && active_) {
      usleep(loadOffTime_ * 1000);
    } else {
      for (int i = 0; i < loadOnTime_; i++) {
        // Prevent compiler from optimizing away busy loop
        __asm__ __volatile__("" : "+g"(i) : :);
      }
    }
  }
}

void CpuCtrl::startLoad(uint32_t load_off_time, uint32_t load_on_time) {
  if (active_) {
    return;
  }
  loadOffTime_ = load_off_time;
  loadOnTime_ = load_on_time;
  thread_ = new std::thread(loop, nullptr);
}

void CpuCtrl::stopLoad() {
  active_ = false;
  if (thread_) {
    thread_->join();
  }
  delete thread_;
  thread_ = nullptr;
}

void CpuCtrl::lowLatency() { SET_AFFINITY(sizeof(cpu_set_t), &cpusetLow_); }

void CpuCtrl::normalLatency() { SET_AFFINITY(sizeof(cpu_set_t), &cpusetall_); }

void CpuCtrl::highLatency() { SET_AFFINITY(sizeof(cpu_set_t), &cpusetHigh_); }

void CpuCtrl::init() {
    std::vector <uint32_t> allcores;
    std::vector <uint32_t> low_latency_cores;
    std::vector <uint32_t> high_latency_cores;
    int maxcores = 0;
    Socs::define_soc(allcores, low_latency_cores, high_latency_cores, maxcores);

    for (auto i = 0; i < maxcores; i++) {
        allcores.emplace_back(i);
    }

    CPU_ZERO(&cpusetLow_);
    for (auto core: low_latency_cores) {
        CPU_SET(core, &cpusetLow_);
    }
    CPU_ZERO(&cpusetHigh_);
    for (auto core: high_latency_cores) {
        CPU_SET(core, &cpusetHigh_);
    }
    CPU_ZERO(&cpusetall_);
    for (auto core: allcores) {
        CPU_SET(core, &cpusetall_);
    }
}