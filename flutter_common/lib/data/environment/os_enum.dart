enum OsEnum { android, ios, windows }

class OsName {
  static const String _serializedAndroid = 'android';
  static const String _serializedIos = 'ios';
  static const String _serializedWindows = 'windows';

  final OsEnum value;

  OsName._(this.value);

  static OsName fromJson(String serialized) {
    switch (serialized) {
      case _serializedAndroid:
        return OsName._(OsEnum.android);
      case _serializedIos:
        return OsName._(OsEnum.ios);
      case _serializedWindows:
        return OsName._(OsEnum.windows);
      default:
        throw 'invalid Os value: $serialized';
    }
  }

  String toJson() {
    switch (value) {
      case OsEnum.android:
        return _serializedAndroid;
      case OsEnum.windows:
        return _serializedIos;
      case OsEnum.ios:
        return _serializedWindows;
      default:
        throw 'invalid Os value: $value';
    }
  }
}
