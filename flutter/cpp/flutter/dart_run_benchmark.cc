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
#include "flutter/cpp/datasets/snu_sr.h"
#include "flutter/cpp/datasets/squad.h"
#include "flutter/cpp/mlperf_driver.h"
#include "flutter/cpp/proto/backend_setting.pb.h"
#include "flutter/cpp/proto/mlperf_task.pb.h"

static ::mlperf::mobile::MlperfDriver* global_driver = nullptr;
static ::std::mutex global_driver_mutex;

static std::atomic<int32_t> datasetTotalSamples;

#define li LOG(INFO) << "li:" << __FILE__ << ":" << __LINE__ << "@" << __func__
#define lin(X) LOG(INFO) << "in->" << #X "=" << in->X
#define lout(X) LOG(INFO) << "out->" << #X "=" << out->X

struct dart_ffi_run_benchmark_out* dart_ffi_run_benchmark(
    const struct dart_ffi_run_benchmark_in* in) {
  lin(backend_model_path);
  lin(backend_lib_path);
  lin(backend_settings_len);
  lin(backend_native_lib_path);
  lin(dataset_type);
  lin(dataset_data_path);
  lin(dataset_groundtruth_path);
  lin(dataset_offset);
  lin(image_width);
  lin(image_height);
  lin(scenario);
  lin(mode);
  lin(batch_size);
  lin(min_query_count);
  lin(min_duration);
  lin(max_duration);
  lin(single_stream_expected_latency_ns);
  lin(output_dir);

  li;

  auto out = new dart_ffi_run_benchmark_out;

  ::mlperf::mobile::SettingList settings;
  if (settings.ParseFromArray(in->backend_settings_data,
                              in->backend_settings_len)) {
    LOG(INFO) << "Using settings:\n" << settings.DebugString();
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
          in->dataset_offset, in->image_width, in->image_height);
      break;
    case ::mlperf::mobile::DatasetConfig::COCO:
      dataset = std::make_unique<::mlperf::mobile::Coco>(
          backend.get(), in->dataset_data_path, in->dataset_groundtruth_path,
          in->dataset_offset, 91 /* num_classes, from RunMLPerfWorker.java */,
          in->image_width, in->image_height);
      break;
    case ::mlperf::mobile::DatasetConfig::SQUAD:
      dataset = std::make_unique<::mlperf::mobile::Squad>(
          backend.get(), in->dataset_data_path, in->dataset_groundtruth_path);
      break;
    case ::mlperf::mobile::DatasetConfig::ADE20K:
      dataset = std::make_unique<::mlperf::mobile::ADE20K>(
          backend.get(), in->dataset_data_path, in->dataset_groundtruth_path,
          31 /* num_classes, from RunMLPerfWorker.java */, in->image_width,
          in->image_height);
      break;
    case ::mlperf::mobile::DatasetConfig::SNUSR:
      dataset = std::make_unique<::mlperf::mobile::SNUSR>(
          backend.get(), in->dataset_data_path, in->dataset_groundtruth_path,
          3 /* num_channels */, 2 /* scale */, in->image_width,
          in->image_height);
      break;
    default:
      return nullptr;
  }
  li;

  datasetTotalSamples = dataset->TotalSampleCount();

  ::mlperf::mobile::MlperfDriver driver(std::move(dataset), std::move(backend),
                                        in->scenario, in->batch_size);
  li;

  {
    ::std::lock_guard<::std::mutex> guard(global_driver_mutex);
    global_driver = &driver;
  }

  auto start = std::chrono::steady_clock::now();
  driver.RunMLPerfTest(in->mode, in->min_query_count, in->min_duration,
                       in->max_duration, in->single_stream_expected_latency_ns,
                       in->output_dir);
  auto end = std::chrono::steady_clock::now();
  li;

  {
    ::std::lock_guard<::std::mutex> guard(global_driver_mutex);
    global_driver = nullptr;
  }

  out->run_ok = true;
  out->num_samples = driver.GetCounter();

  out->duration = std::chrono::duration<float>{end - start}.count();

  if (driver.HasAccuracy()) {
    out->accuracy1 = new dart_ffi_run_benchmark_out_accuracy;
    out->accuracy1->normalized = driver.ComputeAccuracy();
    out->accuracy1->formatted = strdup(driver.ComputeAccuracyString().c_str());
  } else {
    out->accuracy1 = nullptr;
  }

  // Second accuracy is not yet implemented in datasets
  out->accuracy2 = nullptr;

  li;
  lout(run_ok);
  lout(num_samples);
  lout(duration);
  lout(accuracy1);
  lout(accuracy2);
  lout(backend_name);
  lout(backend_vendor);
  lout(accelerator_name);

  return out;
}

void dart_ffi_run_benchmark_free(struct dart_ffi_run_benchmark_out* out) {
  if (out->accuracy1 != nullptr) {
    free(out->accuracy1->formatted);
    delete out->accuracy1;
  }
  if (out->accuracy2 != nullptr) {
    free(out->accuracy2->formatted);
    delete out->accuracy2;
  }
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
