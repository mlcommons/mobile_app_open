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
  final EnvSocInfo socInfo;

  EnvironmentInfo({
    required this.osName,
    required this.osVersion,
    required this.manufacturer,
    required this.modelCode,
    required this.modelName,
    required this.socInfo,
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

@JsonSerializable(fieldRename: FieldRename.snake)
class EnvSocInfo {
  static const String _tagCpuinfo = 'cpuinfo';
  static const String _tagAndroidInfo = 'android_info';

  final EnvCpuinfo cpuinfo;
  final EnvAndroidInfo? androidInfo;

  EnvSocInfo({
    required this.cpuinfo,
    required this.androidInfo,
  });

  factory EnvSocInfo.fromJson(Map<String, dynamic> json) =>
      _$EnvSocInfoFromJson(json);

  Map<String, dynamic> toJson() => _$EnvSocInfoToJson(this);
}

@JsonSerializable(fieldRename: FieldRename.snake)
class EnvCpuinfo {
  static const String _tagSocName = 'soc_name';
  final String socName;

  EnvCpuinfo({
    required this.socName,
  });

  factory EnvCpuinfo.fromJson(Map<String, dynamic> json) =>
      _$EnvCpuinfoFromJson(json);

  Map<String, dynamic> toJson() => _$EnvCpuinfoToJson(this);
}

@JsonSerializable(fieldRename: FieldRename.snake)
class EnvAndroidInfo {
  static const String _tagPropSocModel = 'prop_soc_model';
  static const String _tagPropSocManufacturer = 'prop_soc_manufacturer';
  static const String _tagBuildBoard = 'build_board';

  final String propSocModel;
  final String propSocManufacturer;
  final String? buildBoard;

  EnvAndroidInfo({
    required this.propSocModel,
    required this.propSocManufacturer,
    required this.buildBoard,
  });

  factory EnvAndroidInfo.fromJson(Map<String, dynamic> json) =>
      _$EnvAndroidInfoFromJson(json);

  Map<String, dynamic> toJson() => _$EnvAndroidInfoToJson(this);
}
