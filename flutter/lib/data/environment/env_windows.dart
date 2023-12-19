import 'package:json_annotation/json_annotation.dart';

part 'env_windows.g.dart';

@JsonSerializable(fieldRename: FieldRename.snake)
class EnvWindows {
  final String osVersion;
  final String cpuFullName;

  EnvWindows({
    required this.osVersion,
    required this.cpuFullName,
  });

  factory EnvWindows.fromJson(Map<String, dynamic> json) =>
      _$EnvWindowsFromJson(json);

  Map<String, dynamic> toJson() => _$EnvWindowsToJson(this);
}
