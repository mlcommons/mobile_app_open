class BackendExtraSetting {
  static const String _tagId = 'id';
  static const String _tagName = 'name';
  static const String _tagValue = 'value';
  static const String _tagValueName = 'value_name';

  final String id;
  final String name;
  final String value;
  final String valueName;

  BackendExtraSetting(
      {required this.id,
      required this.name,
      required this.value,
      required this.valueName});

  BackendExtraSetting.fromJson(Map<String, dynamic> json)
      : this(
            id: json[_tagId] as String,
            name: json[_tagName] as String,
            value: json[_tagValue] as String,
            valueName: json[_tagValueName] as String);

  Map<String, dynamic> toJson() => {
        _tagId: id,
        _tagName: name,
        _tagValue: value,
        _tagValueName: valueName,
      };
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
