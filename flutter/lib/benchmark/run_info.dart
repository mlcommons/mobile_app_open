import 'package:mlperfbench/backend/bridge/run_result.dart';
import 'package:mlperfbench/backend/bridge/run_settings.dart';
import 'package:mlperfbench/backend/loadgen_log_parser.dart';

class RunInfo {
  final RunSettings settings;
  final RunResult result;
  final LoadgenInfo? loadgenInfo;
  final double throughput;

  RunInfo({
    required this.settings,
    required this.result,
    required this.loadgenInfo,
    required this.throughput,
  });
}
