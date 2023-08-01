import 'package:json_annotation/json_annotation.dart';

import 'package:mlperfbench_common/data/build_info/build_info.dart';
import 'package:mlperfbench_common/data/environment/environment_info.dart';
import 'package:mlperfbench_common/data/meta_info.dart';
import 'package:mlperfbench_common/data/results/benchmark_result.dart';

part 'extended_result.g.dart';

@JsonSerializable(fieldRename: FieldRename.snake)
class ExtendedResult {
  final ResultMetaInfo meta;
  final List<BenchmarkExportResult> results;
  final EnvironmentInfo environmentInfo;
  final BuildInfo buildInfo;

  ExtendedResult({
    required this.meta,
    required this.results,
    required this.environmentInfo,
    required this.buildInfo,
  });

  factory ExtendedResult.fromJson(Map<String, dynamic> json) =>
      _$ExtendedResultFromJson(json);

  Map<String, dynamic> toJson() => _$ExtendedResultToJson(this);
}
