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

#include "soc_utility.h"
#include <stdint.h>
#include <fstream>
#include <thread>

#include "tensorflow/core/platform/logging.h"

SocInfo unsupportedSoc = SocInfo(UNSUPPORTED_SOC_STR);
SocInfo Socs::m_soc_info;
bool Socs::is_init_done;

std::map<uint32_t, SocInfo> socDetails = SocProperties({
     // num_dsp, num_aip, num_gpu, num_cpu,
     // useDspFeatures, settings,
     // soc_name, num_inits,
     // hlc,
     // llc,
     // max_cores, needs_rpcmem
    {356,
        SocInfo(1, 6, 0, 0,
                false, qti_settings_sdm865,
                "SDM865", 1,
                std::vector<int>({0,1,2,3}),
                std::vector<int>({4,5,6,7}),
                8, false)
    },
    {415,
        SocInfo(2, 0, 4, 0,
                true, qti_settings_sdm888,
                "SDM888", 1,
                std::vector<int>({0,1,2,3}),
                std::vector<int>({4,5,6,7}),
                8, true)
    },
    {475,
        SocInfo(2, 0, 0, 0,
                true, qti_settings_sdm778,
                "SDM778", 1,
                std::vector<int>({0,1,2,3}),
                std::vector<int>({4,5,6,7}),
                8, true)
    },
    {506,
        SocInfo(2, 0, 0, 0,
                true, qti_settings_sd7g1,
                "SD7G1", 1,
                std::vector<int>({0,1,2,3}),
                std::vector<int>({4,5,6,7}),
                8, true)
    },
    {457,
        SocInfo(2, 0, 0, 0,
                true, qti_settings_sd8g1,
                "SD8G1", 1,
                std::vector<int>({0,1,2,3}),
                std::vector<int>({4,5,6,7}),
                8, true)
    },
    {552,
        SocInfo(2, 0, 0, 0,
                true, qti_settings_sd8pg1,
                "SD8PG1 4G", 1,
                std::vector<int>({0,1,2,3}),
                std::vector<int>({4,5,6,7}),
                8, true)
    },
    {540,
        SocInfo(2, 0, 0, 0,
                true, qti_settings_sd8pg1,
                "SD8PG1 prime ", 1,
                std::vector<int>({0,1,2,3}),
                std::vector<int>({4,5,6,7}),
                8, true)
    },
    {530,
        SocInfo(2, 0, 0, 0,
                true, qti_settings_sd8pg1,
                "SD8PG1", 1,
                std::vector<int>({0,1,2,3}),
                std::vector<int>({4,5,6,7}),
                8, true)
    },
}).m_soc_details;

void Socs::soc_info_init(){
    if (is_init_done) return;

    std::ifstream in_file;
    std::vector<char> line(5);
    in_file.open("/sys/devices/soc0/soc_id");
    if (in_file.fail()) {
        in_file.open("/sys/devices/system/soc/soc0/id");
    }
    if (in_file.fail()) {
        m_soc_info = unsupportedSoc;
        LOG(INFO) << "Failed to read SOC file: ";
        return;
    }

    in_file.read(line.data(), 5);
    in_file.close();
    uint32_t soc_id = (uint32_t)std::atoi(line.data());

    LOG(INFO) << "Soc ID: " << soc_id;
    if(socDetails.find(soc_id) != socDetails.end()){
        m_soc_info = socDetails.find(soc_id)->second;
    }
}

std::string Socs::get_soc_name() {
  soc_info_init();
  return m_soc_info.m_soc_name;
}

void Socs:: soc_offline_core_instance(int &num_dsp, int &num_aip,
                                      int &num_gpu, int &num_cpu,
                                      std::string &delegate) {
    soc_info_init();
    num_dsp = m_soc_info.m_num_dsp;
    num_aip = m_soc_info.m_num_aip;
    num_gpu = m_soc_info.m_num_gpu;
    num_cpu = m_soc_info.m_num_cpu;
}

int Socs::soc_num_inits() {
    soc_info_init();
    return m_soc_info.m_num_inits;
}

bool Socs::isSnapDragon(const char *manufacturer) {
    soc_info_init();
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

int Socs::soc_check_feature(bool &useIonBuffers_,
                            std::string &platformOptionStr){
    soc_info_init();
    // These features are not for SDM865, so turning them off.
    if (m_soc_info.m_useDspFeatures) {
        // use Zero copy for input and output buffers.
        // Requires rpc registered ion buffers.
        if (useIonBuffers_) {
            platformOptionStr += ";useDspZeroCopy:ON";
        }
        platformOptionStr += ";dspPowerSettingContext:ON";
        return 1;
    }
    return 0;
}

bool Socs::soc_settings(const char **settings, const char **not_allowed_message){
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
    if (m_soc_info.m_soc_name == DEFAULT_SOC_STR){
        // it's a QTI SOC, but the chipset is not yet supported
        *not_allowed_message = "Unsupported QTI SoC";
    }
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

void Socs::define_soc(std::vector<uint32_t> &allcores, std::vector<uint32_t> &low_latency_cores,
                      std::vector<uint32_t> &high_latency_cores, int &maxcores){
    soc_info_init();
    high_latency_cores.assign(m_soc_info.m_high_latency_cores.begin(),
                              m_soc_info.m_high_latency_cores.end());
    low_latency_cores.assign(m_soc_info.m_low_latency_cores.begin(),
                              m_soc_info.m_low_latency_cores.end());
    maxcores = m_soc_info.m_max_cores;
}