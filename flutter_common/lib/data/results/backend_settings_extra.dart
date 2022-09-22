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

class BackendExtraSettingList {
  final List<BackendExtraSetting> list;

  BackendExtraSettingList(this.list);

  static BackendExtraSettingList fromJson(List<dynamic> json) {
    final list = <BackendExtraSetting>[];
    for (var item in json) {
      list.add(BackendExtraSetting.fromJson(item as Map<String, dynamic>));
    }
    return BackendExtraSettingList(list);
  }

  List<dynamic> toJson() {
    var result = <dynamic>[];
    for (var item in list) {
      result.add(item);
    }
    return result;
  }
}
