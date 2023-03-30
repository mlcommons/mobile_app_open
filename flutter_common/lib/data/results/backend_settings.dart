import 'package:json_annotation/json_annotation.dart';

import 'package:mlperfbench_common/constants.dart';
import 'package:mlperfbench_common/data/results/backend_settings_extra.dart';

part 'backend_settings.g.dart';

@JsonSerializable(fieldRename: FieldRename.snake)
class BackendSettingsInfo {
  final String acceleratorCode;
  final String acceleratorDesc;
  final String framework;
  @JsonKey(defaultValue: StringValue.unknown)
  final String delegate;
  final String modelPath;
  final int batchSize;
  final List<BackendExtraSetting> extraSettings;

  BackendSettingsInfo({
    required this.acceleratorCode,
    required this.acceleratorDesc,
    required this.framework,
    required this.delegate,
    required this.modelPath,
    required this.batchSize,
    required this.extraSettings,
  });

  factory BackendSettingsInfo.fromJson(Map<String, dynamic> json) =>
      _$BackendSettingsInfoFromJson(json);

  Map<String, dynamic> toJson() => _$BackendSettingsInfoToJson(this);
}
