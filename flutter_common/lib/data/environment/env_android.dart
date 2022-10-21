import 'package:json_annotation/json_annotation.dart';

part 'env_android.g.dart';

@JsonSerializable(fieldRename: FieldRename.snake)
class EnvAndroid {
  final String osVersion;
  final String? manufacturer;
  final String? modelCode;
  final String? modelName;
  final String? boardCode;
  final String procCpuinfoSocName;
  final List<EnvAndroidProp> props;

  EnvAndroid({
    required this.osVersion,
    required this.manufacturer,
    required this.modelCode,
    required this.modelName,
    required this.boardCode,
    required this.procCpuinfoSocName,
    required this.props,
  });

  factory EnvAndroid.fromJson(Map<String, dynamic> json) =>
      _$EnvAndroidFromJson(json);

  Map<String, dynamic> toJson() => _$EnvAndroidToJson(this);
}

@JsonSerializable(fieldRename: FieldRename.snake)
class EnvAndroidProp {
  final String type;
  final String name;
  final String value;

  EnvAndroidProp({
    required this.type,
    required this.name,
    required this.value,
  });

  factory EnvAndroidProp.fromJson(Map<String, dynamic> json) =>
      _$EnvAndroidPropFromJson(json);

  Map<String, dynamic> toJson() => _$EnvAndroidPropToJson(this);
}
