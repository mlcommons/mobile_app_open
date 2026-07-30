import 'dart:io' show Platform;

import 'package:flutter/foundation.dart' show visibleForTesting;

import 'package:mlperfbench/backend/bridge/ffi_match.dart';
import 'package:mlperfbench/data/environment/environment_info.dart';
import 'package:mlperfbench/device_info.dart';
import 'package:mlperfbench/protos/backend_setting.pb.dart' as pb;

part 'list.gen.dart';

class BackendInfoHelper {
  static const fallbackBackend = 'libtflitebackend';

  // When true, the TFLite fallback backend is offered alongside any matching
  // vendor backend, so each benchmark can choose among all matching backends
  // and tasks the vendor backend lacks fall back to TFLite.
  // When false, the fallback is probed only when no vendor backend matches,
  // which reproduces the historical single-backend-per-session behavior.
  static const alwaysOfferFallback = true;

  // Returns all matching backends in priority order
  // (the order of _backendsList, vendor backends before the fallback).
  List<BackendInfo> findMatchingBackends({
    bool alwaysOfferFallback = BackendInfoHelper.alwaysOfferFallback,
  }) {
    final matches = <BackendInfo>[];
    // Try to match all backends except the fallback
    for (var name in getBackendsList()) {
      if (name == fallbackBackend) continue;
      print('Checking $name');
      final backendSettings = match(name);
      if (backendSettings != null) {
        matches.add(BackendInfo._(backendSettings, name));
      }
    }

    if (matches.isEmpty || alwaysOfferFallback) {
      print('Checking $fallbackBackend');
      final backendSettings = match(fallbackBackend);
      if (backendSettings != null) {
        matches.add(BackendInfo._(backendSettings, fallbackBackend));
      }
    }
    if (matches.isEmpty) {
      throw 'no matching backend found';
    }
    print('Matching backends: ${matches.map((e) => e.libName).join(', ')}');
    return matches;
  }

  pb.BackendSetting? match(String libName) {
    switch (DeviceInfo.instance.envInfo.platform) {
      case EnvPlatform.android:
        return matchAndroid(libName);
      case EnvPlatform.ios:
        return matchIos(libName);
      case EnvPlatform.windows:
        return matchWindows(libName);
    }
  }

  pb.BackendSetting? matchAndroid(String libName) {
    final info = DeviceInfo.instance.envInfo.value.android!;
    return backendMatch(
      libName: libName,
      manufacturer: info.manufacturer ?? '',
      model: info.modelCode ?? '',
    );
  }

  pb.BackendSetting? matchIos(String libName) {
    final info = DeviceInfo.instance.envInfo.value.ios!;
    return backendMatch(
      libName: libName,
      manufacturer: 'Apple',
      model: info.modelCode ?? '',
    );
  }

  pb.BackendSetting? matchWindows(String libName) {
    return backendMatch(libName: libName, manufacturer: '', model: '');
  }

  List<String> getBackendsList() {
    if (Platform.isWindows || Platform.isAndroid || Platform.isIOS) {
      return _backendsList.where((element) => element != '').toList();
    } else {
      throw 'current platform is unsupported';
    }
  }
}

class BackendInfo {
  final pb.BackendSetting settings;
  final String libName;

  BackendInfo._(this.settings, this.libName);

  @visibleForTesting
  BackendInfo.forTest(this.settings, this.libName);
}
