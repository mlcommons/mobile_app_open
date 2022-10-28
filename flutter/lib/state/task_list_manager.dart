import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/protos/backend_setting.pb.dart' as pb;
import 'package:mlperfbench/protos/mlperf_task.pb.dart' as pb;

class TaskListManager {
  final pb.BackendSetting backendSettings;

  TaskListManager({
    required this.backendSettings,
  });

  late BenchmarkList taskList;

  void setAppConfig(pb.MLPerfConfig config) {
    taskList = BenchmarkList(
      appConfig: config,
      backendConfig: backendSettings.benchmarkSetting,
    );
  }
}
