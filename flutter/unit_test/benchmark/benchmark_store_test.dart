// The analyzer does not treat this non-standard test dir as a test context.
// ignore_for_file: invalid_use_of_visible_for_testing_member

import 'package:flutter_test/flutter_test.dart';

import 'package:mlperfbench/backend/list.dart';
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
    final model1 = pb.ModelFile(modelPath: 'model1-path');
    final choice1 = pb.DelegateSetting(
      delegateName: 'delegate1',
      modelFile: [model1],
    );
    final backendSettings1 = pb.BenchmarkSetting(
      benchmarkId: 'task1',
      delegateChoice: [choice1],
      delegateSelected: 'delegate1',
    );

    final task2 = pb.TaskConfig(id: 'task2');
    final backendSettings2 = pb.BenchmarkSetting(benchmarkId: 'task2');

    final coremlSettings1 = pb.BenchmarkSetting(
      benchmarkId: 'task1',
      delegateChoice: [
        pb.DelegateSetting(
          delegateName: 'coreml-delegate',
          modelFile: [pb.ModelFile(modelPath: 'coreml-model1-path')],
        ),
      ],
      delegateSelected: 'coreml-delegate',
    );

    BackendInfo tfliteBackend() => BackendInfo.forTest(
      pb.BackendSetting(benchmarkSetting: [backendSettings1]),
      'libtflitebackend',
    );
    BackendInfo tfliteBackend2() => BackendInfo.forTest(
      pb.BackendSetting(benchmarkSetting: [backendSettings2]),
      'libtflitebackend',
    );
    BackendInfo tfliteBackendBoth() => BackendInfo.forTest(
      pb.BackendSetting(benchmarkSetting: [backendSettings1, backendSettings2]),
      'libtflitebackend',
    );
    BackendInfo coremlBackend() => BackendInfo.forTest(
      pb.BackendSetting(benchmarkSetting: [coremlSettings1]),
      'libcoremlbackend',
    );

    test('match', () async {
      final store = BenchmarkStore(
        appConfig: pb.MLPerfConfig(task: [task1, task2]),
        backends: [tfliteBackend()],
        taskSelection: {},
      );

      expect(store.allBenchmarks.length, 1);

      expect(store.allBenchmarks.first.taskConfig, task1);
      expect(store.allBenchmarks.first.benchmarkSettings, backendSettings1);
      expect(
        store.allBenchmarks.first.isActive,
        true,
        reason: 'benchmarks must be enabled by default',
      );
    });

    test('order', () async {
      final store = BenchmarkStore(
        appConfig: pb.MLPerfConfig(task: [task2, task1]),
        backends: [tfliteBackendBoth()],
        taskSelection: {},
      );

      expect(store.allBenchmarks.length, 2);

      expect(store.allBenchmarks.first.taskConfig, task2);
      expect(store.allBenchmarks.first.benchmarkSettings, backendSettings2);

      expect(store.allBenchmarks.last.taskConfig, task1);
      expect(store.allBenchmarks.last.benchmarkSettings, backendSettings1);
    });

    test('selection', () async {
      final store = BenchmarkStore(
        appConfig: pb.MLPerfConfig(task: [task1, task2]),
        backends: [tfliteBackendBoth()],
        taskSelection: {task1.id: true, task2.id: false},
      );

      expect(store.allBenchmarks.length, 2);
      expect(store.allBenchmarks.first.isActive, true);
      expect(store.allBenchmarks.last.isActive, false);
    });

    test('resource list: skip', () async {
      final store = BenchmarkStore(
        appConfig: pb.MLPerfConfig(task: [task1]),
        backends: [tfliteBackend()],
        taskSelection: {task1.id: false},
      );

      final modes = [BenchmarkRunModeEnum.performanceOnly.performanceRunMode];
      final resources = store.listResources(
        modes: modes,
        benchmarks: store.activeBenchmarks,
      );

      expect(resources.length, 0);
    });
    test('resource list: accuracy', () async {
      final store = BenchmarkStore(
        appConfig: pb.MLPerfConfig(task: [task1]),
        backends: [tfliteBackend()],
        taskSelection: {},
      );

      final modes = [BenchmarkRunModeEnum.accuracyOnly.accuracyRunMode];
      final resources = store.listResources(
        modes: modes,
        benchmarks: store.activeBenchmarks,
      );

      expect(resources.length, 3);
      expect(
        resources,
        contains(
          Resource(
            type: ResourceTypeEnum.datasetData,
            path: task1.datasets.full.inputPath,
            md5Checksum: task1.datasets.full.inputChecksum,
          ),
        ),
      );
      expect(
        resources,
        contains(
          Resource(
            type: ResourceTypeEnum.datasetGroundtruth,
            path: task1.datasets.full.groundtruthPath,
            md5Checksum: task1.datasets.full.groundtruthChecksum,
          ),
        ),
      );
      expect(
        resources,
        contains(
          Resource(
            path:
                backendSettings1.delegateChoice.first.modelFile.first.modelPath,
            type: ResourceTypeEnum.model,
            md5Checksum: '',
          ),
        ),
      );
    });
    test('resource list: performance', () async {
      final store = BenchmarkStore(
        appConfig: pb.MLPerfConfig(task: [task1]),
        backends: [tfliteBackend()],
        taskSelection: {},
      );

      final modes = [BenchmarkRunModeEnum.performanceOnly.performanceRunMode];
      final activeBenchmarks = store.activeBenchmarks;
      final resources = store.listResources(
        modes: modes,
        benchmarks: activeBenchmarks,
      );

      expect(resources.length, 2);
      expect(
        resources,
        contains(
          Resource(
            type: ResourceTypeEnum.datasetData,
            path: task1.datasets.lite.inputPath,
            md5Checksum: task1.datasets.lite.inputChecksum,
          ),
        ),
      );
      expect(
        resources,
        contains(
          Resource(
            path:
                backendSettings1.delegateChoice.first.modelFile.first.modelPath,
            type: ResourceTypeEnum.model,
            md5Checksum: '',
          ),
        ),
      );
    });
    test('resource list: test', () async {
      final store = BenchmarkStore(
        appConfig: pb.MLPerfConfig(task: [task1]),
        backends: [tfliteBackend()],
        taskSelection: {},
      );

      final modes = [
        BenchmarkRunModeEnum.integrationTestRun.accuracyRunMode,
        BenchmarkRunModeEnum.integrationTestRun.performanceRunMode,
      ];
      final activeBenchmarks = store.activeBenchmarks;
      final resources = store.listResources(
        modes: modes,
        benchmarks: activeBenchmarks,
      );

      expect(resources.length, 3);
      expect(
        resources,
        contains(
          Resource(
            type: ResourceTypeEnum.datasetData,
            path: task1.datasets.lite.inputPath,
            md5Checksum: task1.datasets.lite.inputChecksum,
          ),
        ),
      );
      expect(
        resources,
        contains(
          Resource(
            path:
                backendSettings1.delegateChoice.first.modelFile.first.modelPath,
            type: ResourceTypeEnum.model,
            md5Checksum: '',
          ),
        ),
      );
    });

    test('multi-backend: candidates in priority order, first is default', () {
      final store = BenchmarkStore(
        appConfig: pb.MLPerfConfig(task: [task1, task2]),
        backends: [coremlBackend(), tfliteBackend()],
        taskSelection: {},
      );
      final task1Benchmark = store.allBenchmarks.firstWhere(
        (e) => e.id == 'task1',
      );
      expect(task1Benchmark.backends.map((e) => e.info.libName), [
        'libcoremlbackend',
        'libtflitebackend',
      ]);
      expect(task1Benchmark.selectedBackend.info.libName, 'libcoremlbackend');
      expect(task1Benchmark.benchmarkSettings, coremlSettings1);
    });

    test('multi-backend: task unsupported by first backend falls back', () {
      final store = BenchmarkStore(
        appConfig: pb.MLPerfConfig(task: [task1, task2]),
        backends: [coremlBackend(), tfliteBackend2()],
        taskSelection: {},
      );
      // task2 exists only in the tflite backend: present, defaults to tflite
      final task2Benchmark = store.allBenchmarks.firstWhere(
        (e) => e.id == 'task2',
      );
      expect(task2Benchmark.backends.length, 1);
      expect(task2Benchmark.selectedBackend.info.libName, 'libtflitebackend');
    });

    test('selectBackend swaps settings and delegate choices', () {
      final store = BenchmarkStore(
        appConfig: pb.MLPerfConfig(task: [task1]),
        backends: [coremlBackend(), tfliteBackend()],
        taskSelection: {},
      );
      final benchmark = store.allBenchmarks.single;
      expect(benchmark.selectedDelegate.delegateName, 'coreml-delegate');
      expect(benchmark.selectBackend('libtflitebackend'), isTrue);
      expect(benchmark.benchmarkSettings, backendSettings1);
      expect(benchmark.selectedDelegate.delegateName, 'delegate1');
      expect(benchmark.selectBackend('libunknownbackend'), isFalse);
      expect(benchmark.selectedBackend.info.libName, 'libtflitebackend');
    });

    test('persisted backend selection is applied, stale entries ignored', () {
      final store = BenchmarkStore(
        appConfig: pb.MLPerfConfig(task: [task1, task2]),
        backends: [coremlBackend(), tfliteBackendBoth()],
        taskSelection: {},
        backendSelection: {
          'task1': 'libtflitebackend',
          'task2': 'libgonebackend',
        },
      );
      final task1Benchmark = store.allBenchmarks.firstWhere(
        (e) => e.id == 'task1',
      );
      expect(task1Benchmark.selectedBackend.info.libName, 'libtflitebackend');
      final task2Benchmark = store.allBenchmarks.firstWhere(
        (e) => e.id == 'task2',
      );
      expect(task2Benchmark.selectedBackend.info.libName, 'libtflitebackend');
    });

    test('listResources follows the selected backend', () {
      final store = BenchmarkStore(
        appConfig: pb.MLPerfConfig(task: [task1]),
        backends: [coremlBackend(), tfliteBackend()],
        taskSelection: {},
      );
      final modes = [BenchmarkRunModeEnum.performanceOnly.performanceRunMode];
      final before = store.listResources(
        modes: modes,
        benchmarks: store.allBenchmarks,
      );
      expect(before.map((e) => e.path), contains('coreml-model1-path'));
      expect(before.map((e) => e.path), isNot(contains('model1-path')));

      store.allBenchmarks.single.selectBackend('libtflitebackend');
      final after = store.listResources(
        modes: modes,
        benchmarks: store.allBenchmarks,
      );
      expect(after.map((e) => e.path), contains('model1-path'));
      expect(after.map((e) => e.path), isNot(contains('coreml-model1-path')));
    });

    test('mergeCustomSettings is idempotent', () {
      final settings = pb.BenchmarkSetting(benchmarkId: 'task1');
      final configs = [pb.CustomConfig(id: 'cc1', value: 'v1')];
      mergeCustomSettings(settings, configs);
      mergeCustomSettings(settings, configs);
      expect(settings.customSetting.length, 1);
      expect(settings.customSetting.single.id, 'cc1');
      expect(settings.customSetting.single.value, 'v1');
    });
  });
}
