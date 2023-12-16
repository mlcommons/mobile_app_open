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
    final choice1 = pb.DelegateSetting(
      delegateName: 'delegate1',
      modelPath: 'model1-path',
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

      final modes = [BenchmarkRunMode.accuracy];
      final resources = store.listResources(modes: modes, skipInactive: true);

      expect(resources.length, 0);
    });
    test('resource list: accuracy', () async {
      final store = BenchmarkStore(
        appConfig: pb.MLPerfConfig(task: [task1]),
        backendConfig: [backendSettings1],
        taskSelection: {},
      );

      final modes = [BenchmarkRunMode.accuracy];
      final resources = store.listResources(modes: modes, skipInactive: true);

      expect(resources.length, 3);
      expect(
          resources,
          contains(Resource(
            path: task1.datasets.full.inputPath,
            type: ResourceTypeEnum.datasetData,
          )));
      expect(
          resources,
          contains(Resource(
            path: task1.datasets.full.groundtruthPath,
            type: ResourceTypeEnum.datasetGroundtruth,
          )));
      expect(
          resources,
          contains(Resource(
            path: backendSettings1.delegateChoice.first.modelPath,
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

      final modes = [BenchmarkRunMode.performance];
      final resources = store.listResources(modes: modes);

      expect(resources.length, 2);
      expect(
          resources,
          contains(Resource(
            path: task1.datasets.lite.inputPath,
            type: ResourceTypeEnum.datasetData,
          )));
      expect(
          resources,
          contains(Resource(
            path: backendSettings1.delegateChoice.first.modelPath,
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
        BenchmarkRunMode.accuracyTest,
        BenchmarkRunMode.performanceTest,
      ];
      final resources = store.listResources(modes: modes);

      expect(resources.length, 2);
      expect(
          resources,
          contains(Resource(
            path: task1.datasets.tiny.inputPath,
            type: ResourceTypeEnum.datasetData,
          )));
      expect(
          resources,
          contains(Resource(
            path: backendSettings1.delegateChoice.first.modelPath,
            type: ResourceTypeEnum.model,
            md5Checksum: '',
          )));
    });
  });
}
