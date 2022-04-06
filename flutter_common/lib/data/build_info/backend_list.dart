class BackendList {
  List<String> list;

  BackendList(this.list);

  static BackendList fromJson(List<dynamic> json) {
    final list = <String>[];
    for (var item in json) {
      list.add(item as String);
    }
    return BackendList(list);
  }

  List<dynamic> toJson() {
    var result = <dynamic>[];
    for (var item in list) {
      result.add(item);
    }
    return result;
  }
}
