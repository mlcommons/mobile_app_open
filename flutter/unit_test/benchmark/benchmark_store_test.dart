import 'package:flutter_test/flutter_test.dart';

import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/benchmark/run_mode.dart';
import 'package:mlperfbench/protos/backend_setting.pb.dart' as pb;
import 'package:mlperfbench/protos/mlperf_task.pb.dart' as pb;
import 'package:mlperfbench/resources/resource.dart';

void main() {
  group('BenchmarkStore tests', () {
    final task1 = pb.TaskConfig(
      id: 'task1',
      datasets: pb.DatasetConfig(
        full: pb.OneDatasetConfig(
          inputPath: 'full-inputPath',
          groundtruthPath: 'full-gtpath',
        ),
        lite: pb.OneDatasetConfig(inputPath: 'lite-inputPath'),
        tiny: pb.OneDatasetConfig(inputPath: 'tiny-inputPath'),
      ),
    );
    final model1 = pb.ModelFile(
      modelPath: 'model1-path',
    );
    final choice1 = pb.DelegateSetting(
      delegateName: 'delegate1',
      modelFile: [model1],
    );
    final backendSettings1 = pb.BenchmarkSetting(
      benchmarkId: 'task1',
      delegateChoice: [choice1],
    );

    final task2 = pb.TaskConfig(id: 'task2');
    final backendSettings2 = pb.BenchmarkSetting(benchmarkId: 'task2');

    test('match', () async {
      final store = BenchmarkStore(
        appConfig: pb.MLPerfConfig(task: [task1, task2]),
        backendConfig: [backendSettings1],
        taskSelection: {},
      );

      expect(store.benchmarks.length, 1);

      expect(store.benchmarks.first.taskConfig, task1);
      expect(store.benchmarks.first.benchmarkSettings, backendSettings1);
      expect(
        store.benchmarks.first.isActive,
        true,
        reason: 'benchmarks must be enabled by default',
      );
    });

    test('order', () async {
      final store = BenchmarkStore(
        appConfig: pb.MLPerfConfig(task: [task2, task1]),
        backendConfig: [backendSettings1, backendSettings2],
        taskSelection: {},
      );

      expect(store.benchmarks.length, 2);

      expect(store.benchmarks.first.taskConfig, task2);
      expect(store.benchmarks.first.benchmarkSettings, backendSettings2);

      expect(store.benchmarks.last.taskConfig, task1);
      expect(store.benchmarks.last.benchmarkSettings, backendSettings1);
    });

    test('selection', () async {
      final store = BenchmarkStore(
        appConfig: pb.MLPerfConfig(task: [task1, task2]),
        backendConfig: [backendSettings1, backendSettings2],
        taskSelection: {task1.id: true, task2.id: false},
      );

      expect(store.benchmarks.length, 2);
      expect(store.benchmarks.first.isActive, true);
      expect(store.benchmarks.last.isActive, false);
    });

    test('resource list: skip', () async {
      final store = BenchmarkStore(
        appConfig: pb.MLPerfConfig(task: [task1]),
        backendConfig: [backendSettings1],
        taskSelection: {task1.id: false},
      );

      final modes = [BenchmarkRunModeEnum.performanceOnly.performanceRunMode];
      final activeBenchmarks =
          store.benchmarks.where((e) => e.isActive).toList();
      final resources = store.listResources(
        modes: modes,
        benchmarks: activeBenchmarks,
      );

      expect(resources.length, 0);
    });
    test('resource list: accuracy', () async {
      final store = BenchmarkStore(
        appConfig: pb.MLPerfConfig(task: [task1]),
        backendConfig: [backendSettings1],
        taskSelection: {},
      );

      final modes = [BenchmarkRunModeEnum.accuracyOnly.accuracyRunMode];
      final activeBenchmarks =
          store.benchmarks.where((e) => e.isActive).toList();
      final resources = store.listResources(
        modes: modes,
        benchmarks: activeBenchmarks,
      );

      expect(resources.length, 3);
      expect(
          resources,
          contains(Resource(
            type: ResourceTypeEnum.datasetData,
            path: task1.datasets.full.inputPath,
            md5Checksum: task1.datasets.full.inputChecksum,
          )));
      expect(
          resources,
          contains(Resource(
            type: ResourceTypeEnum.datasetGroundtruth,
            path: task1.datasets.full.groundtruthPath,
            md5Checksum: task1.datasets.full.groundtruthChecksum,
          )));
      expect(
          resources,
          contains(Resource(
            path:
                backendSettings1.delegateChoice.first.modelFile.first.modelPath,
            type: ResourceTypeEnum.model,
            md5Checksum: '',
          )));
    });
    test('resource list: performance', () async {
      final store = BenchmarkStore(
        appConfig: pb.MLPerfConfig(task: [task1]),
        backendConfig: [backendSettings1],
        taskSelection: {},
      );

      final modes = [BenchmarkRunModeEnum.performanceOnly.performanceRunMode];
      final activeBenchmarks =
          store.benchmarks.where((e) => e.isActive).toList();
      final resources = store.listResources(
        modes: modes,
        benchmarks: activeBenchmarks,
      );

      expect(resources.length, 2);
      expect(
          resources,
          contains(Resource(
            type: ResourceTypeEnum.datasetData,
            path: task1.datasets.lite.inputPath,
            md5Checksum: task1.datasets.lite.inputChecksum,
          )));
      expect(
          resources,
          contains(Resource(
            path:
                backendSettings1.delegateChoice.first.modelFile.first.modelPath,
            type: ResourceTypeEnum.model,
            md5Checksum: '',
          )));
    });
    test('resource list: test', () async {
      final store = BenchmarkStore(
        appConfig: pb.MLPerfConfig(task: [task1]),
        backendConfig: [backendSettings1],
        taskSelection: {},
      );

      final modes = [
        BenchmarkRunModeEnum.integrationTestRun.accuracyRunMode,
        BenchmarkRunModeEnum.integrationTestRun.performanceRunMode,
      ];
      final activeBenchmarks =
          store.benchmarks.where((e) => e.isActive).toList();
      final resources = store.listResources(
        modes: modes,
        benchmarks: activeBenchmarks,
      );

      expect(resources.length, 2);
      expect(
          resources,
          contains(Resource(
            type: ResourceTypeEnum.datasetData,
            path: task1.datasets.tiny.inputPath,
            md5Checksum: task1.datasets.tiny.inputChecksum,
          )));
      expect(
          resources,
          contains(Resource(
            path:
                backendSettings1.delegateChoice.first.modelFile.first.modelPath,
            type: ResourceTypeEnum.model,
            md5Checksum: '',
          )));
    });
  });
}
