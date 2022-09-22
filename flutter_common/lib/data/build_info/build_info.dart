import 'package:json_annotation/json_annotation.dart';

part 'build_info.g.dart';

@JsonSerializable(fieldRename: FieldRename.snake)
class BuildInfo {
  final String version;
  final String buildNumber;
  final bool officialReleaseFlag;
  final bool devTestFlag;
  final List<String> backendList;
  final String gitBranch;
  final String gitCommit;
  final bool gitDirtyFlag;

  BuildInfo({
    required this.version,
    required this.buildNumber,
    required this.officialReleaseFlag,
    required this.devTestFlag,
    required this.backendList,
    required this.gitBranch,
    required this.gitCommit,
    required this.gitDirtyFlag,
  });

  factory BuildInfo.fromJson(Map<String, dynamic> json) =>
      _$BuildInfoFromJson(json);

  Map<String, dynamic> toJson() => _$BuildInfoToJson(this);
}
