import 'backend_list.dart';

class BuildInfo {
  static const String _tagVersion = 'version';
  static const String _tagBuildNumber = 'build_number';
  static const String _tagOfficialReleaseFlag = 'official_release_flag';
  static const String _tagDevTestFlag = 'dev_test_flag';
  static const String _tagBackendList = 'backend_list';
  static const String _tagGitBranch = 'git_branch';
  static const String _tagGitCommit = 'git_commit';
  static const String _tagGitDirtyFlag = 'git_dirty_flag';

  final String version;
  final String buildNumber;
  final bool officialReleaseFlag;
  final bool devTestFlag;
  final BackendList backends;
  final String gitBranch;
  final String gitCommit;
  final bool gitDirtyFlag;

  BuildInfo({
    required this.version,
    required this.buildNumber,
    required this.officialReleaseFlag,
    required this.devTestFlag,
    required this.backends,
    required this.gitBranch,
    required this.gitCommit,
    required this.gitDirtyFlag,
  });

  BuildInfo.fromJson(Map<String, dynamic> json)
      : this(
          version: json[_tagVersion] as String,
          buildNumber: json[_tagBuildNumber] as String,
          officialReleaseFlag: json[_tagOfficialReleaseFlag] as bool,
          devTestFlag: json[_tagDevTestFlag] as bool,
          backends:
              BackendList.fromJson(json[_tagBackendList] as List<dynamic>),
          gitBranch: json[_tagGitBranch] as String,
          gitCommit: json[_tagGitCommit] as String,
          gitDirtyFlag: json[_tagGitDirtyFlag] as bool,
        );

  Map<String, dynamic> toJson() => {
        _tagVersion: version,
        _tagBuildNumber: buildNumber,
        _tagOfficialReleaseFlag: officialReleaseFlag,
        _tagDevTestFlag: devTestFlag,
        _tagBackendList: backends,
        _tagGitBranch: gitBranch,
        _tagGitCommit: gitCommit,
        _tagGitDirtyFlag: gitDirtyFlag,
      };
}
