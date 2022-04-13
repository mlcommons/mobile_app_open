import 'build_info/build_info.dart';
import 'environment/environment_info.dart';
import 'meta_info.dart';
import 'results/benchmark_result.dart';

class ExtendedResult {
  static const String _tagMeta = 'meta';
  static const String _tagResultJson = 'results';
  static const String _tagEnvInfo = 'environment_info';
  static const String _tagBuildInfo = 'build_info';

  final ResultMetaInfo meta;
  final BenchmarkExportResultList results;
  final EnvironmentInfo envInfo;
  final BuildInfo buildInfo;

  ExtendedResult(
      {required this.meta,
      required this.results,
      required this.envInfo,
      required this.buildInfo});

  ExtendedResult.fromJson(Map<String, dynamic> json)
      : this(
            meta:
                ResultMetaInfo.fromJson(json[_tagMeta] as Map<String, dynamic>),
            results: BenchmarkExportResultList.fromJson(
                json[_tagResultJson] as List<dynamic>),
            envInfo: EnvironmentInfo.fromJson(
                json[_tagEnvInfo] as Map<String, dynamic>),
            buildInfo: BuildInfo.fromJson(
                json[_tagBuildInfo] as Map<String, dynamic>));

  Map<String, dynamic> toJson() => {
        _tagMeta: meta,
        _tagResultJson: results,
        _tagEnvInfo: envInfo,
        _tagBuildInfo: buildInfo,
      };
}
