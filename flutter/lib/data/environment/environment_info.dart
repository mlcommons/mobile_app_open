import 'package:json_annotation/json_annotation.dart';

import 'package:mlperfbench/data/environment/env_android.dart';
import 'package:mlperfbench/data/environment/env_ios.dart';
import 'package:mlperfbench/data/environment/env_windows.dart';
import 'package:mlperfbench/ui/formatter.dart';

part 'environment_info.g.dart';

enum EnvPlatform { android, ios, windows }

@JsonSerializable(fieldRename: FieldRename.snake)
class EnvironmentInfo {
  final EnvPlatform platform;
  final EnvInfoValue value;

  EnvironmentInfo({
    required this.platform,
    required this.value,
  });

  EnvironmentInfo.makeAndroid({required EnvAndroid info})
      : platform = EnvPlatform.android,
        value = EnvInfoValue(
          android: info,
          ios: null,
          windows: null,
        );

  EnvironmentInfo.makeIos({required EnvIos info})
      : platform = EnvPlatform.ios,
        value = EnvInfoValue(
          android: null,
          ios: info,
          windows: null,
        );

  EnvironmentInfo.makeWindows({required EnvWindows info})
      : platform = EnvPlatform.windows,
        value = EnvInfoValue(
          android: null,
          ios: null,
          windows: info,
        );

  factory EnvironmentInfo.fromJson(Map<String, dynamic> json) =>
      _$EnvironmentInfoFromJson(json);

  Map<String, dynamic> toJson() => _$EnvironmentInfoToJson(this);

  String get modelDescription {
    switch (platform) {
      case EnvPlatform.android:
        final android = value.android;
        if (android == null) {
          return 'Unknown Android device';
        }
        return '${android.manufacturer} ${android.modelName}'.toUIString();
      case EnvPlatform.ios:
        final ios = value.ios;
        if (ios == null) {
          return 'Unknown iOS device';
        }
        return 'Apple ${ios.modelName}';
      case EnvPlatform.windows:
        return 'PC';
      default:
        return '';
    }
  }
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
