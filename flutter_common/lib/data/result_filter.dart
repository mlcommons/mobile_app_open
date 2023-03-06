import 'package:json_annotation/json_annotation.dart';

import 'package:mlperfbench_common/constants.dart';
import 'package:mlperfbench_common/data/environment/environment_info.dart';
import 'package:mlperfbench_common/data/extended_result.dart';

part 'result_filter.g.dart';

@JsonSerializable(fieldRename: FieldRename.snake)
class ResultFilter {
  DateTime? creationDate;
  EnvPlatform? platform;
  String? deviceModel;
  String? backendName;
  String? manufacturer;
  String? soc;

  ResultFilter();

  static ResultFilter from(ExtendedResult result) {
    String? deviceModel;
    String? manufacturer;
    String? soc;
    final envInfo = result.environmentInfo;
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
    final filter = ResultFilter()
      ..creationDate = result.meta.creationDate
      ..backendName = result.results.first.backendInfo.backendName
      ..platform = envInfo.platform
      ..deviceModel = deviceModel ?? StringValue.unknown
      ..manufacturer = manufacturer ?? StringValue.unknown
      ..soc = soc ?? StringValue.unknown;
    return filter;
  }

  // other should be an instance of ResultManager.resultFilter
  bool match(ResultFilter other) {
    bool platformMatched =
        other.platform == null ? true : platform == other.platform;
    bool deviceModelMatched =
        deviceModel?.containsIgnoreCase(other.deviceModel ?? '') ?? false;
    bool backendNameMatched =
        backendName?.containsIgnoreCase(other.backendName ?? '') ?? false;
    bool manufacturerMatched =
        manufacturer?.containsIgnoreCase(other.manufacturer ?? '') ?? false;
    bool socMatched = soc?.containsIgnoreCase(other.soc ?? '') ?? false;
    return platformMatched &&
        deviceModelMatched &&
        backendNameMatched &&
        manufacturerMatched &&
        socMatched;
  }

  bool get anyFilterActive {
    return creationDate != null ||
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
