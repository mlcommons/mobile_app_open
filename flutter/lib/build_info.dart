import 'package:package_info_plus/package_info_plus.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/backend/list.dart';
import 'package:mlperfbench/data/build_info/build_info.dart';

part 'build_info.gen.dart';

class BuildInfoHelper {
  static late final BuildInfo info;

  static Future<void> staticInit() async {
    final packageInfo = await PackageInfo.fromPlatform();
    print('packageInfo.appName = ${packageInfo.appName}');
    print('packageInfo.buildNumber = ${packageInfo.buildNumber}');
    print('packageInfo.buildSignature = ${packageInfo.buildSignature}');
    print('packageInfo.packageName = ${packageInfo.packageName}');
    print('packageInfo.version = ${packageInfo.version}');

    info = BuildInfo(
        version: packageInfo.version,
        buildNumber: packageInfo.buildNumber,
        buildDate: DateTime.now(),
        officialReleaseFlag: DartDefine.isOfficialBuild,
        devTestFlag: DartDefine.perfTestEnabled,
        backendList: BackendInfoHelper().getBackendsList(),
        gitBranch: GeneratedBuildInfo.gitBranch,
        gitCommit: GeneratedBuildInfo.gitCommit,
        gitDirtyFlag: GeneratedBuildInfo.gitDirty != 0);
  }
}
