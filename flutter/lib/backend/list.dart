import 'dart:io' show Platform;

import 'package:flutter/foundation.dart' show visibleForTesting;

import 'package:mlperfbench/backend/bridge/ffi_match.dart';
import 'package:mlperfbench/data/environment/environment_info.dart';
import 'package:mlperfbench/device_info.dart';
import 'package:mlperfbench/protos/backend_setting.pb.dart' as pb;

part 'list.gen.dart';

class BackendInfoHelper {
  static const fallbackBackend = 'libtflitebackend';

  // Returns all matching backends in priority order (the order of
  // _backendsList, vendor backends before the fallback). Whether a matched
  // backend is actually offered for a given benchmark is decided per
  // benchmark in BenchmarkStore: the priority walk over these matches stops
  // after the first claiming backend whose claim_policy is
  // CLAIM_EXCLUSIVE (the default), so each backend controls whether
  // lower-priority backends stay selectable for benchmarks it claims.
  List<BackendInfo> findMatchingBackends() {
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

    // The fallback is always probed last; per-benchmark visibility is
    // governed by the claim policies of the backends above it.
    print('Checking $fallbackBackend');
    final backendSettings = match(fallbackBackend);
    if (backendSettings != null) {
      matches.add(BackendInfo._(backendSettings, fallbackBackend));
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
