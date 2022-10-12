import 'package:json_annotation/json_annotation.dart';

part 'environment_info.g.dart';

enum OsEnum { android, ios, windows }

@JsonSerializable(fieldRename: FieldRename.snake)
class EnvironmentInfo {
  final OsEnum osName;
  final String osVersion;
  final String? manufacturer;
  final String? modelCode;
  final String? modelName;

  EnvironmentInfo({
    required this.osName,
    required this.osVersion,
    required this.manufacturer,
    required this.modelCode,
    required this.modelName,
  });

  factory EnvironmentInfo.fromJson(Map<String, dynamic> json) =>
      _$EnvironmentInfoFromJson(json);

  Map<String, dynamic> toJson() => _$EnvironmentInfoToJson(this);

  static OsEnum parseOs(String name) {
    return _$OsEnumEnumMap.entries
        .firstWhere((element) => element.value == name)
        .key;
  }
}
