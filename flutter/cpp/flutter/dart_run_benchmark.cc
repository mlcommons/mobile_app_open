#include "./dart_run_benchmark.h"

#include <google/protobuf/text_format.h>

#include <chrono>
#include <fstream>
#include <iostream>
#include <mutex>

#include "flutter/cpp/backends/external.h"
#include "flutter/cpp/datasets/ade20k.h"
#include "flutter/cpp/datasets/coco.h"
#include "flutter/cpp/datasets/imagenet.h"
#include "flutter/cpp/datasets/squad.h"
#include "flutter/cpp/mlperf_driver.h"
#include "flutter/cpp/proto/backend_setting.pb.h"
#include "flutter/cpp/proto/mlperf_task.pb.h"

static ::mlperf::mobile::MlperfDriver* global_driver = nullptr;
static ::std::mutex global_driver_mutex;

static std::atomic<int32_t> datasetTotalSamples;

#define li LOG(INFO) << "li:" << __FILE__ << ":" << __LINE__ << "@" << __func__
#define lip(X) LOG(INFO) << #X "=" << in->X << ";"

struct dart_ffi_run_benchmark_out* dart_ffi_run_benchmark(
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
  lip(single_stream_expected_latency_ns);
  lip(output_dir);

  li;

  auto out = new dart_ffi_run_benchmark_out;

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

  out->backend_name = strdup(backend->Name().c_str());
  out->backend_vendor = strdup(backend->Vendor().c_str());
  out->accelerator_name = strdup(backend->AcceleratorName().c_str());

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

  datasetTotalSamples = dataset->TotalSampleCount();

  ::mlperf::mobile::MlperfDriver driver(
      std::move(dataset), std::move(backend), in->scenario,
      settings.benchmark_setting().batch_size());
  li;

  {
    ::std::lock_guard<::std::mutex> guard(global_driver_mutex);
    global_driver = &driver;
  }

  auto start = std::chrono::steady_clock::now();
  driver.RunMLPerfTest(in->mode, in->min_query_count, in->min_duration,
                       in->single_stream_expected_latency_ns, in->output_dir);
  auto end = std::chrono::steady_clock::now();
  li;

  {
    ::std::lock_guard<::std::mutex> guard(global_driver_mutex);
    global_driver = nullptr;
  }

  out->ok = 1;
  out->num_samples = driver.GetCounter();

  out->duration = std::chrono::duration<float>{end - start}.count();

  out->accuracy_normalized = driver.ComputeAccuracy();
  out->accuracy_formatted = strdup(driver.ComputeAccuracyString().c_str());

  // Second accuracy is not yet implemented in datasets
  out->accuracy_normalized2 = -1.0f;
  out->accuracy_formatted2 = strdup("");

  li;

  return out;
}

void dart_ffi_run_benchmark_free(struct dart_ffi_run_benchmark_out* out) {
  free(out->accuracy_formatted);
  free(out->accuracy_formatted2);
  free(out->backend_name);
  free(out->backend_vendor);
  free(out->accelerator_name);
  delete out;
}

int32_t dart_ffi_get_dataset_size() { return datasetTotalSamples.load(); }

int32_t dart_ffi_get_query_counter() {
  ::std::lock_guard<::std::mutex> guard(global_driver_mutex);
  if (global_driver == nullptr) {
    return -1;
  }
  return global_driver->GetCounter();
}
