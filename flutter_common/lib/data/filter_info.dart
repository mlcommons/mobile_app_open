import 'package:json_annotation/json_annotation.dart';

import 'package:mlperfbench_common/constants.dart';
import 'package:mlperfbench_common/data/build_info/build_info.dart';
import 'package:mlperfbench_common/data/environment/environment_info.dart';
import 'package:mlperfbench_common/data/results/benchmark_result.dart';

part 'filter_info.g.dart';

@JsonSerializable(fieldRename: FieldRename.snake)
class FilterInfo {
  final String platform;
  final String deviceModel;
  final String backendName;
  final String manufacturer;

  FilterInfo({
    required this.platform,
    required this.deviceModel,
    required this.backendName,
    required this.manufacturer,
  });

  static FilterInfo create(EnvironmentInfo envInfo, BuildInfo buildInfo,
      BenchmarkExportResult result) {
    String? deviceModel;
    String? manufacturer;
    switch (envInfo.platform) {
      case EnvPlatform.android:
        deviceModel = envInfo.value.android?.modelName;
        manufacturer = envInfo.value.android?.manufacturer;
        break;
      case EnvPlatform.ios:
        deviceModel = envInfo.value.ios?.modelName;
        manufacturer = StringValue.apple;
        break;
      case EnvPlatform.windows:
        deviceModel = envInfo.value.windows?.cpuFullName;
        manufacturer = StringValue.unknown;
        break;
    }
    return FilterInfo(
      backendName: result.backendInfo.backendName,
      platform: envInfo.platform.name,
      deviceModel: deviceModel ?? StringValue.unknown,
      manufacturer: manufacturer ?? StringValue.unknown,
    );
  }

  factory FilterInfo.fromJson(Map<String, dynamic> json) =>
      _$FilterInfoFromJson(json);

  Map<String, dynamic> toJson() => _$FilterInfoToJson(this);
}
