import 'package:flutter/foundation.dart' show ChangeNotifier;

import 'package:shared_preferences/shared_preferences.dart';

import 'package:mlperfbench/benchmark/run_mode.dart';

class Store extends ChangeNotifier {
  final SharedPreferences _storeFromDisk;

  Store._(this._storeFromDisk);

  static Future<Store> create() async {
    final store = await SharedPreferences.getInstance();
    return Store._(store);
  }

  bool _getBool(String key, [bool defaultValue = false]) {
    final value = _storeFromDisk.getBool(key);
    return value ?? defaultValue;
  }

  int _getInt(String key, [int defaultValue = 0]) {
    final value = _storeFromDisk.getInt(key);
    return value ?? defaultValue;
  }

  String _getString(String key) {
    final value = _storeFromDisk.getString(key);
    return value ?? '';
  }

  BenchmarkRunModeEnum get selectedBenchmarkRunMode {
    String name = _getString(StoreConstants.selectedBenchmarkRunMode);
    if (name == '') name = BenchmarkRunModeEnum.performanceOnly.name;
    return BenchmarkRunModeEnum.values.byName(name);
  }

  set selectedBenchmarkRunMode(BenchmarkRunModeEnum value) {
    _storeFromDisk.setString(
        StoreConstants.selectedBenchmarkRunMode, value.name);
    cooldownDuration = value.cooldownDuration;
    notifyListeners();
  }

  bool get offlineMode => _getBool(StoreConstants.offlineMode);

  set offlineMode(bool value) {
    _storeFromDisk.setBool(StoreConstants.offlineMode, value);
    notifyListeners();
  }

  bool get artificialCPULoadEnabled =>
      _getBool(StoreConstants.artificialCPULoadEnabled, false);

  set artificialCPULoadEnabled(bool value) {
    _storeFromDisk.setBool(StoreConstants.artificialCPULoadEnabled, value);
    notifyListeners();
  }

  bool get cooldown => _getBool(StoreConstants.cooldown, true);

  set cooldown(bool value) {
    _storeFromDisk.setBool(StoreConstants.cooldown, value);
    notifyListeners();
  }

  int get cooldownDuration => _getInt(StoreConstants.cooldownDuration, 5 * 60);

  set cooldownDuration(int value) {
    _storeFromDisk.setInt(StoreConstants.cooldownDuration, value);
    notifyListeners();
  }

  String get chosenConfigurationName =>
      _getString(StoreConstants.chosenConfigurationName);

  set chosenConfigurationName(String value) {
    _storeFromDisk.setString(StoreConstants.chosenConfigurationName, value);
    notifyListeners();
  }

  String get previousExtendedResult =>
      _getString(StoreConstants.previousExtendedResult);

  set previousExtendedResult(String value) {
    _storeFromDisk.setString(StoreConstants.previousExtendedResult, value);
  }

  String get previousAppVersion =>
      _getString(StoreConstants.previousAppVersion);

  set previousAppVersion(String value) {
    _storeFromDisk.setString(StoreConstants.previousAppVersion, value);
  }

  bool get keepLogs => _getBool(StoreConstants.keepLogs, true);

  set keepLogs(bool value) {
    _storeFromDisk.setBool(StoreConstants.keepLogs, value);
    notifyListeners();
  }

  String get taskSelection => _getString(StoreConstants.taskSelection);

  set taskSelection(String value) {
    _storeFromDisk.setString(StoreConstants.taskSelection, value);
  }

  bool get crashlyticsEnabled =>
      _getBool(StoreConstants.crashlyticsEnabled, false);

  set crashlyticsEnabled(bool value) {
    _storeFromDisk.setBool(StoreConstants.crashlyticsEnabled, value);
    notifyListeners();
  }
}

class StoreConstants {
  static const selectedBenchmarkRunMode = 'selectedBenchmarkRunMode';
  static const artificialCPULoadEnabled = 'artificial cpu load enabled';
  static const offlineMode = 'offline mode';
  static const testMode = 'test mode';
  static const cooldown = 'cooldown';
  static const cooldownDuration = 'cooldown pause';
  static const chosenConfigurationName = 'chosen configuration name';
  static const previousExtendedResult = 'previous extended result';
  static const previousAppVersion = 'previous app version';
  static const keepLogs = 'keep_logs';
  static const taskSelection = 'disabled_tasks';
  static const crashlyticsEnabled = 'crashlyticsEnabled';
}
