import 'dart:typed_data';

import 'package:mlperfbench/benchmark/benchmark.dart';

class RunSettings {
  final String backend_description;
  final String backend_model_path;
  final String backend_lib_path;
  final Uint8List backend_settings;
  final String backend_native_lib_path;
  final int dataset_type; // 0: Imagenet; 1: Coco; 2: Squad; 3: Ade20k
  final String dataset_data_path;
  final String dataset_groundtruth_path;
  final int dataset_offset;
  final String scenario;
  final int batch;
  final int batch_size;
  final int threads_number;
  final String mode; // Submission/Accuracy/Performance
  final int min_query_count;
  final int min_duration;
  final String output_dir;
  final String benchmark_id;
  final DatasetMode dataset_mode;

  RunSettings({
    required this.backend_description,
    required this.backend_model_path,
    required this.backend_lib_path,
    required this.backend_settings,
    required this.backend_native_lib_path,
    required this.dataset_type, // 0: Imagenet, 1: Coco, 2: Squad, 3: Ade20k
    required this.dataset_data_path,
    required this.dataset_groundtruth_path,
    required this.dataset_offset,
    required this.scenario,
    required this.batch,
    required this.batch_size,
    required this.threads_number,
    required this.mode, // Submission/Accuracy/Performance
    required this.min_query_count,
    required this.min_duration,
    required this.output_dir,
    required this.benchmark_id,
    required this.dataset_mode,
  });
}
