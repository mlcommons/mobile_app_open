import 'package:mlperfbench/data/export_result.dart';
import 'environment_info.dart';

class ExtendedResult {
  static const String _tagUuid = 'uuid';
  static const String _tagResultJson = 'results';
  static const String _tagEnvInfo = 'envInfo';

  final String uuid;
  final ExportResultList results;
  final EnvironmentInfo envInfo;

  ExtendedResult(
      {required this.uuid, required this.results, required this.envInfo});

  ExtendedResult.fromJson(Map<String, dynamic> json)
      : this(
            uuid: json[_tagUuid] as String,
            results: ExportResultList.fromJson(
                json[_tagResultJson] as List<dynamic>),
            envInfo: EnvironmentInfo.fromJson(
                json[_tagEnvInfo] as Map<String, dynamic>));

  Map<String, dynamic> toJson() => {
        _tagUuid: uuid,
        _tagResultJson: results.toJson(),
        _tagEnvInfo: envInfo.toJson(),
      };
}
