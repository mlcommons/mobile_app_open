
import 'package:mlperfbench/protos/backend_setting.pb.dart' as pb;
import 'dart:typed_data';

class RunSettings {
  final String backend_model_path;
  final String backend_lib_path;
  final pb.SettingList backend_settings;
  final String backend_native_lib_path;
  final int dataset_type; // 0: Imagenet; 1: Coco; 2: Squad; 3: Ade20k
  final String dataset_data_path;
  final String dataset_groundtruth_path;
  final int dataset_offset;
  final String scenario;
  final String mode; // Submission/Accuracy/Performance
  final int min_query_count;
  final int min_duration;
  final String output_dir;
  final String benchmark_id;

  RunSettings({
    required this.backend_model_path,
    required this.backend_lib_path,
    required this.backend_settings,
    required this.backend_native_lib_path,
    required this.dataset_type, // 0: Imagenet, 1: Coco, 2: Squad, 3: Ade20k
    required this.dataset_data_path,
    required this.dataset_groundtruth_path,
    required this.dataset_offset,
    required this.scenario,
    required this.mode, // Submission/Accuracy/Performance
    required this.min_query_count,
    required this.min_duration,
    required this.output_dir,
    required this.benchmark_id,
  });
}
