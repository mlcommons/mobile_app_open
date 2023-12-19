import 'package:mlperfbench/backend/bridge/run_result.dart';
import 'package:mlperfbench/backend/bridge/run_settings.dart';
import 'package:mlperfbench/backend/loadgen_info.dart';
import 'package:mlperfbench/data/results/benchmark_result.dart';

class RunInfo {
  final RunSettings settings;
  final NativeRunResult result;
  final LoadgenInfo? loadgenInfo;
  final Throughput? throughput;

  RunInfo({
    required this.settings,
    required this.result,
    required this.loadgenInfo,
    required this.throughput,
  }) {
    if (!(throughput?.value.isFinite ?? true)) {
      throw 'throughput must be a finite number: $throughput';
    }
  }
}
