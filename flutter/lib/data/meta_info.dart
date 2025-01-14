import 'package:json_annotation/json_annotation.dart';
import 'package:mlperfbench/benchmark/run_mode.dart';

part 'meta_info.g.dart';

@JsonSerializable(fieldRename: FieldRename.snake)
class ResultMetaInfo {
  final String uuid;
  final DateTime creationDate;
  final DateTime? uploadDate;
  final BenchmarkRunModeEnum? runMode;

  ResultMetaInfo({
    required this.uuid,
    required this.creationDate,
    required this.runMode,
    DateTime? uploadDate,
  }) : uploadDate = uploadDate?.toUtc();

  factory ResultMetaInfo.fromJson(Map<String, dynamic> json) =>
      _$ResultMetaInfoFromJson(json);

  Map<String, dynamic> toJson() => _$ResultMetaInfoToJson(this);
}
