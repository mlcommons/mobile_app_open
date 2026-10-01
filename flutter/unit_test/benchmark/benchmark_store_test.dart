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
      pb.BackendSetting(
        benchmarkSetting: [coremlSettings1],
        fallbackPolicy: pb.FallbackPolicy.FALLBACK_COEXIST,
      ),
      'libcoremlbackend',
    );
    BackendInfo coremlFillGapsBackend() => BackendInfo.forTest(
      pb.BackendSetting(
        benchmarkSetting: [coremlSettings1],
        fallbackPolicy: pb.FallbackPolicy.FALLBACK_FILL_GAPS,
      ),
      'libcoremlbackend',
    );
    BackendInfo coremlDefaultPolicyBackend() => BackendInfo.forTest(
      pb.BackendSetting(benchmarkSetting: [coremlSettings1]),
      'libcoremlbackend',
    );

    test('match', () async {
      final store = BenchmarkStore(
        appConfig: pb.MLPerfConfig(task: [task1, task2]),
        backends: [tfliteBackend()],
        taskSelection: {},
        taskSetSelection: {},
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
        taskSetSelection: {},
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
        taskSetSelection: {},
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
        taskSetSelection: {},
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
        taskSetSelection: {},
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
        taskSetSelection: {},
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
        taskSetSelection: {},
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
        taskSetSelection: {},
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
        taskSetSelection: {},
      );
      // task2 exists only in the tflite backend: present, defaults to tflite
      final task2Benchmark = store.allBenchmarks.firstWhere(
        (e) => e.id == 'task2',
      );
      expect(task2Benchmark.backends.length, 1);
      expect(task2Benchmark.selectedBackend.info.libName, 'libtflitebackend');
    });

    test('default policy: fallback is not offered for vendor tasks', () {
      final store = BenchmarkStore(
        appConfig: pb.MLPerfConfig(task: [task1]),
        backends: [coremlDefaultPolicyBackend(), tfliteBackend()],
        taskSelection: {},
        taskSetSelection: {},
      );
      final benchmark = store.allBenchmarks.single;
      expect(benchmark.backends.map((e) => e.info.libName), [
        'libcoremlbackend',
      ]);
    });

    test(
      'FALLBACK_FILL_GAPS: fallback offered only for tasks vendor lacks',
      () {
        final store = BenchmarkStore(
          appConfig: pb.MLPerfConfig(task: [task1, task2]),
          backends: [coremlFillGapsBackend(), tfliteBackendBoth()],
          taskSelection: {},
          taskSetSelection: {},
        );
        // task1 is supported by the vendor: fallback is filtered out
        final task1Benchmark = store.allBenchmarks.firstWhere(
          (e) => e.id == 'task1',
        );
        expect(task1Benchmark.backends.map((e) => e.info.libName), [
          'libcoremlbackend',
        ]);
        // task2 is a gap: fallback still fills it
        final task2Benchmark = store.allBenchmarks.firstWhere(
          (e) => e.id == 'task2',
        );
        expect(task2Benchmark.backends.map((e) => e.info.libName), [
          'libtflitebackend',
        ]);
      },
    );

    test('FALLBACK_FILL_GAPS overrides a persisted fallback selection', () {
      final store = BenchmarkStore(
        appConfig: pb.MLPerfConfig(task: [task1]),
        backends: [coremlFillGapsBackend(), tfliteBackend()],
        taskSelection: {},
        backendSelection: {'task1': 'libtflitebackend'},
        taskSetSelection: {},
      );
      final benchmark = store.allBenchmarks.single;
      expect(benchmark.selectedBackend.info.libName, 'libcoremlbackend');
    });

    test('selectBackend swaps settings and delegate choices', () {
      final store = BenchmarkStore(
        appConfig: pb.MLPerfConfig(task: [task1]),
        backends: [coremlBackend(), tfliteBackend()],
        taskSelection: {},
        taskSetSelection: {},
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
        taskSetSelection: {},
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
        taskSetSelection: {},
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

  group('BenchmarkSet option persistence', () {
    // Mirrors flutter/assets/tasks.pbtxt: the image_classification set ships
    // with both options on, the llm set ships with every size off.
    pb.TaskSet icSet() => pb.TaskSet(
      id: 'ic',
      name: 'Image Classification v2',
      optionSet: [
        pb.OptionSet(
          id: 'options',
          opt: [
            pb.Option(id: 'offline', name: 'Offline', enabled: true),
            pb.Option(id: 'online', name: 'Online', enabled: true),
          ],
        ),
      ],
    );
    pb.TaskSet llmSet() => pb.TaskSet(
      id: 'llm',
      name: 'LLM',
      optionSet: [
        pb.OptionSet(
          id: 'parameters',
          opt: [
            pb.Option(id: '1b', name: '1B'),
            pb.Option(id: '3b', name: '3B'),
          ],
        ),
        pb.OptionSet(
          id: 'dataset',
          hidden: true,
          opt: [pb.Option(id: 'mmlu', name: 'MMLU', enabled: true)],
        ),
      ],
    );

    final icOnline = pb.TaskConfig(
      id: 'ic_online',
      taskSet: 'ic',
      requiredOption: ['online'],
    );
    final icOffline = pb.TaskConfig(
      id: 'ic_offline',
      taskSet: 'ic',
      requiredOption: ['offline'],
    );
    final llm1b = pb.TaskConfig(
      id: 'llm_1b',
      taskSet: 'llm',
      requiredOption: ['1b', 'mmlu'],
    );
    final llm3b = pb.TaskConfig(
      id: 'llm_3b',
      taskSet: 'llm',
      requiredOption: ['3b', 'mmlu'],
    );

    BackendInfo backend() => BackendInfo.forTest(
      pb.BackendSetting(
        benchmarkSetting: [
          for (final id in ['ic_online', 'ic_offline', 'llm_1b', 'llm_3b'])
            pb.BenchmarkSetting(benchmarkId: id),
        ],
      ),
      'libtflitebackend',
    );

    BenchmarkStore storeWith(Map<String, Map<String, bool>> setSelection) =>
        BenchmarkStore(
          appConfig: pb.MLPerfConfig(
            task: [icOnline, icOffline, llm1b, llm3b],
            taskSet: [icSet(), llmSet()],
          ),
          backends: [backend()],
          taskSelection: {},
          taskSetSelection: setSelection,
        );

    test('defaults come from the config when nothing is stored', () {
      final store = storeWith({});
      expect(store.activeBenchmarks.map((e) => e.id), [
        'ic_online',
        'ic_offline',
      ]);
    });

    test('restored option state decides which benchmarks are active', () {
      // The user picked "LLM only": both image classification options off,
      // the 1B parameter option on.
      final store = storeWith({
        'ic': {'offline': false, 'online': false},
        'llm': {'1b': true, '3b': false},
      });

      expect(store.activeBenchmarks.map((e) => e.id), ['llm_1b']);
    });

    test('restored option state round-trips through setSelection', () {
      final selection = {
        'ic': {'offline': false, 'online': false},
        'llm': {'1b': true, '3b': false},
      };
      expect(storeWith(selection).setSelection, selection);
    });

    test('unknown stored option ids are ignored, not fatal', () {
      // A stored id can disappear when tasks.pbtxt changes under an upgrade.
      final store = storeWith({
        'llm': {'1b': true, 'option_that_no_longer_exists': true},
      });
      // The unknown id is dropped; the rest of the stored state still lands.
      expect(store.activeBenchmarks.map((e) => e.id), contains('llm_1b'));
    });

    test('a stored set id that no longer exists is ignored', () {
      final store = storeWith({
        'set_that_no_longer_exists': {'whatever': true},
      });
      expect(store.activeBenchmarks.map((e) => e.id), [
        'ic_online',
        'ic_offline',
      ]);
    });
  });

  group('BenchmarkOptionSet selection constraints', () {
    BenchmarkOptionSet optionSet({int minSelected = 0, int maxSelected = 0}) =>
        BenchmarkOptionSet(
          config: pb.OptionSet(
            id: 'parameters',
            minSelected: minSelected,
            maxSelected: maxSelected,
            opt: [
              pb.Option(id: 'a', name: 'A', enabled: true),
              pb.Option(id: 'b', name: 'B'),
            ],
          ),
        );

    test('the selected count tracks set and unset', () {
      final set = optionSet();
      expect(set.selected, 1);
      set.setOptionTo('b', true);
      expect(set.selected, 2);
      set.setOptionTo('a', false);
      set.setOptionTo('b', false);
      expect(set.selected, 0);
    });

    test('swapping the choice works when maxSelected is 1', () {
      final set = optionSet(maxSelected: 1);
      expect(set.setOptionTo('a', false), isTrue);
      expect(set.setOptionTo('b', true), isTrue);
      expect(set.getOption('a'), isFalse);
      expect(set.getOption('b'), isTrue);
    });

    test('maxSelected still blocks an over-selection', () {
      final set = optionSet(maxSelected: 1);
      expect(set.setOptionTo('b', true), isFalse);
      expect(set.getOption('b'), isFalse);
      expect(set.selected, 1);
    });

    test('minSelected still blocks an under-selection', () {
      final set = optionSet(minSelected: 1);
      expect(set.setOptionTo('a', false), isFalse);
      expect(set.getOption('a'), isTrue);
      expect(set.selected, 1);
    });

    test('setting an already-set option does not double count', () {
      final set = optionSet();
      expect(set.setOptionTo('a', true), isTrue);
      expect(set.selected, 1);
      expect(set.setOptionTo('b', false), isTrue);
      expect(set.selected, 1);
    });
  });
}
