/* Copyright 2020-2025 Samsung Electronics Co. LTD  All Rights Reserved.

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
#include "mbe_helper.hpp"

#include <sched.h>
#include <string.h>

#include <array>
#include <map>

namespace mbe {
static std::string get_core_info() {
  std::array<char, 128> buffer;
  std::string core_info;

  std::unique_ptr<FILE, decltype(&pclose)> pipe(
      popen("getprop | grep ro.hardware", "r"), pclose);
  if (!pipe) {
    MLOGD("Can not find Samsung specific information");
    return "";
  }
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    core_info += buffer.data();
  }
  MLOGD("completed hardware info [%s]", core_info.c_str());
  return core_info;
}

static bool get_manufacturer(const char *manufacturer) {
  MLOGD("Check for support  manufacturer[%s]", manufacturer);
  if (strstr((char *)manufacturer, "samsung") ||
      strstr((char *)manufacturer, "Samsung"))
    return true;
  return false;
}

static bool retrieve_model_surfix(const char *model, int core_id) {
  MLOGD("Check for surfix model[%s] with [%d]", model, core_id);
  if (core_id == SOC_2200) {
    if (strstr((char *)model, "B") || strstr((char *)model, "B/DS"))
      return true;
    else if (strstr((char *)model, "UNIVERSAL") ||
             strstr((char *)model, "universal"))
      return true;
    else if (strstr((char *)model, "ERD") || strstr((char *)model, "erd"))
      return true;
  } else {
    /* TBD 1200, 2100. 2300*/
    return true;
  }
  return false;
}

static int get_core_id_from_model(const char *model) {
  MLOGD("Check for support model[%s]", model);
  int core_id = CORE_INVALID;

  if (strstr((char *)model, "ERD8825") || strstr((char *)model, "A536") ||
      strstr((char *)model, "S5E8825"))
    core_id = SOC_1200;
  else if (strstr((char *)model, "G998") || strstr((char *)model, "G996") ||
           strstr((char *)model, "G991") || strstr((char *)model, "G998") ||
           strstr((char *)model, "UNIVERSAL2100"))
    core_id = SOC_2100;
  else if (strstr((char *)model, "ERD9925") ||
           strstr((char *)model, "S5E9925") || strstr((char *)model, "S901") ||
           strstr((char *)model, "S906") || strstr((char *)model, "S908") ||
           strstr((char *)model, "S711") || strstr((char *)model, "S23 FE"))
    core_id = SOC_2200;
  else if (strstr((char *)model, "ERD9935") ||
           strstr((char *)model, "S5E9935") || strstr((char *)model, "S919O"))
    core_id = SOC_2300;
  else if (strstr((char *)model, "ERD9945") ||
           strstr((char *)model, "S5E9945") || strstr((char *)model, "MU3S") ||
           strstr((char *)model, "S921") || strstr((char *)model, "S926") ||
           strstr((char *)model, "S928") || strstr((char *)model, "S721") ||
           strstr((char *)model, "F761"))
    core_id = SOC_2400;
  else if (strstr((char *)model, "ERD9955") ||
           strstr((char *)model, "S5E9955") || strstr((char *)model, "NE1S") ||
           strstr((char *)model, "S931") || strstr((char *)model, "S936") ||
           strstr((char *)model, "S938") || strstr((char *)model, "F766"))
    core_id = SOC_2500;
  else
    return CORE_INVALID;
  return retrieve_model_surfix(model, core_id) ? core_id : CORE_INVALID;
}

static int get_core_id_from_hardware(const char *hardware) {
  MLOGD("Check for support hardware[%s]", hardware);
  int core_id = CORE_INVALID;

  if (strstr((char *)hardware, "8825"))
    core_id = SOC_1200;
  else if (strstr((char *)hardware, "2100"))
    core_id = SOC_2100;
  else if (strstr((char *)hardware, "9925"))
    core_id = SOC_2200;
  else if (strstr((char *)hardware, "9935"))
    core_id = SOC_2300;
  else if (strstr((char *)hardware, "9945"))
    core_id = SOC_2400;
  else if (strstr((char *)hardware, "9955"))
    core_id = SOC_2500;
  else
    return CORE_INVALID;
  return core_id;
}

int core_ctrl::support_mbe(const char *manufacturer, const char *model) {
  std::string version = VERSION(MAJOR, MINOR, PATCH);
  MLOGV("mbe version [%s]", version.c_str());
  bool is_samsung = get_manufacturer(manufacturer);
  if (!is_samsung) {
    return CORE_INVALID;
  }

  int core_id = get_core_id_from_model(model);
  if (core_id > CORE_INVALID) {
    return core_id;
  }

  core_id = get_core_id();
  if (core_id > CORE_INVALID) {
    return core_id;
  }
  return CORE_INVALID;
}

const char *core_ctrl::get_benchmark_config(int core_id) {
  const char *settings = nullptr;

  if (core_id == SOC_1200) {
    settings = mbe_config_1200_pbtxt.c_str();
  } else if (core_id == SOC_2100) {
    settings = mbe_config_2100_pbtxt.c_str();
  } else if (core_id == SOC_2200) {
    settings = mbe_config_2200_pbtxt.c_str();
  } else if (core_id == SOC_2300) {
    settings = mbe_config_2300_pbtxt.c_str();
  } else if (core_id == SOC_2400) {
    settings = mbe_config_2400_pbtxt.c_str();
  } else if (core_id == SOC_2500) {
    settings = mbe_config_2500_pbtxt.c_str();
  }
  return settings;
}

int core_ctrl::get_core_id() {
  std::string temp = get_core_info();
  const char *core_model = temp.c_str();
  MLOGD("check system info. core[%s]", core_model);
  if (!core_model) return CORE_INVALID;
  return get_core_id_from_hardware(core_model);
}
}  // namespace mbe
