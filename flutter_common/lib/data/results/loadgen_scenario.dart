enum LoadgenScenarioEnum { singleStream, offline }

class LoadgenScenario {
  static const String _serializedSingleStream = 'SingleStream';
  static const String _serializedOffline = 'Offline';

  final LoadgenScenarioEnum value;

  LoadgenScenario._(this.value);

  static LoadgenScenario fromJson(String serialized) {
    switch (serialized) {
      case _serializedSingleStream:
        return LoadgenScenario._(LoadgenScenarioEnum.singleStream);
      case _serializedOffline:
        return LoadgenScenario._(LoadgenScenarioEnum.offline);
      default:
        throw 'invalid LoadgenScenario value: $serialized';
    }
  }

  String toJson() {
    switch (value) {
      case LoadgenScenarioEnum.singleStream:
        return _serializedSingleStream;
      case LoadgenScenarioEnum.offline:
        return _serializedOffline;
      default:
        throw 'invalid LoadgenScenario value: $value';
    }
  }
}
