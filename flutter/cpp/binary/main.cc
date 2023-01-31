/* Copyright 2019-2021 The MLPerf Authors. All Rights Reserved.

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

}  // namespace

void report_accuracy(std::string test_results_file, std::string benchmark_id,
                     std::string accuracy) {
  std::ofstream results_file(test_results_file);
  results_file << "#######" << benchmark_id << "##########" << std::endl;
  LOG(INFO) << "Accuracy: " << accuracy;
  results_file << "Accuracy: " << accuracy << std::endl;

  results_file.close();
}

void report_performance(std::string loadgen_summary_file,
                        std::string test_results_file, std::string benchmark_id,
                        bool is_offline) {
  std::ifstream summary_file(loadgen_summary_file);

  std::vector<std::string> keys = {"90.00 percentile latency (ns)",
                                   "Result is"};
  std::vector<std::string> values(keys.size());

  std::string line;
  std::string separator = ":";
  int i;
  while (getline(summary_file, line)) {
    for (i = 0; i < keys.size(); i++) {
      if (line.find(keys[i]) != std::string::npos) {
        values[i] = line.substr(line.find(separator) + 2);
      }
    }
  }

  keys.emplace_back("QPS");
  long long latency = std::stoll(values[0].c_str());
  float final_qps = 0;
  if (latency != 0) {
    final_qps = (1000000000.0 / (float)latency);
    if (is_offline) {
      final_qps = (24576 * 1000000000.0 / (float)latency);
    }
  }
  values.emplace_back(std::to_string(final_qps));

  LOG(INFO) << keys[2] << " : " << values[2];
  std::ofstream results_file(test_results_file);
  results_file << "#######" << benchmark_id << "##########" << std::endl;

  for (i = 0; i < keys.size(); i++) {
    results_file << keys[i] << " : " << values[i] << std::endl;
  }

  summary_file.close();
  results_file.close();
}

int Main(int argc, char *argv[]) {
  using tflite::Flag;
  using tflite::Flags;
  std::string command_line = argv[0];
  // Flags for backend and dataset.
  std::string backend_name, dataset_name;
  std::string benchmark_id;
  BackendType backend_type = BackendType::NONE;
  DatasetConfig::DatasetType dataset_type = DatasetConfig::NONE;
  std::vector<Flag> flag_list{
      Flag::CreateFlag("backend", &backend_name,
                       "Backend. Only TFLite is supported at the moment.",
                       Flag::kPositional),
      Flag::CreateFlag(
          "dataset", &dataset_name,
          "Dataset. One of ade20k, imagenet, coco, squad, or snusr",
          Flag::kPositional)};
  Flags::Parse(&argc, const_cast<const char **>(argv), flag_list);
  backend_type = Str2BackendType(backend_name);
  dataset_type = Str2DatasetType(dataset_name);
  if (backend_type == BackendType::NONE) {
    LOG(FATAL) << Flags::Usage(command_line, flag_list);
    return 1;
  }

  // Treats positional flags as subcommands.
  command_line += " " + backend_name + " " + dataset_name;

  // Command Line Flags for mlperf.
  std::string mode, scenario = "SingleStream", output_dir, results_file;
  int min_query_count = 100, min_duration = 100,
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
       Flag::CreateFlag("min_duration", &min_duration,
                        "The test will guarantee to run at least this "
                        "duration in performance mode. The duration is in ms."),
       Flag::CreateFlag("single_stream_expected_latency_ns",
                        &single_stream_expected_latency_ns,
                        "single_stream_expected_latency_ns"),
       Flag::CreateFlag("output_dir", &output_dir,
                        "The output directory of mlperf.", Flag::kRequired),
       Flag::CreateFlag("results_file", &results_file,
                        "The results summary file in json.", Flag::kRequired)});

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
           Flag::CreateFlag("native_lib_path", &native_lib_path,
                            "Path to the additioal .so files for the backend."),
           Flag::CreateFlag("scenario", &scenario,
                            "Scenario to run the benchmark.")});

      if (Flags::Parse(&argc, const_cast<const char **>(argv), flag_list)) {
        const char *pbdata;
        std::string msg = mlperf::mobile::BackendFunctions::isSupported(
            lib_path, "", model_file_path, &pbdata);
        std::string backend_setting_string(pbdata, strlen(pbdata));
        BackendSetting backend_setting;

        google::protobuf::TextFormat::ParseFromString(pbdata, &backend_setting);

        switch (dataset_type) {
          case DatasetConfig::IMAGENET: {
            if (scenario == "Offline")
              benchmark_id = "image_classification_offline";
            else
              benchmark_id = "image_classification";
          }; break;
          case DatasetConfig::COCO:
            benchmark_id = "object_detection";
            break;
          case DatasetConfig::SQUAD:
            benchmark_id = "natural_language_processing";
            break;
          case DatasetConfig::ADE20K:
            benchmark_id = "image_segmentation_v1";
            break;
          case DatasetConfig::SNUSR:
            benchmark_id = "super_resolution";
            break;
          // Need to check this
          case DatasetConfig::NONE:
          default:
            LOG(INFO) << "how come";
            break;
        }

        SettingList setting_list =
            createSettingList(backend_setting, benchmark_id);

        // If batch size is not specified the default is 0, so set to 1
        batch_size = setting_list.benchmark_setting().batch_size() == 0
                         ? 1
                         : setting_list.benchmark_setting().batch_size();

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
          Flag::CreateFlag("scenario", &scenario,
                           "Scenario to run the benchmark."),
          Flag::CreateFlag("batch_size", &batch_size, "Batch size."),

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
  driver.RunMLPerfTest(mode, min_query_count, min_duration / 1000.0,
                       single_stream_expected_latency_ns, output_dir);

  if (driver.HasAccuracy()) {
    // Report accuracy if computed
    report_accuracy(results_file, benchmark_id, driver.ComputeAccuracyString());
  } else {
    // Report performance
    report_performance(output_dir + "\\mlperf_log_summary.txt", results_file,
                       benchmark_id, scenario == "Offline");
  }
  return 0;
}

}  // namespace mobile
}  // namespace mlperf

int main(int argc, char *argv[]) { return mlperf::mobile::Main(argc, argv); }
