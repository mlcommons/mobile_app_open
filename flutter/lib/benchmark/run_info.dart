import 'package:mlperfbench/backend/bridge/run_result.dart';
import 'package:mlperfbench/backend/bridge/run_settings.dart';
import 'package:mlperfbench/benchmark/run_mode.dart';

class RunInfo {
  final RunResult result;
  final RunSettings settings;
  final BenchmarkRunMode runMode;

  RunInfo(this.result, this.settings, this.runMode);
}
