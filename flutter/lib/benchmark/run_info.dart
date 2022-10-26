import 'package:mlperfbench/backend/bridge/run_result.dart';
import 'package:mlperfbench/backend/bridge/run_settings.dart';
import 'package:mlperfbench/backend/loadgen_info.dart';

class RunInfo {
  final RunSettings settings;
  final NativeRunResult result;
  final LoadgenInfo? loadgenInfo;
  final double? throughput;

  RunInfo({
    required this.settings,
    required this.result,
    required this.loadgenInfo,
    required this.throughput,
  }) {
    if (!(throughput?.isFinite ?? true)) {
      throw 'throughput must be a finite number: $throughput';
    }
  }
}
