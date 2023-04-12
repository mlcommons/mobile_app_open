import 'dart:io' show Platform;

import 'package:mlperfbench_common/data/environment/environment_info.dart';

import 'package:mlperfbench/backend/bridge/ffi_match.dart';
import 'package:mlperfbench/device_info.dart';
import 'package:mlperfbench/protos/backend_setting.pb.dart' as pb;

part 'list.gen.dart';

class BackendInfoHelper {
  BackendInfo findMatching() {
    for (var name in getBackendsList()) {
      final backendSettings = match(name);
      if (backendSettings != null) {
        final migratedBackendSettings = migrateBackendSetting(backendSettings);
        return BackendInfo._(migratedBackendSettings, name);
      }
    }
    throw 'no matching backend found';
  }

  // Support old benchmark_setting with no delegate_choice.
  // Remove this after deprecated fields are removed from backend_setting.proto
  // ignore_for_file: deprecated_member_use_from_same_package
  pb.BackendSetting migrateBackendSetting(pb.BackendSetting setting) {
    for (final benchmarkSetting in setting.benchmarkSetting) {
      if (benchmarkSetting.delegateChoice.isEmpty) {
        final delegateChoice = pb.DelegateSetting(
          delegateName: '',
          acceleratorName: benchmarkSetting.accelerator,
          acceleratorDesc: benchmarkSetting.acceleratorDesc,
          modelPath: benchmarkSetting.modelPath,
          modelChecksum: benchmarkSetting.modelChecksum,
          batchSize: benchmarkSetting.batchSize,
        );
        benchmarkSetting.delegateChoice.add(delegateChoice);
      }
    }
    return setting;
  }

  pb.BackendSetting? match(String libName) {
    switch (DeviceInfo.instance.envInfo.platform) {
      case EnvPlatform.android:
        return matchAndroid(libName);
      case EnvPlatform.ios:
        return matchIos(libName);
      case EnvPlatform.windows:
        return matchWindows(libName);
      default:
        throw 'unsupported platform';
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
    return backendMatch(
      libName: libName,
      manufacturer: '',
      model: '',
    );
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
}
