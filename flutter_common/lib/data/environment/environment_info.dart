import 'os_enum.dart';

class EnvironmentInfo {
  static const String _tagOsEnum = 'os_name';
  static const String _tagOsVersion = 'os_version';
  static const String _tagManufacturer = 'manufacturer';
  static const String _tagModelCode = 'model_code';
  static const String _tagModelName = 'model_name';
  static const String _tagSocInfo = 'soc_info';

  final OsName osName;
  final String osVersion;
  final String manufacturer;
  final String modelCode;
  final String modelName;
  final EnvSocInfo socInfo;

  EnvironmentInfo({
    required this.osName,
    required this.osVersion,
    required this.manufacturer,
    required this.modelCode,
    required this.modelName,
    required this.socInfo,
  });

  EnvironmentInfo.fromJson(Map<String, dynamic> json)
      : this(
          osName: OsName.fromJson(json[_tagOsEnum] as String),
          osVersion: json[_tagOsVersion] as String,
          manufacturer: json[_tagManufacturer] as String,
          modelCode: json[_tagModelCode] as String,
          modelName: json[_tagModelName] as String,
          socInfo: EnvSocInfo.fromJson(json[_tagSocInfo]),
        );

  Map<String, dynamic> toJson() => {
        _tagManufacturer: manufacturer,
        _tagModelCode: modelCode,
        _tagModelName: modelName,
        _tagOsEnum: osName,
        _tagOsVersion: osVersion,
        _tagSocInfo: socInfo,
      };
}

class EnvSocInfo {
  static const String _tagCpuinfo = 'cpuinfo';
  static const String _tagAndroidInfo = 'android_info';

  final EnvCpuinfo cpuinfo;
  final EnvAndroidInfo? androidInfo;

  EnvSocInfo({
    required this.cpuinfo,
    required this.androidInfo,
  });

  EnvSocInfo.fromJson(Map<String, dynamic> json)
      : this(
          cpuinfo: EnvCpuinfo.fromJson(json[_tagCpuinfo]),
          androidInfo: json[_tagAndroidInfo] == null
              ? null
              : EnvAndroidInfo.fromJson(json[_tagAndroidInfo]),
        );

  Map<String, dynamic> toJson() => {
        _tagCpuinfo: cpuinfo,
        _tagAndroidInfo: androidInfo,
      };
}

class EnvCpuinfo {
  static const String _tagSocName = 'soc_name';
  final String socName;

  EnvCpuinfo({
    required this.socName,
  });

  EnvCpuinfo.fromJson(Map<String, dynamic> json)
      : this(
          socName: json[_tagSocName],
        );

  Map<String, dynamic> toJson() => {
        _tagSocName: socName,
      };
}

class EnvAndroidInfo {
  static const String _tagPropSocModel = 'prop_soc_model';
  static const String _tagPropSocManufacturer = 'prop_soc_manufacturer';
  static const String _tagBuildBoard = 'build_board';

  final String propSocModel;
  final String propSocManufacturer;
  final String buildBoard;

  EnvAndroidInfo({
    required this.propSocModel,
    required this.propSocManufacturer,
    required this.buildBoard,
  });

  EnvAndroidInfo.fromJson(Map<String, dynamic> json)
      : this(
          propSocModel: json[_tagPropSocModel],
          propSocManufacturer: json[_tagPropSocManufacturer],
          buildBoard: json[_tagBuildBoard],
        );

  Map<String, dynamic> toJson() => {
        _tagPropSocModel: propSocModel,
        _tagPropSocManufacturer: propSocManufacturer,
        _tagBuildBoard: buildBoard,
      };
}
