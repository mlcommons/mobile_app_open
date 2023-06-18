import 'package:json_annotation/json_annotation.dart';

import 'package:mlperfbench_common/constants.dart';
import 'package:mlperfbench_common/data/environment/environment_info.dart';
import 'package:mlperfbench_common/data/extended_result.dart';

part 'result_filter.g.dart';

@JsonSerializable(fieldRename: FieldRename.snake)
class ResultFilter {
  DateTime? fromCreationDate;
  DateTime? toCreationDate;
  String? platform;
  String? deviceModel;
  String? backend;
  String? manufacturer;
  String? soc;
  String? benchmarkId;
  String? sortBy;

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
    String resultBackend = result.results.first.backendInfo.filename;
    String resultPlatform = envInfo.platform.name;

    bool fromCreationDateMatched = fromCreationDate == null
        ? true
        : resultCreationDate.isAfter(fromCreationDate!);
    // toCreationDate will be at 00:00:00.000 but we want to include that day.
    const oneDay = Duration(days: 1);
    bool toCreationDateMatched = toCreationDate == null
        ? true
        : resultCreationDate.isBefore(toCreationDate!.add(oneDay));
    bool platformMatched = platform == null ? true : platform == resultPlatform;
    bool deviceModelMatched =
        resultDeviceModel.containsIgnoreCase(deviceModel ?? '');
    bool backendMatched = backend == null ? true : backend == resultBackend;
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
        backendMatched &&
        manufacturerMatched &&
        socMatched &&
        benchmarkIdMatched;
  }

  bool get anyFilterActive {
    return fromCreationDate != null ||
        toCreationDate != null ||
        platform != null ||
        deviceModel != null ||
        backend != null ||
        manufacturer != null ||
        soc != null ||
        benchmarkId != null;
  }

  factory ResultFilter.fromJson(Map<String, dynamic> json) =>
      _$ResultFilterFromJson(json);

  Map<String, dynamic> toJson() => _$ResultFilterToJson(this);
}

extension StringExtensions on String {
  bool containsIgnoreCase(String secondString) =>
      toLowerCase().contains(secondString.toLowerCase());
}
