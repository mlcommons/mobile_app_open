import 'package:mlperfbench_common/constants.dart';
import 'package:mlperfbench_common/data/build_info/build_info.dart';
import 'package:mlperfbench_common/data/environment/environment_info.dart';
import 'package:mlperfbench_common/data/results/benchmark_result.dart';

class FilterInfo {
  final DateTime creationDate;
  final String platform;
  final String deviceModel;
  final String backendName;
  final String manufacturer;
  final String soc;

  FilterInfo({
    required this.creationDate,
    required this.platform,
    required this.deviceModel,
    required this.backendName,
    required this.manufacturer,
    required this.soc,
  });

  static FilterInfo create(DateTime creationDate, EnvironmentInfo envInfo,
      BuildInfo buildInfo, BenchmarkExportResult result) {
    String? deviceModel;
    String? manufacturer;
    String? soc;
    switch (envInfo.platform) {
      case EnvPlatform.android:
        deviceModel = envInfo.value.android?.modelName;
        manufacturer = envInfo.value.android?.manufacturer;
        soc = envInfo.value.android?.procCpuinfoSocName;
        break;
      case EnvPlatform.ios:
        deviceModel = envInfo.value.ios?.modelName;
        manufacturer = StringValue.apple;
        soc = envInfo.value.ios?.socName;
        break;
      case EnvPlatform.windows:
        deviceModel = envInfo.value.windows?.cpuFullName;
        manufacturer = StringValue.unknown;
        soc = envInfo.value.windows?.cpuFullName;
        break;
    }
    return FilterInfo(
      creationDate: creationDate,
      backendName: result.backendInfo.backendName,
      platform: envInfo.platform.name,
      deviceModel: deviceModel ?? StringValue.unknown,
      manufacturer: manufacturer ?? StringValue.unknown,
      soc: soc ?? StringValue.unknown,
    );
  }
}
