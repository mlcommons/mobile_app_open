import 'package:json_annotation/json_annotation.dart';

part 'env_ios.g.dart';

@JsonSerializable(fieldRename: FieldRename.snake)
class EnvIos {
  final String osVersion;
  final String? modelCode;
  final String? modelName;
  final String socName;

  EnvIos({
    required this.osVersion,
    required this.modelCode,
    required this.modelName,
    required this.socName,
  });

  factory EnvIos.fromJson(Map<String, dynamic> json) => _$EnvIosFromJson(json);

  Map<String, dynamic> toJson() => _$EnvIosToJson(this);
}
