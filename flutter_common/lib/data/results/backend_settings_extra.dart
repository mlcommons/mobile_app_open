import 'package:json_annotation/json_annotation.dart';

part 'backend_settings_extra.g.dart';

@JsonSerializable(fieldRename: FieldRename.snake)
class BackendExtraSetting {
  final String id;
  final String name;
  final String value;
  final String valueName;

  BackendExtraSetting({
    required this.id,
    required this.name,
    required this.value,
    required this.valueName,
  });

  factory BackendExtraSetting.fromJson(Map<String, dynamic> json) =>
      _$BackendExtraSettingFromJson(json);

  Map<String, dynamic> toJson() => _$BackendExtraSettingToJson(this);
}
