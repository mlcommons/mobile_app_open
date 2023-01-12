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

  bool get share => _getBool(StoreConstants.share);

  set share(bool value) {
    _storeFromDisk.setBool(StoreConstants.share, value);
    notifyListeners();
  }

  BenchmarkRunModeEnum get selectedBenchmarkRunMode {
    String name = _getString(StoreConstants.selectedBenchmarkRunMode);
    if (name == '') name = BenchmarkRunModeEnum.performanceOnly.name;
    return BenchmarkRunModeEnum.values.byName(name);
  }

  set selectedBenchmarkRunMode(BenchmarkRunModeEnum value) {
    _storeFromDisk.setString(
        StoreConstants.selectedBenchmarkRunMode, value.name);
    notifyListeners();
  }

  bool get submissionMode => _getBool(StoreConstants.submissionMode);

  set submissionMode(bool value) {
    _storeFromDisk.setBool(StoreConstants.submissionMode, value);
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

  bool get testMode => _getBool(StoreConstants.testMode);

  set testMode(bool value) {
    _storeFromDisk.setBool(StoreConstants.testMode, value);
    notifyListeners();
  }

  bool get cooldown => _getBool(StoreConstants.cooldown, true);

  set cooldown(bool value) {
    _storeFromDisk.setBool(StoreConstants.cooldown, value);
    notifyListeners();
  }

  int get cooldownDuration => _getInt(StoreConstants.cooldownDuration, 5);

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

  bool isShareOptionChosen() =>
      _storeFromDisk.containsKey(StoreConstants.share);

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

  String get dataFolderType => _getString(StoreConstants.dataFolderType);

  set dataFolderType(String value) {
    _storeFromDisk.setString(StoreConstants.dataFolderType, value);
    notifyListeners();
  }

  String get customDataFolder => _getString(StoreConstants.customDataFolder);

  set customDataFolder(String value) {
    _storeFromDisk.setString(StoreConstants.customDataFolder, value);
    notifyListeners();
  }

  String get taskSelection => _getString(StoreConstants.taskSelection);

  set taskSelection(String value) {
    _storeFromDisk.setString(StoreConstants.taskSelection, value);
  }

  int get testMinDuration => _getInt(StoreConstants.testMinDuration);

  int get testCooldown => _getInt(StoreConstants.testCooldownDuration);

  int get testMinQueryCount => _getInt(StoreConstants.testMinQueryCount);
}

class StoreConstants {
  static const share = 'share';
  static const selectedBenchmarkRunMode = 'selectedBenchmarkRunMode';
  static const submissionMode = 'submission mode';
  static const artificialCPULoadEnabled = 'artificial cpu load enabled';
  static const offlineMode = 'offline mode';
  static const testMode = 'test mode';
  static const cooldown = 'cooldown';
  static const cooldownDuration = 'cooldown pause';
  static const chosenConfigurationName = 'chosen configuration name';
  static const previousExtendedResult = 'previous extended result';
  static const previousAppVersion = 'previous app version';
  static const keepLogs = 'keep_logs';
  static const dataFolderType = 'data folder type';
  static const customDataFolder = 'custom data folder';
  static const taskSelection = 'disabled_tasks';
  static const testMinDuration = 'test min duration';
  static const testCooldownDuration = 'test cooldown duration';
  static const testMinQueryCount = 'test min query count';
}
