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

#include "soc_utility.h"

#include <stdint.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#include "qti_backend_helper.h"
#include "tensorflow/core/platform/logging.h"
#ifndef __ANDROID__
#include <Windows.h>

#include <codecvt>
#include <unordered_map>

#include "acpitabl.h"
#endif

SocInfo unsupportedSoc = SocInfo(UNSUPPORTED_SOC_STR);
SocInfo Socs::m_soc_info;
bool Socs::is_init_done;
bool external_config =
#ifdef EXTERNAL_CONFIG
    true  // reads the external config file in device
#else
    false  // defaults to internal config
#endif
    ;

std::string Socs::get_external_config_string() {
  std::string file_path = "/data/local/tmp/external/qti_settings.pbtxt";
  std::ifstream f(file_path);  // taking file as inputstream
  std::string str;
  if (f) {
    std::ostringstream ss;
    ss << f.rdbuf();  // reading data
    str = ss.str();
    return str;
  } else {
    LOG(FATAL) << "The external config file qti_settings.pbtxt is not present "
                  "in //data/local/tmp/external";
  }
}

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
                          std::vector<int>({4, 5, 6, 7}), 8, true)},
            {568, SocInfo(0, 0, 1, 0, false, qti_settings_sm4450, "SM4450", 1,
                          std::vector<int>({0, 1, 2, 3}),
                          std::vector<int>({4, 5, 6, 7}), 8, true)},
            {538, SocInfo(2, 0, 0, 0, false, qti_settings_sd8cxg3, "SDX_Elite",
                          1, std::vector<int>({0, 1, 2, 3}),
                          std::vector<int>({4, 5, 6, 7}), 8, true)},
            {475, SocInfo(2, 0, 0, 0, false, qti_settings_sd7cxg3, "SD7cxG3", 1,
                          std::vector<int>({0, 1, 2, 3}),
                          std::vector<int>({4, 5, 6, 7}), 8, false)},
            {557, SocInfo(2, 0, 0, 0, true, qti_settings_sd8g3, "SD8G3", 1,
                          std::vector<int>({0, 1, 2, 3}),
                          std::vector<int>({4, 5, 6, 7}), 8, true, /* stable_diffusion */ true)},
            {614, SocInfo(2, 0, 0, 0, true, qti_settings_sm8635, "SM8635", 1,
                          std::vector<int>({0, 1, 2, 3}),
                          std::vector<int>({4, 5, 6, 7}), 8, true)},
            {608, SocInfo(2, 0, 0, 0, true, qti_settings_sm7550, "SM7550", 1,
                          std::vector<int>({0, 1, 2, 3}),
                          std::vector<int>({4, 5, 6, 7}), 8, true)},
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
    {LEVEL_ID(136ULL, 555ULL), 538},  // SDX_Elite
    {LEVEL_ID(136ULL, 615ULL), 538},  // SDX_Elite (C10)
    {LEVEL_ID(118ULL, 487ULL), 475},  // SD7cxG3 (SC7280)
    {LEVEL_ID(118ULL, 488ULL), 475},  // SD7cxG3 (SC7295)
    {LEVEL_ID(118ULL, 546ULL), 475},  // SD7cxG3 (SC7280P)
    {LEVEL_ID(118ULL, 553ULL), 475},  // SD7cxG3 (SC8270)
    {LEVEL_ID(118ULL, 563ULL), 475}   // SD7cxG3 (SC8270P)
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

std::string Socs::getServiceBinaryPath(std::wstring const &serviceName) {
  SC_HANDLE handleSCManager = OpenSCManagerW(NULL, NULL, STANDARD_RIGHTS_READ);
  if (NULL == handleSCManager) {
    LOG(ERROR) << "Failed to open SCManager which is required to access "
                  "service configuration on Windows.";
    return "";
  }

  SC_HANDLE handleService =
      OpenServiceW(handleSCManager, serviceName.c_str(), SERVICE_QUERY_CONFIG);

  if (NULL == handleService) {
    LOG(ERROR) << "Failed to open service which is required to query service "
                  "information.";
    CloseServiceHandle(handleSCManager);
    return "";
  }

  DWORD bufferSize;
  if (!QueryServiceConfigW(handleService, NULL, 0, &bufferSize) &&
      (GetLastError() != ERROR_INSUFFICIENT_BUFFER)) {
    CloseServiceHandle(handleService);
    CloseServiceHandle(handleSCManager);
    LOG(ERROR) << "Failed to query service configuration to get size of config "
                  "object. Error: "
               << GetLastError();
    return "";
  }

  LPQUERY_SERVICE_CONFIGW serviceConfig =
      static_cast<LPQUERY_SERVICE_CONFIGW>(LocalAlloc(LMEM_FIXED, bufferSize));
  if (!QueryServiceConfigW(handleService, serviceConfig, bufferSize,
                           &bufferSize)) {
    LocalFree(serviceConfig);
    CloseServiceHandle(handleService);
    CloseServiceHandle(handleSCManager);
    LOG(ERROR) << "Failed to query service configuration. Error: "
               << GetLastError();
    return "";
  }

  // Read the driver file path
  std::wstring driverPath = std::wstring(serviceConfig->lpBinaryPathName);
  // Get the parent directory of the driver file
  driverPath = driverPath.substr(0, driverPath.find_last_of(L"\\"));

  // Clean up resources
  LocalFree(serviceConfig);
  CloseServiceHandle(handleService);
  CloseServiceHandle(handleSCManager);

  const std::wstring systemRootPlaceholder = L"\\SystemRoot";
  if (0 != driverPath.compare(0, systemRootPlaceholder.length(),
                              systemRootPlaceholder)) {
    LOG(ERROR)
        << "The string pattern does not match. We expect that we can find "
        << std::wstring_convert<std::codecvt_utf8<wchar_t> >()
               .to_bytes(systemRootPlaceholder)
               .c_str()
        << " in the beginning of the queried path "
        << std::wstring_convert<std::codecvt_utf8<wchar_t> >()
               .to_bytes(driverPath)
               .c_str();
    return "";
  }

  const std::wstring systemRootEnv = L"windir";

  DWORD numWords = GetEnvironmentVariableW(systemRootEnv.c_str(), NULL, 0);
  if (numWords == 0) {
    LOG(ERROR) << "Failed to query the buffer size when calling "
                  "GetEnvironmentVariableW().";
    return "";
  }

  std::vector<wchar_t> systemRoot(numWords + 1);
  numWords = GetEnvironmentVariableW(systemRootEnv.c_str(), systemRoot.data(),
                                     numWords + 1);
  if (numWords == 0) {
    LOG(ERROR) << "Failed to read value from environment variables.";
    return "";
  }
  driverPath.replace(0, systemRootPlaceholder.length(),
                     std::wstring(systemRoot.data()));

  return std::wstring_convert<std::codecvt_utf8<wchar_t> >().to_bytes(
      driverPath);
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

  m_soc_info = socDetails.find(soc_id)->second;

  if (external_config) {
    LOG(INFO) << "Config settings derived externally from "
                 "//data/local/tmp/external/qti_settings.pbtxt";
    m_soc_info.m_settings = get_external_config_string();
  }
  if (soc_id == UNSUPPORTED_SOC_ID) {
    if (QTIBackendHelper::IsRuntimeAvailable(SNPE_DSP)) {
      m_soc_info.m_settings = qti_settings_default_dsp;
    } else if (QTIBackendHelper::IsRuntimeAvailable(SNPE_GPU)) {
      m_soc_info.m_settings = qti_settings_default_gpu;
    } else {
      m_soc_info.m_settings = qti_settings_default_cpu;
    }
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
  soc_info_init();
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

  // Check if this SoC is supported
  *not_allowed_message = nullptr;
  *settings = m_soc_info.m_settings.c_str();

  return true;
}

bool Socs::needs_rpcmem() {
  soc_info_init();
  return m_soc_info.m_needs_rpcmem;
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
