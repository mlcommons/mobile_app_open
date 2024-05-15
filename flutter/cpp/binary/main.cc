/* Copyright 2019-2021,2024 The MLPerf Authors. All Rights Reserved.

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
#include <sys/stat.h>

#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "absl/strings/match.h"
#include "flutter/cpp/backends/external.h"
#include "flutter/cpp/datasets/ade20k.h"
#include "flutter/cpp/datasets/coco.h"
#include "flutter/cpp/datasets/imagenet.h"
#include "flutter/cpp/datasets/snu_sr.h"
#include "flutter/cpp/datasets/squad.h"
#include "flutter/cpp/mlperf_driver.h"
#include "flutter/cpp/proto/mlperf_task.pb.h"
#include "flutter/cpp/utils.h"
#include "google/protobuf/text_format.h"
#include "tensorflow/lite/tools/command_line_flags.h"

namespace mlperf {
namespace mobile {
namespace {
// Supported backends.
enum class BackendType {
  NONE = 0,
  TFLITE = 1,
  EXTERNAL = 2,
};

BackendType Str2BackendType(absl::string_view name) {
  if (absl::EqualsIgnoreCase(name, "TFLITE")) {
    return BackendType::TFLITE;
  } else if (absl::EqualsIgnoreCase(name, "EXTERNAL")) {
    return BackendType::EXTERNAL;
  } else {
    return BackendType::NONE;
  }
}

DatasetConfig::DatasetType Str2DatasetType(absl::string_view name) {
  if (absl::EqualsIgnoreCase(name, "COCO")) {
    return DatasetConfig::COCO;
  } else if (absl::EqualsIgnoreCase(name, "IMAGENET")) {
    return DatasetConfig::IMAGENET;
  } else if (absl::EqualsIgnoreCase(name, "SQUAD")) {
    return DatasetConfig::SQUAD;
  } else if (absl::EqualsIgnoreCase(name, "ADE20K")) {
    return DatasetConfig::ADE20K;
  } else if (absl::EqualsIgnoreCase(name, "SNUSR")) {
    return DatasetConfig::SNUSR;
  } else if (absl::EqualsIgnoreCase(name, "DUMMY")) {
    return DatasetConfig::NONE;
  } else {
    LOG(FATAL) << "Unregconized dataset type: " << name;
    return DatasetConfig::NONE;
  }
}

DatasetConfig::DatasetType BenchmarkId2DatasetType(absl::string_view name) {
  if (absl::StartsWith(name, "image_classification")) {
    return DatasetConfig::IMAGENET;
  } else if (absl::StartsWith(name, "object_detection")) {
    return DatasetConfig::COCO;
  } else if (absl::StartsWith(name, "natural_language_processing")) {
    return DatasetConfig::SQUAD;
  } else if (absl::StartsWith(name, "image_segmentation")) {
    return DatasetConfig::ADE20K;
  } else if (absl::StartsWith(name, "super_resolution")) {
    return DatasetConfig::SNUSR;
  } else {
    LOG(FATAL) << "Unrecognized benchmark_id: " << name;
    return DatasetConfig::NONE;
  }
}

}  // namespace

int Main(int argc, char *argv[]) {
  using tflite::Flag;
  using tflite::Flags;
  std::string command_line = argv[0];
  // Flags for backend and dataset.
  std::string backend_name, benchmark_id;
  BackendType backend_type = BackendType::NONE;
  DatasetConfig::DatasetType dataset_type = DatasetConfig::NONE;
  std::vector<Flag> flag_list{
      Flag::CreateFlag("backend", &backend_name,
                       "Backend. Only TFLite is supported at the moment.",
                       Flag::kPositional),
      Flag::CreateFlag(
          "benchmark", &benchmark_id,
          "Benchmark ID. One of image_classification, "
          "image_classification_v2, object_detection, "
          "natural_language_processing, "
          "image_segmentation_v2, super_resolution, "
          "image_classification_offline, image_classification_offline_v2",
          Flag::kPositional)};
  Flags::Parse(&argc, const_cast<const char **>(argv), flag_list);
  backend_type = Str2BackendType(backend_name);
  if (backend_type == BackendType::NONE) {
    LOG(FATAL) << Flags::Usage(command_line, flag_list);
    return 1;
  }
  dataset_type = BenchmarkId2DatasetType(benchmark_id);
  if (dataset_type == DatasetConfig::NONE) {
    LOG(FATAL) << Flags::Usage(command_line, flag_list);
    return 1;
  }

  // Treats positional flags as subcommands.
  command_line += " " + backend_name + " " + benchmark_id;

  // Command Line Flags for mlperf.
  std::string mode, scenario = "SingleStream", output_dir;
  int min_query_count = 100, min_duration_ms = 100,
      max_duration_ms = 10 * 60 * 1000,
      single_stream_expected_latency_ns = 1000000;
  flag_list.clear();
  flag_list.insert(
      flag_list.end(),
      {Flag::CreateFlag("mode", &mode,
                        "Mode is one among PerformanceOnly, "
                        "AccuracyOnly, SubmissionRun.",
                        Flag::kRequired),
       Flag::CreateFlag("min_query_count", &min_query_count,
                        "The test will guarantee to run at least this "
                        "number of samples in performance mode."),
       Flag::CreateFlag("min_duration_ms", &min_duration_ms,
                        "The test will guarantee to run at least this "
                        "duration in performance mode. The duration is in ms."),
       Flag::CreateFlag("max_duration_ms", &max_duration_ms,
                        "The test will early exit when max duration is reached."
                        "The duration is in ms."),
       Flag::CreateFlag("single_stream_expected_latency_ns",
                        &single_stream_expected_latency_ns,
                        "A hint used by the loadgen to pre-generate "
                        "enough samples to meet the minimum test duration."),
       Flag::CreateFlag("output_dir", &output_dir,
                        "The output directory of mlperf.", Flag::kRequired)});

  // Command Line Flags for backend.
  std::unique_ptr<Backend> backend;
  std::unique_ptr<Dataset> dataset;

  int batch_size = 1;
  switch (backend_type) {
    case BackendType::EXTERNAL: {
      LOG(INFO) << "Using External backend";
      std::string model_file_path;
      std::string lib_path;
      std::string native_lib_path;
      flag_list.insert(
          flag_list.end(),
          {Flag::CreateFlag("model_file", &model_file_path,
                            "Path to model file.", Flag::kRequired),
           Flag::CreateFlag("lib_path", &lib_path,
                            "Path to the backend library .so file."),
           Flag::CreateFlag(
               "native_lib_path", &native_lib_path,
               "Path to the additional .so files for the backend."),
           Flag::CreateFlag("scenario", &scenario,
                            "Scenario to run the benchmark."),
           Flag::CreateFlag("batch_size", &batch_size, "Batch size.")});

      if (Flags::Parse(&argc, const_cast<const char **>(argv), flag_list)) {
        const char *pbdata;
        std::string msg = mlperf::mobile::BackendFunctions::isSupported(
            lib_path, native_lib_path, "Unknown manufacturer", "Unknown model",
            &pbdata);
        std::string backend_setting_string(pbdata, strlen(pbdata));
        BackendSetting backend_setting;
        google::protobuf::TextFormat::ParseFromString(pbdata, &backend_setting);

        // If batch_size flag is set, override the backend_setting
        for (auto &bs : *backend_setting.mutable_benchmark_setting()) {
          if (bs.benchmark_id() != benchmark_id) {
            continue;
          }
          for (auto &ds : *bs.mutable_delegate_choice()) {
            if (batch_size > 1) {
              ds.set_batch_size(batch_size);
              LOG(INFO) << "Override benchmark " << benchmark_id
                        << " with batch_size " << batch_size;
            } else {
              batch_size = ds.batch_size() == 0 ? 1 : ds.batch_size();
            }
          }
        }

        SettingList setting_list =
            createSettingList(backend_setting, benchmark_id);

        ExternalBackend *external_backend = new ExternalBackend(
            model_file_path, lib_path, setting_list, native_lib_path);
        backend.reset(external_backend);
      }
    } break;
    default:
      break;
  }

  // Command Line Flags for dataset.
  switch (dataset_type) {
    case DatasetConfig::IMAGENET: {
      LOG(INFO) << "Using Imagenet dataset";
      std::string images_directory, groundtruth_file;
      int offset = 1, image_width = 224, image_height = 224;
      if (benchmark_id == "image_classification_v2" ||
          benchmark_id == "image_classification_offline_v2") {
        offset = 0;
        image_width = 384;
        image_height = 384;
      }
      std::vector<Flag> dataset_flags{
          Flag::CreateFlag("images_directory", &images_directory,
                           "Path to ground truth images.", Flag::kRequired),
          Flag::CreateFlag("offset", &offset,
                           "The offset of the first meaningful class in the "
                           "classification model.",
                           Flag::kRequired),
          Flag::CreateFlag("groundtruth_file", &groundtruth_file,
                           "Path to the imagenet ground truth file.",
                           Flag::kRequired),
          Flag::CreateFlag("image_width", &image_width,
                           "The width of the processed image."),
          Flag::CreateFlag("image_height", &image_height,
                           "The height of the processed image."),
      };
      if (Flags::Parse(&argc, const_cast<const char **>(argv), dataset_flags) &&
          backend) {
        dataset.reset(new Imagenet(backend.get(), images_directory,
                                   groundtruth_file, offset, image_width,
                                   image_height));
      }
      // Adds to flag_list for showing help.
      flag_list.insert(flag_list.end(), dataset_flags.begin(),
                       dataset_flags.end());
    } break;
    case DatasetConfig::COCO: {
      LOG(INFO) << "Using Coco dataset";
      std::string images_directory, groundtruth_file;
      int offset = 1, num_classes = 91, image_width = 320, image_height = 320;
      std::vector<Flag> dataset_flags{
          Flag::CreateFlag("images_directory", &images_directory,
                           "Path to ground truth images.", Flag::kRequired),
          Flag::CreateFlag("offset", &offset,
                           "The offset of the first meaningful class in the "
                           "classification model.",
                           Flag::kRequired),
          Flag::CreateFlag("num_classes", &num_classes,
                           "The number of classes in the model outputs.",
                           Flag::kRequired),
          Flag::CreateFlag("groundtruth_file", &groundtruth_file,
                           "Path to the imagenet ground truth file.",
                           Flag::kRequired),
          Flag::CreateFlag("image_width", &image_width,
                           "The width of the processed image."),
          Flag::CreateFlag("image_height", &image_height,
                           "The height of the processed image.")};
      if (Flags::Parse(&argc, const_cast<const char **>(argv), dataset_flags) &&
          backend) {
        dataset.reset(new Coco(backend.get(), images_directory,
                               groundtruth_file, offset, num_classes,
                               image_width, image_height));
      }
      // Adds to flag_list for showing help.
      flag_list.insert(flag_list.end(), dataset_flags.begin(),
                       dataset_flags.end());
    } break;
    case DatasetConfig::SQUAD: {
      LOG(INFO) << "Using SQuAD 1.1 dataset for MobileBert.";
      std::string input_tfrecord, gt_tfrecord;
      std::vector<Flag> dataset_flags{
          Flag::CreateFlag(
              "input_file", &input_tfrecord,
              "Path to the tfrecord file containing inputs for the model.",
              Flag::kRequired),
          Flag::CreateFlag(
              "groundtruth_file", &gt_tfrecord,
              "Path to the tfrecord file containing ground truth data.",
              Flag::kRequired),
      };

      if (Flags::Parse(&argc, const_cast<const char **>(argv), dataset_flags) &&
          backend) {
        dataset.reset(new Squad(backend.get(), input_tfrecord, gt_tfrecord));
      }
      // Adds to flag_list for showing help.
      flag_list.insert(flag_list.end(), dataset_flags.begin(),
                       dataset_flags.end());
    } break;
    case DatasetConfig::ADE20K: {
      LOG(INFO) << "Using ADE20K dataset";
      std::string images_directory, ground_truth_directory;
      int num_classes = 31;
      int image_width = 512, image_height = 512;
      std::vector<Flag> dataset_flags{
          Flag::CreateFlag("images_directory", &images_directory,
                           "Path to ground truth images.", Flag::kRequired),
          Flag::CreateFlag("ground_truth_directory", &ground_truth_directory,
                           "Path to the imagenet ground truth file.",
                           Flag::kRequired),
          Flag::CreateFlag("num_class", &num_classes, "number of classes"),
          Flag::CreateFlag("image_width", &image_width,
                           "The width of the processed image."),
          Flag::CreateFlag("image_height", &image_height,
                           "The height of the processed image.")};
      if (Flags::Parse(&argc, const_cast<const char **>(argv), dataset_flags) &&
          backend) {
        dataset.reset(new ADE20K(backend.get(), images_directory,
                                 ground_truth_directory, num_classes,
                                 image_width, image_height));
      }
      // Adds to flag_list for showing help.
      flag_list.insert(flag_list.end(), dataset_flags.begin(),
                       dataset_flags.end());
    } break;
    case DatasetConfig::SNUSR: {
      LOG(INFO) << "Using SNU SR dataset";
      std::string images_directory, ground_truth_directory;
      // Number of channels
      int scale = 2;
      int num_channels = 3;
      int image_width = 960;
      int image_height = 540;
      std::vector<Flag> dataset_flags{
          Flag::CreateFlag("images_directory", &images_directory,
                           "Path to ground truth images.", Flag::kRequired),
          Flag::CreateFlag("ground_truth_directory", &ground_truth_directory,
                           "Path to the imagenet ground truth file.",
                           Flag::kRequired),
          Flag::CreateFlag("image_width", &image_width,
                           "The width of the processed image."),
          Flag::CreateFlag("image_height", &image_height,
                           "The height of the processed image."),
          Flag::CreateFlag("n_channels", &num_channels,
                           "The number of color channels."),
          Flag::CreateFlag("scale", &scale, "Super-resolution scale factor")};
      if (Flags::Parse(&argc, const_cast<const char **>(argv), dataset_flags) &&
          backend) {
        dataset.reset(new SNUSR(backend.get(), images_directory,
                                ground_truth_directory, num_channels, scale,
                                image_width, image_height));
      }
      // Adds to flag_list for showing help.
      flag_list.insert(flag_list.end(), dataset_flags.begin(),
                       dataset_flags.end());
    } break;
    case DatasetConfig::NONE:
    default:
      break;
  }

  // Show usage if needed.
  if (!backend || !dataset) {
    LOG(FATAL) << Flags::Usage(command_line, flag_list);
    return 1;
  }

  // Creat out directory if needed.
  struct stat st;
  if (stat(output_dir.c_str(), &st) != 0) {
    LOG(FATAL) << "Output directory " << output_dir << " does not exist";
  }

  if (dataset_type == DatasetConfig::NONE) {
    LOG(FATAL) << "No dataset specified";
    return 1;
  }

  // Running mlperf.
  MlperfDriver driver(std::move(dataset), std::move(backend), scenario,
                      batch_size);
  driver.RunMLPerfTest(mode, min_query_count, min_duration_ms / 1000.0,
                       max_duration_ms / 1000.0,
                       single_stream_expected_latency_ns, output_dir);
  LOG(INFO) << "Accuracy: " << driver.ComputeAccuracyString();
  return 0;
}

}  // namespace mobile
}  // namespace mlperf

int main(int argc, char *argv[]) { return mlperf::mobile::Main(argc, argv); }
