import 'package:mlperfbench/backend/bridge/run_result.dart';
import 'package:mlperfbench/backend/bridge/run_settings.dart';
import 'benchmark.dart';

class RunInfo {
  final RunSettings settings;
  final RunResult result;

  RunInfo({
    required this.settings,
    required this.result,
  });
}
