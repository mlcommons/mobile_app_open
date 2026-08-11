/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly
 * prohibited.
 */
/* MediaTek Inc. (C) 2023. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY
 * ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY
 * THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK
 * SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO
 * RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN
 * FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER
 * WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT
 * ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER
 * TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#pragma once

#include <android/trace.h>
#include <sys/mman.h>
#include <sys/system_properties.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

inline std::string GetPropertyValue(const std::string& property) {
  char value[PROP_VALUE_MAX];
  if (0 == __system_property_get(property.c_str(), value)) {
    return std::string();
  }
  return std::string(value);
}

inline bool GetSystemPropertyBool(const std::string& property) {
  auto prop = GetPropertyValue(property);
  if (prop.length() == 0) {
    return false;
  }
  if (prop == "true") {
    return true;
  } else if (std::stoi(prop) > 0) {
    return true;
  }
  return false;
}

inline int GetSystemPropertyInt(const std::string& property, int default_ = 0) {
  auto prop = GetPropertyValue(property);
  return prop.length() ? std::stoi(prop) : default_;
}

inline std::vector<int32_t> ParseFile(std::ifstream& infile, char split) {
  std::vector<int> numbers;
  std::string line;
  while (std::getline(infile, line)) {
    std::stringstream ss(line);
    std::string token;
    while (std::getline(ss, token, split)) {
      size_t start = token.find_first_not_of(" \t");
      size_t end = token.find_last_not_of(" \t");
      if (start == std::string::npos) continue;
      token = token.substr(start, end - start + 1);
      int value = std::stoi(token, nullptr, 0);
      numbers.push_back(value);
    }
  }
  return numbers;
}

class Config {
 public:
  static bool GetEnableTrace() {
    return GetSystemPropertyBool("debug.mlperf.trace");
  }

  static bool GetEnableCustomBuffer() {
    return GetSystemPropertyBool("debug.mlperf.buffer.enable");
  }

  static int GetCustomInputBufferType() {
    return GetSystemPropertyInt("debug.mlperf.buffer.input");
  }

  static int GetCustomOutputBufferType() {
    return GetSystemPropertyInt("debug.mlperf.buffer.output");
  }

  static int GetDisablePerformanceLocker() {
    return GetSystemPropertyBool("debug.mlperf.performance_locker_disabled");
  }

  static int GetEnableUserDefinedPerformance() {
    return GetSystemPropertyBool("debug.mlperf.performance.enable");
  }

  static std::string GetUserDefinedPerformanceFilePath() {
    return GetPropertyValue("debug.mlperf.performance.file_path");
  }

  static int GetUserDefinedBoostHint() {
    return GetSystemPropertyInt("debug.mlperf.performance.boost_value", -1);
  }

  static int GetUserDefinedPreference() {
    return GetSystemPropertyInt("debug.mlperf.performance.preference", -1);
  }

  static std::vector<int32_t> GetUserDefinedPerformanceParam() {
    auto file = GetUserDefinedPerformanceFilePath();
    std::ifstream infile(file);
    if (!infile) {
      abort();
    }
    return ParseFile(infile, ',');
  }
};

class Tracer {
 public:
  explicit Tracer(std::string tag) {
    if (IsEnable()) {
      mTag += tag;
      ATrace_beginSection(mTag.c_str());
    }
  }
  ~Tracer() {
    if (IsEnable()) {
      ATrace_endSection();
    }
  }

 private:
  bool IsEnable() {
    static bool enable = Config::GetEnableTrace();
    return enable;
  }

  std::string mTag = "[MLPerf]";
};