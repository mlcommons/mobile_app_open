import 'package:flutter/foundation.dart' show ChangeNotifier;

import 'package:shared_preferences/shared_preferences.dart';

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

  set share(bool shareFlag) {
    _storeFromDisk.setBool(StoreConstants.share, shareFlag);
    notifyListeners();
  }

  bool get submissionMode => _getBool(StoreConstants.submissionMode);

  set submissionMode(bool submissionModeFlag) {
    _storeFromDisk.setBool(StoreConstants.submissionMode, submissionModeFlag);
    notifyListeners();
  }

  bool get offlineMode => _getBool(StoreConstants.offlineMode);

  set offlineMode(bool offlineModeFlag) {
    _storeFromDisk.setBool(StoreConstants.offlineMode, offlineModeFlag);
    notifyListeners();
  }

  bool get testMode => _getBool(StoreConstants.testMode);

  set testMode(bool testModeFlag) {
    _storeFromDisk.setBool(StoreConstants.testMode, testModeFlag);
    notifyListeners();
  }

  bool get cooldown => _getBool(StoreConstants.cooldown, true);

  set cooldown(bool submissionModeFlag) {
    _storeFromDisk.setBool(StoreConstants.cooldown, submissionModeFlag);
    notifyListeners();
  }

  int get cooldownDuration => _getInt(StoreConstants.cooldownDuration, 5);

  set cooldownDuration(int value) {
    _storeFromDisk.setInt(StoreConstants.cooldownDuration, value);
    notifyListeners();
  }

  String get chosenConfigurationName =>
      _getString(StoreConstants.chosenConfigurationName);

  set chosenConfigurationName(String configurationName) {
    _storeFromDisk.setString(
        StoreConstants.chosenConfigurationName, configurationName);
    notifyListeners();
  }

  String get previousExtendedResult =>
      _getString(StoreConstants.previousExtendedResult);

  set previousExtendedResult(String result) {
    _storeFromDisk.setString(StoreConstants.previousExtendedResult, result);
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

  String get taskSelection => _getString(StoreConstants.taskSelection);

  set taskSelection(String value) {
    _storeFromDisk.setString(StoreConstants.taskSelection, value);
  }

  int get testMinDuration => _getInt(StoreConstants.testMinDuration);
  int get testCooldown => _getInt(StoreConstants.testCooldown);
  int get testMinQueryCount => _getInt(StoreConstants.testMinQueryCount);
}

class StoreConstants {
  static const share = 'share';
  static const submissionMode = 'submission mode';
  static const offlineMode = 'offline mode';
  static const testMode = 'test mode';
  static const cooldown = 'cooldown';
  static const cooldownDuration = 'cooldown pause';
  static const chosenConfigurationName = 'chosen configuration name';
  static const previousExtendedResult = 'previous extended result';
  static const previousAppVersion = 'previous app version';
  static const keepLogs = 'keep_logs';
  static const taskSelection = 'disabled_tasks';
  static const testMinDuration = 'test min duration';
  static const testCooldown = 'test cooldown';
  static const testMinQueryCount = 'test min query count';
}
