import 'package:json_annotation/json_annotation.dart';

import 'package:mlperfbench_common/constants.dart';
import 'package:mlperfbench_common/data/environment/environment_info.dart';
import 'package:mlperfbench_common/data/extended_result.dart';

part 'result_filter.g.dart';

@JsonSerializable(fieldRename: FieldRename.snake)
class ResultFilter {
  DateTime? fromCreationDate;
  DateTime? toCreationDate;
  EnvPlatform? platform;
  String? deviceModel;
  String? backendName;
  String? manufacturer;
  String? soc;
  String? benchmarkId;

  ResultFilter();

  bool match(ExtendedResult result) {
    String? resultDeviceModel;
    String? resultManufacturer;
    String? resultSoc;
    final envInfo = result.environmentInfo;
    switch (envInfo.platform) {
      case EnvPlatform.android:
        resultDeviceModel = envInfo.value.android?.modelName;
        resultManufacturer = envInfo.value.android?.manufacturer;
        resultSoc = envInfo.value.android?.procCpuinfoSocName;
        break;
      case EnvPlatform.ios:
        resultDeviceModel = envInfo.value.ios?.modelName;
        resultManufacturer = StringValue.apple;
        resultSoc = envInfo.value.ios?.socName;
        break;
      case EnvPlatform.windows:
        resultDeviceModel = envInfo.value.windows?.cpuFullName;
        resultManufacturer = StringValue.unknown;
        resultSoc = envInfo.value.windows?.cpuFullName;
        break;
    }
    resultDeviceModel = resultDeviceModel ?? StringValue.unknown;
    resultManufacturer = resultManufacturer ?? StringValue.unknown;
    resultSoc = resultSoc ?? StringValue.unknown;

    DateTime resultCreationDate = result.meta.creationDate;
    String resultBackendName = result.results.first.backendInfo.backendName;
    EnvPlatform resultPlatform = envInfo.platform;

    bool fromCreationDateMatched = fromCreationDate == null
        ? true
        : resultCreationDate.isAfter(fromCreationDate!);
    bool toCreationDateMatched = toCreationDate == null
        ? true
        : resultCreationDate.isBefore(toCreationDate!);
    bool platformMatched = platform == null ? true : platform == resultPlatform;
    bool deviceModelMatched =
        resultDeviceModel.containsIgnoreCase(deviceModel ?? '');
    bool backendNameMatched =
        resultBackendName.containsIgnoreCase(backendName ?? '');
    bool manufacturerMatched =
        resultManufacturer.containsIgnoreCase(manufacturer ?? '');
    bool socMatched = resultSoc.containsIgnoreCase(soc ?? '');
    bool benchmarkIdMatched = benchmarkId == null
        ? true
        : result.results.any((e) => e.benchmarkId == benchmarkId);

    return fromCreationDateMatched &&
        toCreationDateMatched &&
        platformMatched &&
        deviceModelMatched &&
        backendNameMatched &&
        manufacturerMatched &&
        socMatched &&
        benchmarkIdMatched;
  }

  bool get anyFilterActive {
    return fromCreationDate != null ||
        toCreationDate != null ||
        platform != null ||
        deviceModel != null ||
        backendName != null ||
        manufacturer != null ||
        soc != null;
  }

  factory ResultFilter.fromJson(Map<String, dynamic> json) =>
      _$ResultFilterFromJson(json);

  Map<String, dynamic> toJson() => _$ResultFilterToJson(this);
}

extension StringExtensions on String {
  bool containsIgnoreCase(String secondString) =>
      toLowerCase().contains(secondString.toLowerCase());
}
