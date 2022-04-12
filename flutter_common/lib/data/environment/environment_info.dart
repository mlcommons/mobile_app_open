import 'os_enum.dart';
import '../results/backend_info.dart';

class EnvironmentInfo {
  static const String _tagOsEnum = 'os_name';
  static const String _tagOsVersion = 'os_version';
  static const String _tagManufacturer = 'manufacturer';
  static const String _tagModel = 'model';

  final OsName osName;
  final String osVersion;
  final String manufacturer;
  final String model;

  EnvironmentInfo(
      {required this.manufacturer,
      required this.model,
      required this.osName,
      required this.osVersion});

  EnvironmentInfo.fromJson(Map<String, dynamic> json)
      : this(
            osName: OsName.fromJson(json[_tagOsEnum] as String),
            osVersion: json[_tagOsVersion] as String,
            manufacturer: json[_tagManufacturer] as String,
            model: json[_tagModel] as String,);

  Map<String, dynamic> toJson() => {
        _tagManufacturer: manufacturer,
        _tagModel: model,
        _tagOsEnum: osName,
        _tagOsVersion: osVersion,
      };
}
