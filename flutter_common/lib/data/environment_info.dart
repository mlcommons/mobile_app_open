class EnvironmentInfo {
  static const String _tagManufacturer = 'manufacturer';
  static const String _tagModel = 'model';
  static const String _tagOs = 'os';
  static const String _tagOsVersion = 'osVersion';
  static const String _tagAppVersion = 'appVersion';

  final String manufacturer;
  final String model;
  final String os;
  final String osVersion;
  final String appVersion;

  EnvironmentInfo(
      {required this.manufacturer,
      required this.model,
      required this.os,
      required this.osVersion,
      required this.appVersion});

  EnvironmentInfo.fromJson(Map<String, dynamic> json)
      : this(
            manufacturer: json[_tagManufacturer] as String,
            model: json[_tagModel] as String,
            os: json[_tagOs] as String,
            osVersion: json[_tagOsVersion] as String,
            appVersion: json[_tagAppVersion] as String);

  Map<String, dynamic> toJson() => {
        _tagManufacturer: manufacturer,
        _tagModel: model,
        _tagOs: os,
        _tagOsVersion: osVersion,
        _tagAppVersion: appVersion,
      };
}
