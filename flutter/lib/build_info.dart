import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench_common/data/build_info/backend_list.dart';
import 'package:mlperfbench_common/data/build_info/build_info.dart';
import 'package:package_info_plus/package_info_plus.dart';

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
    officialReleaseFlag: OFFICIAL_BUILD,
     devTestFlag: FAST_MODE, 
    backends: BackendList([]),
     gitBranch: '', 
    gitCommit: '', 
    gitDirtyFlag: true);
  }
}
