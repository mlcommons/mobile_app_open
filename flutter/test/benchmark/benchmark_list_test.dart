import 'package:flutter_test/flutter_test.dart';

import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/benchmark/run_mode.dart';
import 'package:mlperfbench/protos/backend_setting.pb.dart' as pb;
import 'package:mlperfbench/protos/mlperf_task.pb.dart' as pb;
import 'package:mlperfbench/resources/resource.dart';

void main() {
  group('BenchmarkList tests', () {
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
    final backendSettings1 = pb.BenchmarkSetting(
      benchmarkId: 'task1',
      modelPath: 'model1-path',
    );

    final task2 = pb.TaskConfig(id: 'task2');
    final backendSettings2 = pb.BenchmarkSetting(benchmarkId: 'task2');

    test('match', () async {
      final list = BenchmarkList(
        appConfig: pb.MLPerfConfig(task: [task1, task2]),
        backendConfig: [backendSettings1],
      );

      expect(list.benchmarks.length, 1);

      expect(list.benchmarks.first.taskConfig, task1);
      expect(list.benchmarks.first.benchmarkSettings, backendSettings1);
      expect(
        list.benchmarks.first.isActive,
        true,
        reason: 'benchmarks must be enabled by default',
      );
    });

    test('order', () async {
      final list = BenchmarkList(
        appConfig: pb.MLPerfConfig(task: [task2, task1]),
        backendConfig: [backendSettings1, backendSettings2],
      );

      expect(list.benchmarks.length, 2);

      expect(list.benchmarks.first.taskConfig, task2);
      expect(list.benchmarks.first.benchmarkSettings, backendSettings2);

      expect(list.benchmarks.last.taskConfig, task1);
      expect(list.benchmarks.last.benchmarkSettings, backendSettings1);
    });

    test('selection: get', () async {
      final list = BenchmarkList(
        appConfig: pb.MLPerfConfig(task: [task1, task2]),
        backendConfig: [backendSettings1, backendSettings2],
      );
      list.benchmarks[0].isActive = false;
      list.benchmarks[1].isActive = true;
      final selection = list.getTaskSelection();

      expect(selection.length, 2);
      expect(selection[task1.id], false);
      expect(selection[task2.id], true);
    });

    test('selection: restore', () async {
      final list = BenchmarkList(
        appConfig: pb.MLPerfConfig(task: [task1, task2]),
        backendConfig: [backendSettings1, backendSettings2],
      );
      list.restoreSelection({task1.id: true, task2.id: false});

      expect(list.benchmarks.length, 2);
      expect(list.benchmarks.first.isActive, true);
      expect(list.benchmarks.last.isActive, false);
    });

    test('selection serialize-deserialize', () async {
      final selection = {task1.id: true, task2.id: false};
      final serialized = BenchmarkList.serializeTaskSelection(selection);
      final deserialized = BenchmarkList.deserializeTaskSelection(serialized);

      expect(deserialized, selection);
    });

    test('resource list: skip', () async {
      final list = BenchmarkList(
        appConfig: pb.MLPerfConfig(task: [task1]),
        backendConfig: [backendSettings1],
      );
      list.restoreSelection({task1.id: false});

      final modes = [BenchmarkRunMode.accuracy];
      final resources = list.listResources(modes: modes, skipInactive: true);

      expect(resources.length, 0);
    });
    test('resource list: accuracy', () async {
      final list = BenchmarkList(
        appConfig: pb.MLPerfConfig(task: [task1]),
        backendConfig: [backendSettings1],
      );

      final modes = [BenchmarkRunMode.accuracy];
      final resources = list.listResources(modes: modes, skipInactive: true);

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
            path: backendSettings1.modelPath,
            type: ResourceTypeEnum.model,
            md5Checksum: '',
          )));
    });
    test('resource list: performance', () async {
      final list = BenchmarkList(
        appConfig: pb.MLPerfConfig(task: [task1]),
        backendConfig: [backendSettings1],
      );

      final modes = [BenchmarkRunMode.performance];
      final resources = list.listResources(modes: modes);

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
            path: backendSettings1.modelPath,
            type: ResourceTypeEnum.model,
            md5Checksum: '',
          )));
    });
    test('resource list: test', () async {
      final list = BenchmarkList(
        appConfig: pb.MLPerfConfig(task: [task1]),
        backendConfig: [backendSettings1],
      );

      final modes = [
        BenchmarkRunMode.accuracyTest,
        BenchmarkRunMode.performanceTest,
      ];
      final resources = list.listResources(modes: modes);

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
            path: backendSettings1.modelPath,
            type: ResourceTypeEnum.model,
            md5Checksum: '',
          )));
    });
  });
}
