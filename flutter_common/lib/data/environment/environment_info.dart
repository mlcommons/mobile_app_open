import 'package:json_annotation/json_annotation.dart';

import 'package:mlperfbench_common/data/environment/env_android.dart';
import 'package:mlperfbench_common/data/environment/env_ios.dart';
import 'package:mlperfbench_common/data/environment/env_windows.dart';

part 'environment_info.g.dart';

enum EnvDeviceType { android, ios, windows }

@JsonSerializable(fieldRename: FieldRename.snake)
class EnvironmentInfo {
  final EnvDeviceType deviceType;
  final EnvInfoValue value;

  EnvironmentInfo({
    required this.deviceType,
    required this.value,
  });

  EnvironmentInfo.makeAndroid({required EnvAndroid info})
      : deviceType = EnvDeviceType.android,
        value = EnvInfoValue(
          android: info,
          ios: null,
          windows: null,
        );

  EnvironmentInfo.makeIos({required EnvIos info})
      : deviceType = EnvDeviceType.ios,
        value = EnvInfoValue(
          android: null,
          ios: info,
          windows: null,
        );

  EnvironmentInfo.makeWIndows({required EnvWindows info})
      : deviceType = EnvDeviceType.windows,
        value = EnvInfoValue(
          android: null,
          ios: null,
          windows: info,
        );

  factory EnvironmentInfo.fromJson(Map<String, dynamic> json) =>
      _$EnvironmentInfoFromJson(json);

  Map<String, dynamic> toJson() => _$EnvironmentInfoToJson(this);
}

@JsonSerializable(fieldRename: FieldRename.snake)
class EnvInfoValue {
  final EnvAndroid? android;
  final EnvIos? ios;
  final EnvWindows? windows;

  EnvInfoValue({
    required this.android,
    required this.ios,
    required this.windows,
  });

  factory EnvInfoValue.fromJson(Map<String, dynamic> json) =>
      _$EnvInfoValueFromJson(json);

  Map<String, dynamic> toJson() => _$EnvInfoValueToJson(this);
}
