#include "./main.h"

#include <google/protobuf/text_format.h>

#include <fstream>
#include <iostream>
#include <mutex>

#include "flutter/cpp/backends/external.h"
#include "flutter/cpp/c/backend_c.h"
#include "flutter/cpp/datasets/ade20k.h"
#include "flutter/cpp/datasets/coco.h"
#include "flutter/cpp/datasets/imagenet.h"
#include "flutter/cpp/datasets/squad.h"
#include "flutter/cpp/mlperf_driver.h"
#include "flutter/cpp/proto/backend_setting.pb.h"
#include "flutter/cpp/proto/mlperf_task.pb.h"

// On iOS we link backend statically and don't export any functions
// Linker sees that we don't use some of the functions and removes them
// (when building for release of profile modes)
// But we still want to access these functions dynamically,
// so we need fake_calls() function to prevent linker from removing them.
extern "C" void fake_calls() {
  volatile intptr_t a = 1;
  if (a) return;
  a = (intptr_t)dart_ffi_run_benchmark;
  a = (intptr_t)dart_ffi_run_benchmark_free;
  a = (intptr_t)dart_ffi_mlperf_config;
  a = (intptr_t)dart_ffi_mlperf_config_free;
  a = (intptr_t)dart_ffi_backend_match;
  a = (intptr_t)dart_ffi_backend_match_free;
#ifdef __APPLE__
  a = (intptr_t)mlperf_backend_matches_hardware;
  a = (intptr_t)mlperf_backend_create;
  a = (intptr_t)mlperf_backend_vendor_name;
  a = (intptr_t)mlperf_backend_accelerator_name;
  a = (intptr_t)mlperf_backend_name;
  a = (intptr_t)mlperf_backend_delete;
  a = (intptr_t)mlperf_backend_issue_query;
  a = (intptr_t)mlperf_backend_flush_queries;
  a = (intptr_t)mlperf_backend_get_input_count;
  a = (intptr_t)mlperf_backend_get_input_type;
  a = (intptr_t)mlperf_backend_set_input;
  a = (intptr_t)mlperf_backend_get_output_count;
  a = (intptr_t)mlperf_backend_get_output_type;
  a = (intptr_t)mlperf_backend_get_output;
#endif
}

static ::mlperf::mobile::MlperfDriver* global_driver = nullptr;
static ::std::mutex global_driver_mutex;

#define li LOG(INFO) << "li:" << __FILE__ << ":" << __LINE__ << "@" << __func__
#define lip(X) LOG(INFO) << #X "=" << in->X << ";"

extern "C" struct dart_ffi_run_benchmark_out* dart_ffi_run_benchmark(
    const struct dart_ffi_run_benchmark_in* in) {
  lip(backend_model_path);
  lip(backend_lib_path);
  lip(backend_settings_len);
  lip(backend_native_lib_path);

  lip(dataset_type);
  lip(dataset_data_path);
  lip(dataset_groundtruth_path);
  lip(dataset_offset);

  lip(scenario);

  lip(mode);
  lip(min_query_count);
  lip(min_duration);
  lip(output_dir);

  li;

  ::mlperf::mobile::SettingList settings;
  if (settings.ParseFromArray(in->backend_settings_data,
                              in->backend_settings_len)) {
    std::string s;
    google::protobuf::TextFormat::PrintToString(settings, &s);
    LOG(INFO) << "Using settings:\n" << s;
  } else {
    LOG(ERROR) << "ERROR parsing settings";
    return nullptr;
  }
  li;

  auto backend = ::std::make_unique<::mlperf::mobile::ExternalBackend>(
      in->backend_model_path, in->backend_lib_path, settings,
      in->backend_native_lib_path);
  li;

  char* backend_name = strdup(backend->Name().c_str());
  char* backend_vendor = strdup(backend->Vendor().c_str());
  char* accelerator_name = strdup(backend->AcceleratorName().c_str());

  ::std::unique_ptr<::mlperf::mobile::Dataset> dataset;
  switch (in->dataset_type) {
    case ::mlperf::mobile::DatasetConfig::IMAGENET:
      dataset = std::make_unique<::mlperf::mobile::Imagenet>(
          backend.get(), in->dataset_data_path, in->dataset_groundtruth_path,
          in->dataset_offset, 224, 224 /* width, height */);
      break;
    case ::mlperf::mobile::DatasetConfig::COCO:
      dataset = std::make_unique<::mlperf::mobile::Coco>(
          backend.get(), in->dataset_data_path, in->dataset_groundtruth_path,
          in->dataset_offset, 91 /* num_classes, from RunMLPerfWorker.java */,
          320, 320 /* width, height */);
      break;
    case ::mlperf::mobile::DatasetConfig::SQUAD:
      dataset = std::make_unique<::mlperf::mobile::Squad>(
          backend.get(), in->dataset_data_path, in->dataset_groundtruth_path);
      break;
    case ::mlperf::mobile::DatasetConfig::ADE20K:
      dataset = std::make_unique<::mlperf::mobile::ADE20K>(
          backend.get(), in->dataset_data_path, in->dataset_groundtruth_path,
          31 /* num_classes, from RunMLPerfWorker.java */, 512,
          512 /* width, height */);
      break;
    default:
      return nullptr;
  }
  li;

  ::mlperf::mobile::MlperfDriver driver(
      std::move(dataset), std::move(backend), in->scenario,
      settings.benchmark_setting().batch_size());
  li;

  {
    ::std::lock_guard<::std::mutex> guard(global_driver_mutex);
    global_driver = &driver;
  }

  driver.RunMLPerfTest(in->mode, in->min_query_count, in->min_duration,
                       in->output_dir);
  li;

  {
    ::std::lock_guard<::std::mutex> guard(global_driver_mutex);
    global_driver = nullptr;
  }

  auto out = new dart_ffi_run_benchmark_out;
  out->ok = 1;
  out->latency = driver.ComputeLatency();
  out->accuracy = strdup(driver.ComputeAccuracyString().c_str());
  out->num_samples = driver.GetNumSamples();
  out->duration_ms = driver.GetDurationMs();
  out->backend_name = backend_name;
  out->backend_vendor = backend_vendor;
  out->accelerator_name = accelerator_name;
  li;

  return out;
}

void dart_ffi_run_benchmark_free(struct dart_ffi_run_benchmark_out* out) {
  free(out->accuracy);
  free(out->backend_name);
  free(out->backend_vendor);
  free(out->accelerator_name);
  delete out;
}
