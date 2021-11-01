#include "./main.h"

#include <google/protobuf/text_format.h>

#include <fstream>
#include <iostream>
#include <mutex>

#include "cpp/backends/external.h"
#include "cpp/c/backend_c.h"
#include "cpp/datasets/ade20k.h"
#include "cpp/datasets/coco.h"
#include "cpp/datasets/imagenet.h"
#include "cpp/datasets/squad.h"
#include "cpp/mlperf_driver.h"
#include "cpp/proto/backend_setting.pb.h"
#include "cpp/proto/mlperf_task.pb.h"

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

#define li \
  std::cout << "li:" << __FILE__ << ":" << __LINE__ << "@" << __func__ << "\n"
#define lip(X) std::cout << #X "=" << in->X << ";\n"

extern "C" void stop_backend() {
  {
    ::std::lock_guard<::std::mutex> guard(global_driver_mutex);
    if (global_driver) {
      li;
      global_driver->AbortMLPerfTest();
      li;
    }
  }
}

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
  lip(batch);

  lip(mode);
  lip(min_query_count);
  lip(min_duration);
  lip(output_dir);

  li;

  std::cout.flush();

  ::mlperf::mobile::SettingList settings;
  if (settings.ParseFromArray(in->backend_settings_data,
                              in->backend_settings_len)) {
    std::string s;
    google::protobuf::TextFormat::PrintToString(settings, &s);
    std::cout << "Using settings:\n" << s << "\n";
  } else {
    std::cout << "ERROR parsing settings\n";
    return nullptr;
  }
  li;

  auto backend = ::std::make_unique<::mlperf::mobile::ExternalBackend>(
      in->backend_model_path, in->backend_lib_path, settings,
      in->backend_native_lib_path);
  li;

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

  ::mlperf::mobile::MlperfDriver driver(std::move(dataset), std::move(backend),
                                        in->scenario, in->batch);
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
  li;

  return out;
}

void dart_ffi_run_benchmark_free(struct dart_ffi_run_benchmark_out* out) {
  free(out->accuracy);
  delete out;
}
