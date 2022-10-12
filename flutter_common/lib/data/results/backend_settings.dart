import 'package:json_annotation/json_annotation.dart';

import 'backend_settings_extra.dart';

part 'backend_settings.g.dart';

@JsonSerializable(fieldRename: FieldRename.snake)
class BackendSettingsInfo {
  final String acceleratorCode;
  final String acceleratorDesc;
  final String configuration;
  final String modelPath;
  final int batchSize;
  final List<BackendExtraSetting> extraSettings;

  BackendSettingsInfo({
    required this.acceleratorCode,
    required this.acceleratorDesc,
    required this.configuration,
    required this.modelPath,
    required this.batchSize,
    required this.extraSettings,
  });

  factory BackendSettingsInfo.fromJson(Map<String, dynamic> json) =>
      _$BackendSettingsInfoFromJson(json);

  Map<String, dynamic> toJson() => _$BackendSettingsInfoToJson(this);
}
