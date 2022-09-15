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

  bool get share => _getBool(_StoreConstants.share);

  set share(bool shareFlag) {
    _storeFromDisk.setBool(_StoreConstants.share, shareFlag);
    notifyListeners();
  }

  bool get submissionMode => _getBool(_StoreConstants.submissionMode);

  set submissionMode(bool submissionModeFlag) {
    _storeFromDisk.setBool(_StoreConstants.submissionMode, submissionModeFlag);
    notifyListeners();
  }

  bool get offlineMode => _getBool(_StoreConstants.offlineMode);

  set offlineMode(bool offlineModeFlag) {
    _storeFromDisk.setBool(_StoreConstants.offlineMode, offlineModeFlag);
    notifyListeners();
  }

  bool get testMode => _getBool(_StoreConstants.testMode);

  set testMode(bool testModeFlag) {
    _storeFromDisk.setBool(_StoreConstants.testMode, testModeFlag);
    notifyListeners();
  }

  bool get cooldown => _getBool(_StoreConstants.cooldown, true);

  set cooldown(bool submissionModeFlag) {
    _storeFromDisk.setBool(_StoreConstants.cooldown, submissionModeFlag);
    notifyListeners();
  }

  int get cooldownDuration => _getInt(_StoreConstants.cooldownDuration, 5);

  set cooldownDuration(int value) {
    _storeFromDisk.setInt(_StoreConstants.cooldownDuration, value);
    notifyListeners();
  }

  String get chosenConfigurationName =>
      _getString(_StoreConstants.chosenConfigurationName);

  set chosenConfigurationName(String configurationName) {
    _storeFromDisk.setString(
        _StoreConstants.chosenConfigurationName, configurationName);
    notifyListeners();
  }

  String get previousExtendedResult =>
      _getString(_StoreConstants.previousExtendedResult);

  set previousExtendedResult(String result) {
    _storeFromDisk.setString(_StoreConstants.previousExtendedResult, result);
  }

  bool isShareOptionChosen() =>
      _storeFromDisk.containsKey(_StoreConstants.share);

  String get previousAppVersion =>
      _getString(_StoreConstants.previousAppVersion);

  set previousAppVersion(String value) {
    _storeFromDisk.setString(_StoreConstants.previousAppVersion, value);
  }

  bool get keepLogs => _getBool(_StoreConstants.keepLogs, true);

  set keepLogs(bool value) {
    _storeFromDisk.setBool(_StoreConstants.keepLogs, value);
    notifyListeners();
  }

  String get taskSelection => _getString(_StoreConstants.taskSelection);

  set taskSelection(String value) {
    _storeFromDisk.setString(_StoreConstants.taskSelection, value);
  }
}

class _StoreConstants {
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
}
