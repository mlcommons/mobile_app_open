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

#ifndef MOBILE_APP_OPEN_SOC_UTILITY_H
#define MOBILE_APP_OPEN_SOC_UTILITY_H

#ifdef __ANDROID__
#include <EGL/egl.h>
#include <GLES/gl.h>
#endif
#include <stdint.h>

#include <map>
#include <string>
#include <vector>

#include "qti_settings.h"

#define DEFAULT_SOC_STR "Default"
#define UNSUPPORTED_SOC_STR "Unsupported"
#define UNSUPPORTED_SOC_ID 0

extern bool useIonBuffer_g;

class SocInfo {
 public:
  SocInfo(std::string soc_name = DEFAULT_SOC_STR)
      : m_num_dsp(0),
        m_num_gpu(0),
        m_num_cpu(0),
        m_num_gpu_fp16(0),
        m_useDspFeatures(false),
        m_settings(empty_settings),
        m_soc_name(soc_name),
        m_num_inits(0),
        m_max_cores(0),
        m_needs_rpcmem(false) {}

  SocInfo(int num_dsp, int num_gpu, int num_cpu, int num_gpu_fp16,
          bool useDspFeatures, const std::string settings, std::string soc_name,
          int num_inits, std::vector<int> hlc, std::vector<int> llc,
          int max_cores, bool needs_rpcmem)
      : m_num_dsp(num_dsp),
        m_num_gpu(num_gpu),
        m_num_cpu(num_cpu),
        m_num_gpu_fp16(num_gpu_fp16),
        m_useDspFeatures(useDspFeatures),
        m_settings(settings),
        m_soc_name(soc_name),
        m_num_inits(num_inits),
        m_high_latency_cores(hlc),
        m_low_latency_cores(llc),
        m_max_cores(max_cores),
        m_needs_rpcmem(needs_rpcmem) {
    if (m_useDspFeatures == false) {
      m_num_inits = 1;
    }
  }

  int m_num_dsp;
  int m_num_gpu;
  int m_num_cpu;
  int m_num_gpu_fp16;
  int m_num_inits;
  bool m_useDspFeatures;
  std::string m_settings;
  std::string m_soc_name;
  std::vector<int> m_high_latency_cores;
  std::vector<int> m_low_latency_cores;
  int m_max_cores;
  bool m_needs_rpcmem;
};

class SocProperties {
 public:
  SocProperties(std::map<uint32_t, SocInfo> soc_details)
      : m_soc_details(soc_details) {}

  std::map<uint32_t, SocInfo> m_soc_details;
};

class Socs {
 private:
#ifdef __ANDROID__
  static uint32_t get_android_soc_id(void);
#else
  static uint32_t get_windows_soc_id(void);
#endif
 public:
  static void soc_info_init();
#ifndef __ANDROID__
  static std::string getServiceBinaryPath(std::wstring const &serviceName);
#endif
  static std::string get_external_config_string();

  static void soc_offline_core_instance(int &num_dsp, int &num_gpu,
                                        int &num_cpu, int &num_gpu_fp16,
                                        std::string &delegate);
  static int soc_num_inits();

  static bool isSnapDragon(const char *manufacturer);

  static int soc_check_feature(bool &useIonBuffers_,
                               std::string &platformOptionStr);

  static bool soc_settings(const char **settings,
                           const char **not_allowed_message);

  static bool is_sdm865();

  static void define_soc(std::vector<uint32_t> &allcores,
                         std::vector<uint32_t> &low_latency_cores,
                         std::vector<uint32_t> &high_latency_cores,
                         int &maxcore);

  static bool needs_rpcmem();

  static std::string get_soc_name();

  static bool get_use_dsp_features();

  static void set_use_dsp_features(bool flag);

  static SocInfo m_soc_info;

  static bool is_init_done;
};

#endif  // MOBILE_APP_OPEN_SOC_UTILITY_H
