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

#include <memory>
#include <string>

#include "android/cpp/backends/external.h"
#include "android/cpp/proto/backend_setting.pb.h"
#include "google/protobuf/text_format.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

JNIEXPORT jbyteArray JNICALL
Java_org_mlperf_inference_MiddleInterface_convertProto(JNIEnv* env,
                                                       jclass clazz,
                                                       jstring jtext) {
  // Convert parameters to C++.
  std::string text = env->GetStringUTFChars(jtext, nullptr);
  std::string binary;

  // Convert text proto to binary.
  mlperf::mobile::BackendSetting settings;
  if (!google::protobuf::TextFormat::ParseFromString(text, &settings)) {
    env->ThrowNew(env->FindClass("java/lang/Exception"),
                  "Failed to parse the proto file. Please check its format.");
  }
  if (!settings.SerializeToString(&binary)) {
    env->ThrowNew(env->FindClass("java/lang/Exception"),
                  "Failed to write proto to string.");
  }
  jbyteArray result = env->NewByteArray(binary.size());
  env->SetByteArrayRegion(result, 0, binary.size(), (jbyte*)binary.c_str());
  return result;
}

// See if backend is supported
JNIEXPORT jobjectArray JNICALL
Java_org_mlperf_inference_MiddleInterface_isBackendSupported(
    JNIEnv* env, jclass clazz, jstring jlib_path, jstring jmanufacturer,
    jstring jmodel) {
  std::string lib_path = env->GetStringUTFChars(jlib_path, nullptr);
  std::string manufacturer = env->GetStringUTFChars(jmanufacturer, nullptr);
  std::string model = env->GetStringUTFChars(jmodel, nullptr);
  const char* pbdata;
  std::string msg = mlperf::mobile::BackendFunctions::isSupported(
      lib_path, manufacturer, model, &pbdata);
  jobjectArray retstrings =
      env->NewObjectArray(2, env->FindClass("java/lang/String"), 0);
  if (!pbdata) pbdata = "";
  jstring str = env->NewStringUTF(pbdata);
  jstring result = env->NewStringUTF(msg.c_str());
  env->SetObjectArrayElement(retstrings, 0, result);
  env->SetObjectArrayElement(retstrings, 1, str);
  return retstrings;
}

JNIEXPORT jlong JNICALL
Java_org_mlperf_inference_MLPerfDriverWrapper_externalBackend(
    JNIEnv* env, jclass clazz, jstring jmodel_file_path, jstring jlib_path,
    jbyteArray jsettings, jstring jnative_lib_path) {
  // Convert parameters to C++.
  std::string model_file_path =
      env->GetStringUTFChars(jmodel_file_path, nullptr);
  std::string lib_path = env->GetStringUTFChars(jlib_path, nullptr);
  std::string native_lib_path =
      env->GetStringUTFChars(jnative_lib_path, nullptr);
  // Convert jbyteArray to C++ string.
  int len = env->GetArrayLength(jsettings);
  char* buf = new char[len];
  env->GetByteArrayRegion(jsettings, 0, len, reinterpret_cast<jbyte*>(buf));
  std::string settings_str(buf, len);
  delete[] buf;

  // Create a new ExternalBackend object.
  mlperf::mobile::SettingList settings;
  settings.ParseFromString(settings_str);
  std::unique_ptr<mlperf::mobile::ExternalBackend> backend_ptr(
      new mlperf::mobile::ExternalBackend(model_file_path, lib_path, settings,
                                          native_lib_path));
  return reinterpret_cast<jlong>(backend_ptr.release());
}

JNIEXPORT void JNICALL
Java_org_mlperf_inference_MLPerfDriverWrapper_nativeDeleteBackend(
    JNIEnv* env, jclass clazz, jlong handle) {
  if (handle != 0) {
    delete reinterpret_cast<mlperf::mobile::Backend*>(handle);
  }
}

// Get setting as serialized string.
JNIEXPORT jbyteArray JNICALL
Java_org_mlperf_inference_MLPerfDriverWrapper_getBackendSettings(
    JNIEnv* env, jclass clazz, jlong backend_handle) {
  std::string settings;
  reinterpret_cast<mlperf::mobile::Backend*>(backend_handle)
      ->GetSettings()
      .SerializeToString(&settings);

  // Copy to jbyteArray.
  jbyteArray array = env->NewByteArray(settings.size());
  env->SetByteArrayRegion(array, 0, settings.size(),
                          reinterpret_cast<const jbyte*>(settings.c_str()));
  return array;
}

// Get setting as serialized string.
JNIEXPORT void JNICALL
Java_org_mlperf_inference_MLPerfDriverWrapper_setBackendSettings(
    JNIEnv* env, jclass clazz, jlong backend_handle, jbyteArray jsettings) {
  int len = env->GetArrayLength(jsettings);
  char* buf = new char[len];
  env->GetByteArrayRegion(jsettings, 0, len, reinterpret_cast<jbyte*>(buf));
  std::string settings_str(buf, len);
  delete[] buf;

  mlperf::mobile::BackendSetting settings;
  settings.ParseFromString(settings_str);
  reinterpret_cast<mlperf::mobile::Backend*>(backend_handle)
      ->SetSettings(settings);
}

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
