/* Copyright (c) 2020-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include "soc_utility.h"

#include <stdint.h>

#include <fstream>
#include <iostream>
#include <thread>

#include "qti_backend_helper.h"
#include "tensorflow/core/platform/logging.h"
#ifndef __ANDROID__
#include <Windows.h>

#include <unordered_map>

#include "acpitabl.h"
#endif

SocInfo unsupportedSoc = SocInfo(UNSUPPORTED_SOC_STR);
SocInfo Socs::m_soc_info;
bool Socs::is_init_done;

std::map<uint32_t, SocInfo> socDetails =
    SocProperties(
        {
            // num_dsp, num_gpu, num_cpu,
            // useDspFeatures, settings,
            // soc_name, num_inits,
            // hlc,
            // llc,
            // max_cores, needs_rpcmem
            {415, SocInfo(2, 0, 0, 0, true, qti_settings_sdm888, "SDM888", 1,
                          std::vector<int>({0, 1, 2, 3}),
                          std::vector<int>({4, 5, 6, 7}), 8, true)},
            {475, SocInfo(2, 0, 0, 0, true, qti_settings_sdm778, "SDM778", 1,
                          std::vector<int>({0, 1, 2, 3}),
                          std::vector<int>({4, 5, 6, 7}), 8, true)},
            {506, SocInfo(2, 0, 0, 0, true, qti_settings_sd7g1, "SD7G1", 1,
                          std::vector<int>({0, 1, 2, 3}),
                          std::vector<int>({4, 5, 6, 7}), 8, true)},
            {457, SocInfo(2, 0, 0, 0, true, qti_settings_sd8g1, "SD8G1", 1,
                          std::vector<int>({0, 1, 2, 3}),
                          std::vector<int>({4, 5, 6, 7}), 8, true)},
            {552, SocInfo(2, 0, 0, 0, true, qti_settings_sd8pg1, "SD8PG1 4G", 1,
                          std::vector<int>({0, 1, 2, 3}),
                          std::vector<int>({4, 5, 6, 7}), 8, true)},
            {540, SocInfo(2, 0, 0, 0, true, qti_settings_sd8pg1,
                          "SD8PG1 prime ", 1, std::vector<int>({0, 1, 2, 3}),
                          std::vector<int>({4, 5, 6, 7}), 8, true)},
            {530, SocInfo(2, 0, 0, 0, true, qti_settings_sd8pg1, "SD8PG1", 1,
                          std::vector<int>({0, 1, 2, 3}),
                          std::vector<int>({4, 5, 6, 7}), 8, true)},
            {519, SocInfo(2, 0, 0, 0, true, qti_settings_sd8g2, "SD8G2", 1,
                          std::vector<int>({0, 1, 2, 3}),
                          std::vector<int>({4, 5, 6, 7}), 8, true)},
            {536, SocInfo(2, 0, 0, 0, true, qti_settings_sd8g2, "SD8G2", 1,
                          std::vector<int>({0, 1, 2, 3}),
                          std::vector<int>({4, 5, 6, 7}), 8, true)},
            {591, SocInfo(2, 0, 0, 0, true, qti_settings_sd7pg2, "SD7PG2", 1,
                          std::vector<int>({0, 1, 2, 3}),
                          std::vector<int>({4, 5, 6, 7}), 8, true)},
            {435, SocInfo(2, 0, 0, 0, true, qti_settings_sd8cxg3, "SD8cxG3", 1,
                          std::vector<int>({0, 1, 2, 3}),
                          std::vector<int>({4, 5, 6, 7}), 8, false)},
            {568, SocInfo(0, 0, 1, 0, false, qti_settings_sm4450, "SM4450", 1,
                          std::vector<int>({0, 1, 2, 3}),
                          std::vector<int>({4, 5, 6, 7}), 8, true)},
            {475, SocInfo(2, 0, 0, 0, false, qti_settings_sd7cxg3, "SD7cxG3", 1,
                          std::vector<int>({0, 1, 2, 3}),
                          std::vector<int>({4, 5, 6, 7}), 8, false)},
            {UNSUPPORTED_SOC_ID,
             SocInfo(2, 0, 0, 0, true, qti_settings_default_dsp, "Snapdragon",
                     1, std::vector<int>({0, 1, 2, 3}),
                     std::vector<int>({4, 5, 6, 7}), 8, false)},
        })
        .m_soc_details;

#ifdef __ANDROID__
uint32_t Socs::get_android_soc_id(void) {
  uint32_t id;
  std::ifstream in_file;
  std::vector<char> line(5);
  in_file.open("/sys/devices/soc0/soc_id");
  if (in_file.fail()) {
    in_file.open("/sys/devices/system/soc/soc0/id");
  }
  if (in_file.fail()) {
    m_soc_info = unsupportedSoc;
    LOG(INFO) << "Failed to read SOC file: ";
    return UNSUPPORTED_SOC_ID;
  }

  in_file.read(line.data(), 5);
  in_file.close();
  id = (uint32_t)std::atoi(line.data());

  return id;
}

#else
#define MAX_FADT_PPTT_SIZE 65536
#define LEVEL_ID(LV1, LV2) ((LV1 << 32) | (LV2))

static std::unordered_map<uint64_t, int> pptt_mappings = {
    {LEVEL_ID(113ULL, 449ULL), 435},  // SD8cxG3
};

uint32_t Socs::get_windows_soc_id(void) {
  DWORD bufsize = 0;
  int ret = 0;
  PPPTT pptt;
  BYTE *buf = NULL;
  int id = 0;

  buf = (BYTE *)malloc(MAX_FADT_PPTT_SIZE);
  if (!buf) {
    return 0;
  }

  // start to try newer approach, level 1 ID, level 2 ID in PPTT
  ret = GetSystemFirmwareTable('ACPI', 'TTPP', 0, 0);
  if (!ret) {
    m_soc_info = unsupportedSoc;
    free(buf);
    return 0;
  }

  bufsize = ret;
  ret = GetSystemFirmwareTable('ACPI', 'TTPP', buf, bufsize);
  if (!ret) {
    m_soc_info = unsupportedSoc;
    free(buf);
    return 0;
  }

  pptt = (PPPTT)buf;
  uint64_t key = 0;
  for (uint32_t i = 0; i < pptt->Header.Length; i++) {
    PPROC_TOPOLOGY_NODE ptn =
        (PPROC_TOPOLOGY_NODE)((BYTE *)&(pptt->HeirarchyNodes[0]) + i);
    // According to ACPI spec, type = 2 is the PPTT_ID_TABLE_TYPE
    if (ptn->Type == 2) {
      key = (ptn->IdNode.Level1 << 32) | (ptn->IdNode.Level2);
      break;
    }
  }

  auto it = pptt_mappings.find(key);
  if (it != pptt_mappings.end()) {
    id = it->second;
  } else {
    m_soc_info = unsupportedSoc;
    id = 0;
    return 0;
  }

  free(buf);
  return id;
}
#endif

void Socs::soc_info_init() {
  if (is_init_done) return;

#ifdef __ANDROID__
  uint32_t soc_id = get_android_soc_id();
#else
  uint32_t soc_id = get_windows_soc_id();
#endif

  LOG(INFO) << "Soc ID: " << soc_id;
  if (soc_id != UNSUPPORTED_SOC_ID) {
    if (socDetails.find(soc_id) == socDetails.end()) {
      soc_id = UNSUPPORTED_SOC_ID;
    }

    m_soc_info = socDetails.find(soc_id)->second;
    if (soc_id == UNSUPPORTED_SOC_ID) {
      if (QTIBackendHelper::IsRuntimeAvailable(SNPE_DSP)) {
        m_soc_info.m_settings = qti_settings_default_dsp;
      } else if (QTIBackendHelper::IsRuntimeAvailable(SNPE_GPU)) {
        m_soc_info.m_settings = qti_settings_default_gpu;
      } else {
        m_soc_info.m_settings = qti_settings_default_cpu;
      }
    }
  } else {
    m_soc_info = unsupportedSoc;
  }
}

std::string Socs::get_soc_name() {
  soc_info_init();
  return m_soc_info.m_soc_name;
}

void Socs::soc_offline_core_instance(int &num_dsp, int &num_gpu, int &num_cpu,
                                     int &num_gpu_fp16, std::string &delegate) {
  soc_info_init();
  num_dsp = m_soc_info.m_num_dsp;
  num_gpu = m_soc_info.m_num_gpu;
  num_cpu = m_soc_info.m_num_cpu;
  num_gpu_fp16 = m_soc_info.m_num_gpu_fp16;

  if (delegate != "snpe_dsp" && delegate != "psnpe_dsp" &&
      delegate != "psnpe_cpu" && delegate != "snpe_gpu_fp16" &&
      delegate != "psnpe_gpu_fp16") {
    LOG(FATAL) << "Error: Unsupported delegate for offline mode";
  }
}

int Socs::soc_num_inits() {
  soc_info_init();
  return m_soc_info.m_num_inits;
}

bool Socs::isSnapDragon(const char *manufacturer) {
#ifdef __ANDROID__
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
#else
  // Always return true for Windows
  // TODO: Find a way to determine a QTI device on Windows
  return true;
#endif
}

int Socs::soc_check_feature(bool &useIonBuffers_,
                            std::string &platformOptionStr) {
  soc_info_init();
  // These features are not for SDM865, so turning them off.
  if (m_soc_info.m_useDspFeatures) {
    // use Zero copy for input and output buffers.
    // Requires rpc registered ion buffers.
    if (useIonBuffers_) {
      platformOptionStr += ";useDspZeroCopy:ON";
    }
    return 1;
  }
  return 0;
}

bool Socs::soc_settings(const char **settings,
                        const char **not_allowed_message) {
  soc_info_init();

  if (m_soc_info.m_soc_name == UNSUPPORTED_SOC_STR) {
    // it's a QTI SOC, but can't access soc_id
    *not_allowed_message = "Unsupported app";
    *settings = empty_settings.c_str();
    return true;
  }

  // Check if this SoC is supported
  *not_allowed_message = nullptr;
  *settings = m_soc_info.m_settings.c_str();
  if (m_soc_info.m_soc_name == DEFAULT_SOC_STR) {
    // it's a QTI SOC, but the chipset is not yet supported
    *not_allowed_message = "Unsupported QTI SoC";
  }
  return true;
}

bool Socs::needs_rpcmem() {
  soc_info_init();
#ifdef __ANDROID__
  return m_soc_info.m_needs_rpcmem;
#else
  return false;
#endif
}

bool Socs::get_use_dsp_features() {
  soc_info_init();
  return m_soc_info.m_useDspFeatures;
}

void Socs::set_use_dsp_features(bool flag) {
  soc_info_init();
  // Update true / false only based on the soc_info value
  m_soc_info.m_useDspFeatures = m_soc_info.m_useDspFeatures && flag;
}

void Socs::define_soc(std::vector<uint32_t> &allcores,
                      std::vector<uint32_t> &low_latency_cores,
                      std::vector<uint32_t> &high_latency_cores,
                      int &maxcores) {
  soc_info_init();
  high_latency_cores.assign(m_soc_info.m_high_latency_cores.begin(),
                            m_soc_info.m_high_latency_cores.end());
  low_latency_cores.assign(m_soc_info.m_low_latency_cores.begin(),
                           m_soc_info.m_low_latency_cores.end());
  maxcores = m_soc_info.m_max_cores;
}
