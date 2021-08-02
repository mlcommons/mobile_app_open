/* Copyright 2019 The MLPerf Authors. All Rights Reserved.

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
#include <jni.h>

#include <fstream>
#include <memory>
#include <streambuf>
#include <string>

#include "cpp/backend.h"
#include "cpp/dataset.h"
#include "cpp/mlperf_driver.h"
#include "cpp/proto/backend_setting.pb.h"
#include "cpp/proto/mlperf_task.pb.h"
#include "google/protobuf/text_format.h"
#include "tensorflow/lite/java/src/main/native/jni_utils.h"

using mlperf::mobile::Backend;
using mlperf::mobile::Dataset;
using mlperf::mobile::MlperfDriver;

MlperfDriver* convertLongToMlperfDriver(JNIEnv* env, jlong handle) {
  if (handle == 0) {
    tflite::jni::ThrowException(
        env, kIllegalArgumentException,
        "Internal error: Invalid handle to MlperfDriver.");
    return nullptr;
  }
  return reinterpret_cast<MlperfDriver*>(handle);
}

std::string readFile(JNIEnv* env, jstring jfilePath) {
  std::string filePath = env->GetStringUTFChars(jfilePath, nullptr);
  std::string protoString;
  std::ifstream stream(filePath);
  if (!stream) {
    env->ThrowNew(env->FindClass("java/io/FileNotFoundException"),
                  "File not found");
    return protoString;
  }
  std::stringstream buffer;
  buffer << stream.rdbuf();

  protoString = buffer.str();
  return protoString;
}

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

JNIEXPORT jlong JNICALL
Java_org_mlperf_inference_MLPerfDriverWrapper_nativeInit(
    JNIEnv* env, jclass clazz, jlong dataset_handle, jlong backend_handle,
    jstring jscenario, jint batch) {
  if (dataset_handle == 0 || backend_handle == 0) {
    tflite::jni::ThrowException(env, kIllegalArgumentException,
                                "Internal error: Invalid handle.");
  }
  Dataset* dataset = reinterpret_cast<Dataset*>(dataset_handle);
  Backend* backend = reinterpret_cast<Backend*>(backend_handle);
  std::string scenario = env->GetStringUTFChars(jscenario, nullptr);
  // create a new MlperfDriver
  std::unique_ptr<MlperfDriver> driver_ptr(
      new MlperfDriver(std::unique_ptr<Dataset>(dataset),
                       std::unique_ptr<Backend>(backend), scenario, batch));
  return reinterpret_cast<jlong>(driver_ptr.release());
}

JNIEXPORT void JNICALL Java_org_mlperf_inference_MLPerfDriverWrapper_nativeRun(
    JNIEnv* env, jclass clazz, jlong driver_handle, jstring jmode,
    jint min_query_count, jint min_duration, jstring joutput_dir) {
  // Convert parameters to C++.
  std::string mode = env->GetStringUTFChars(jmode, nullptr);
  std::string output_dir = env->GetStringUTFChars(joutput_dir, nullptr);
  // Start the test.
  convertLongToMlperfDriver(env, driver_handle)
      ->RunMLPerfTest(mode, min_query_count, min_duration, output_dir);
}

JNIEXPORT jfloat JNICALL
Java_org_mlperf_inference_MLPerfDriverWrapper_nativeGetLatency(
    JNIEnv* env, jclass clazz, jlong helper_handle) {
  return convertLongToMlperfDriver(env, helper_handle)->ComputeLatency();
}

JNIEXPORT jstring JNICALL
Java_org_mlperf_inference_MLPerfDriverWrapper_nativeGetAccuracy(
    JNIEnv* env, jclass clazz, jlong helper_handle) {
  std::string accuracy =
      convertLongToMlperfDriver(env, helper_handle)->ComputeAccuracyString();
  return env->NewStringUTF(accuracy.c_str());
}

JNIEXPORT jlong JNICALL
Java_org_mlperf_inference_MLPerfDriverWrapper_nativeGetNumSamples(
    JNIEnv* env, jclass clazz, jlong helper_handle) {
  return convertLongToMlperfDriver(env, helper_handle)->GetNumSamples();
}

JNIEXPORT jfloat JNICALL
Java_org_mlperf_inference_MLPerfDriverWrapper_nativeGetDurationMs(
    JNIEnv* env, jclass clazz, jlong helper_handle) {
  return convertLongToMlperfDriver(env, helper_handle)->GetDurationMs();
}

JNIEXPORT void JNICALL
Java_org_mlperf_inference_MLPerfDriverWrapper_nativeDelete(
    JNIEnv* env, jclass clazz, jlong driver_handle) {
  if (driver_handle != 0) {
    delete convertLongToMlperfDriver(env, driver_handle);
  }
}

JNIEXPORT jbyteArray JNICALL
Java_org_mlperf_inference_MLPerfDriverWrapper_readConfigFromFile(
    JNIEnv* env, jclass clazz, jstring jconfigPath) {
  jbyteArray result = env->NewByteArray(0);

  std::string protoString = readFile(env, jconfigPath);
  // Check for exception in readFile
  jthrowable exc = env->ExceptionOccurred();
  if (exc) {
    // File was not found.
    return result;
  }

  mlperf::mobile::MLPerfConfig config;
  if (!google::protobuf::TextFormat::ParseFromString(protoString, &config)) {
    env->ThrowNew(env->FindClass("java/lang/Exception"),
                  "Failed to parse the task proto file.");
    return result;
  }
  std::string binaryString;
  if (!config.SerializeToString(&binaryString)) {
    env->ThrowNew(env->FindClass("java/lang/Exception"),
                  "Failed to serialize proto to string.");
    return result;
  }

  result = env->NewByteArray(binaryString.size());
  env->SetByteArrayRegion(result, 0, binaryString.size(),
                          (jbyte*)binaryString.c_str());
  return result;
}

JNIEXPORT jbyteArray JNICALL
Java_org_mlperf_inference_MLPerfDriverWrapper_readSettingsFromFile(
    JNIEnv* env, jclass clazz, jstring jsettingsPath) {
  jbyteArray result = env->NewByteArray(0);
  std::string protoString = readFile(env, jsettingsPath);

  // Check for exception in readFile
  jthrowable exc = env->ExceptionOccurred();
  if (exc) {
    // File was not found.
    return result;
  }

  mlperf::mobile::BackendSetting settings;
  if (!google::protobuf::TextFormat::ParseFromString(protoString, &settings)) {
    env->ThrowNew(env->FindClass("java/lang/Exception"),
                  "Failed to parse the settings file.");
    return result;
  }
  std::string binaryString;
  if (!settings.SerializeToString(&binaryString)) {
    env->ThrowNew(env->FindClass("java/lang/Exception"),
                  "Failed to serialize proto to string.");
    return result;
  }

  result = env->NewByteArray(binaryString.size());
  env->SetByteArrayRegion(result, 0, binaryString.size(),
                          (jbyte*)binaryString.c_str());
  return result;
}

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
