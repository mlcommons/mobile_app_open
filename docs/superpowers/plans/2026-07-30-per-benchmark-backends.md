# Per-Benchmark Backend Selection Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Each benchmark can run on a different backend library (e.g. iOS: image classification on `libcoremlbackend`, LLM benchmarks on `libtflitebackend`), selectable per benchmark from the config screen.

**Architecture:** Enumerate all backends matching the device instead of exactly one; attach the per-benchmark candidate list `(BackendInfo, BenchmarkSetting)` to each `Benchmark` with the highest-priority candidate as default; turn `Benchmark.benchmarkSettings` into a getter over the selection so delegate choices, resources, runs, and exports follow automatically. All changes are Dart — the C++ bridge already loads the backend lib per run from `RunSettings.backend_lib_name`.

**Tech Stack:** Flutter 3.44.4 / Dart 3.12, protobuf-generated Dart classes, `flutter test`.

Spec: `docs/superpowers/specs/2026-07-30-per-benchmark-backends-design.md`

## Global Constraints

- Working dir for all commands: `flutter/` inside the repo.
- Run unit tests with `flutter test --no-pub unit_test/* -r compact`; plain `flutter test` runs `test/`.
- `flutter analyze` must be clean (zero issues) at the end of every task.
- The tree must compile at every task boundary (analyzer clean = proof).
- No C++, proto, or result-JSON-schema changes.
- Behavior with a single matching backend must remain identical (apart from the TFLite fallback now being offered as an additional choice).
- Format changed files with `dart format <files>` before committing.
- Commit messages: conventional commits (`feat:`, `test:`, `docs:`), no Co-Authored-By trailers.

---

### Task 1: `findMatchingBackends()` in `list.dart`

**Files:**
- Modify: `flutter/lib/backend/list.dart`
- Test: `flutter/unit_test/backend/backend_list_test.dart` (create)

**Interfaces:**
- Produces: `List<BackendInfo> BackendInfoHelper.findMatchingBackends({bool alwaysOfferFallback})`, `BackendInfo.forTest(pb.BackendSetting settings, String libName)`, static consts `BackendInfoHelper.fallbackBackend` / `BackendInfoHelper.alwaysOfferFallback` (== `true`).
- Keeps temporarily: `BackendInfo findMatching()` (delegates to `.first`; removed in Task 3).

- [ ] **Step 1: Write the failing test**

Create `flutter/unit_test/backend/backend_list_test.dart`:

```dart
import 'package:flutter_test/flutter_test.dart';

import 'package:mlperfbench/backend/list.dart';
import 'package:mlperfbench/protos/backend_setting.pb.dart' as pb;

class _FakeBackendInfoHelper extends BackendInfoHelper {
  final List<String> backends;
  final Set<String> matching;

  _FakeBackendInfoHelper({required this.backends, required this.matching});

  @override
  List<String> getBackendsList() => backends;

  @override
  pb.BackendSetting? match(String libName) {
    if (matching.contains(libName)) {
      return pb.BackendSetting(
        benchmarkSetting: [pb.BenchmarkSetting(benchmarkId: 'bm-$libName')],
      );
    }
    return null;
  }
}

void main() {
  group('BackendInfoHelper.findMatchingBackends', () {
    const vendor1 = 'libvendor1backend';
    const vendor2 = 'libvendor2backend';
    const tflite = 'libtflitebackend';
    const allBackends = [vendor1, vendor2, tflite];

    test('vendor match also offers the fallback, in priority order', () {
      final helper = _FakeBackendInfoHelper(
        backends: allBackends,
        matching: {vendor1, tflite},
      );
      final matches = helper.findMatchingBackends();
      expect(matches.map((e) => e.libName), [vendor1, tflite]);
    });

    test('multiple vendor matches are all offered, in list order', () {
      final helper = _FakeBackendInfoHelper(
        backends: allBackends,
        matching: {vendor1, vendor2, tflite},
      );
      final matches = helper.findMatchingBackends();
      expect(matches.map((e) => e.libName), [vendor1, vendor2, tflite]);
    });

    test('fallback-only build returns just the fallback', () {
      final helper = _FakeBackendInfoHelper(
        backends: allBackends,
        matching: {tflite},
      );
      final matches = helper.findMatchingBackends();
      expect(matches.map((e) => e.libName), [tflite]);
    });

    test('alwaysOfferFallback=false reproduces single-backend behavior', () {
      final helper = _FakeBackendInfoHelper(
        backends: allBackends,
        matching: {vendor1, tflite},
      );
      final matches = helper.findMatchingBackends(alwaysOfferFallback: false);
      expect(matches.map((e) => e.libName), [vendor1]);
    });

    test('no matching backend throws', () {
      final helper = _FakeBackendInfoHelper(
        backends: allBackends,
        matching: {},
      );
      expect(() => helper.findMatchingBackends(), throwsA(isA<String>()));
    });

    test('settings are carried through per backend', () {
      final helper = _FakeBackendInfoHelper(
        backends: allBackends,
        matching: {vendor1, tflite},
      );
      final matches = helper.findMatchingBackends();
      expect(
        matches.first.settings.benchmarkSetting.first.benchmarkId,
        'bm-$vendor1',
      );
    });
  });
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `flutter test --no-pub unit_test/backend/backend_list_test.dart -r compact`
Expected: FAIL — `findMatchingBackends` is not defined.

- [ ] **Step 3: Implement**

In `flutter/lib/backend/list.dart`, replace the `findMatching()` method body and add the new API. The full new file content of the changed parts:

```dart
import 'dart:io' show Platform;

import 'package:flutter/foundation.dart' show visibleForTesting;

import 'package:mlperfbench/backend/bridge/ffi_match.dart';
import 'package:mlperfbench/data/environment/environment_info.dart';
import 'package:mlperfbench/device_info.dart';
import 'package:mlperfbench/protos/backend_setting.pb.dart' as pb;

part 'list.gen.dart';

class BackendInfoHelper {
  static const fallbackBackend = 'libtflitebackend';

  // When true, the TFLite fallback backend is offered alongside any matching
  // vendor backend, so each benchmark can choose among all matching backends
  // and tasks the vendor backend lacks fall back to TFLite.
  // When false, the fallback is probed only when no vendor backend matches,
  // which reproduces the historical single-backend-per-session behavior.
  static const alwaysOfferFallback = true;

  // Returns all matching backends in priority order
  // (the order of _backendsList, vendor backends before the fallback).
  List<BackendInfo> findMatchingBackends({
    bool alwaysOfferFallback = BackendInfoHelper.alwaysOfferFallback,
  }) {
    final matches = <BackendInfo>[];
    for (var name in getBackendsList()) {
      if (name == fallbackBackend) continue;
      print('Checking $name');
      final backendSettings = match(name);
      if (backendSettings != null) {
        matches.add(BackendInfo._(backendSettings, name));
      }
    }

    if (matches.isEmpty || alwaysOfferFallback) {
      print('Checking $fallbackBackend');
      final backendSettings = match(fallbackBackend);
      if (backendSettings != null) {
        matches.add(BackendInfo._(backendSettings, fallbackBackend));
      }
    }
    if (matches.isEmpty) {
      throw 'no matching backend found';
    }
    print('Matching backends: ${matches.map((e) => e.libName).join(', ')}');
    return matches;
  }

  // TEMPORARY shim so existing call sites keep compiling; removed in the
  // core-rewiring task of the per-benchmark-backends plan.
  BackendInfo findMatching() => findMatchingBackends().first;

  // ... match / matchAndroid / matchIos / matchWindows / getBackendsList
  // stay exactly as they are ...
}

class BackendInfo {
  final pb.BackendSetting settings;
  final String libName;

  BackendInfo._(this.settings, this.libName);

  @visibleForTesting
  BackendInfo.forTest(this.settings, this.libName);
}
```

- [ ] **Step 4: Run tests**

Run: `flutter test --no-pub unit_test/backend/backend_list_test.dart -r compact` → PASS.
Run: `flutter analyze` → No issues.

- [ ] **Step 5: Commit**

```bash
git add lib/backend/list.dart unit_test/backend/backend_list_test.dart
git commit -m "feat: enumerate all matching backends instead of exactly one"
```

---

### Task 2: `Store.backendSelection`

**Files:**
- Modify: `flutter/lib/store.dart`
- Test: `flutter/test/store_test.dart`

**Interfaces:**
- Produces: `String Store.backendSelection` getter/setter (JSON map `{benchmarkId: libName}` as a string, default `''`), `StoreConstants.backendSelection == 'backend_selection'`.

- [ ] **Step 1: Write the failing test**

Append to `flutter/test/store_test.dart` inside `main()`:

```dart
  test('backendSelection defaults to empty and persists a JSON string', () async {
    final store = await Store.create();
    expect(store.backendSelection, '');

    store.backendSelection = '{"image_classification":"libcoremlbackend"}';
    expect(
      store.backendSelection,
      '{"image_classification":"libcoremlbackend"}',
    );
  });
```

- [ ] **Step 2: Run test to verify it fails**

Run: `flutter test test/store_test.dart -r compact`
Expected: FAIL — `backendSelection` is not defined.

- [ ] **Step 3: Implement**

In `flutter/lib/store.dart`, next to the `taskSelection` getter/setter add:

```dart
  String get backendSelection => _getString(StoreConstants.backendSelection);

  set backendSelection(String value) {
    _storeFromDisk.setString(StoreConstants.backendSelection, value);
  }
```

and in `StoreConstants`:

```dart
  static const backendSelection = 'backend_selection';
```

- [ ] **Step 4: Run tests**

Run: `flutter test test/store_test.dart -r compact` → PASS. `flutter analyze` → No issues.

- [ ] **Step 5: Commit**

```bash
git add lib/store.dart test/store_test.dart
git commit -m "feat: persist per-benchmark backend selection in Store"
```

---

### Task 3: Core rewiring — `Benchmark`, `BenchmarkStore`, state, runner, export

This task is atomic by necessity: `Benchmark`'s constructor change ripples through `BenchmarkStore` → `BenchmarkState` → `TaskRunner` → `ResultHelper` in one compile unit.

**Files:**
- Modify: `flutter/lib/benchmark/benchmark.dart`
- Modify: `flutter/lib/benchmark/state.dart`
- Modify: `flutter/lib/state/task_runner.dart`
- Modify: `flutter/lib/resources/export_result_helper.dart`
- Modify: `flutter/lib/backend/list.dart` (remove the `findMatching()` shim)
- Modify: `flutter/integration_test/utils.dart` (uses `state.backendInfo`, must follow)
- Test: `flutter/unit_test/benchmark/benchmark_store_test.dart`

**Interfaces:**
- Produces:
  - `class BenchmarkBackend { final BackendInfo info; final pb.BenchmarkSetting settings; BenchmarkBackend({required this.info, required this.settings}); }`
  - `Benchmark({required List<BenchmarkBackend> backends, required pb.TaskConfig taskConfig, required bool isActive})`
  - `Benchmark.backends` (priority order, length ≥ 1), `Benchmark.selectedBackend`, `bool Benchmark.selectBackend(String libName)`
  - `Benchmark.benchmarkSettings` / `Benchmark.backendRequestDescription` become getters over the selection
  - `Benchmark.createRunSettings({required runMode, required resourceManager, required logDir})` — `commonSettings`/`backendLibName` params removed, derived from `selectedBackend`
  - `void mergeCustomSettings(pb.BenchmarkSetting settings, List<pb.CustomConfig> configs)` (top-level, `@visibleForTesting`)
  - `BenchmarkStore({required pb.MLPerfConfig appConfig, required List<BackendInfo> backends, required Map<String, bool> taskSelection, Map<String, String> backendSelection = const {}})`
  - `Map<String, String> BenchmarkStore.backendSelectionMap`
  - `BenchmarkState.matchedBackends` (replaces `backendInfo`), `void BenchmarkState.benchmarkSetBackend(Benchmark benchmark, String libName)`
- Consumes: Task 1 `findMatchingBackends()` + `BackendInfo.forTest`; Task 2 `Store.backendSelection`.

- [ ] **Step 1: Update `flutter/lib/benchmark/benchmark.dart`**

Replace the `Benchmark` class head (fields + ctor + `selectedDelegate` stays) with:

```dart
class BenchmarkBackend {
  final BackendInfo info;
  final pb.BenchmarkSetting settings;

  BenchmarkBackend({required this.info, required this.settings});
}

class Benchmark {
  final List<BenchmarkBackend> backends;
  final pb.TaskConfig taskConfig;
  bool isActive;

  final BenchmarkInfo info;

  BenchmarkBackend _selectedBackend;

  Benchmark({
    required this.backends,
    required this.taskConfig,
    required this.isActive,
  }) : assert(backends.isNotEmpty),
       info = BenchmarkInfo(taskConfig),
       _selectedBackend = backends.first;

  String get id => taskConfig.id;

  BenchmarkBackend get selectedBackend => _selectedBackend;

  pb.BenchmarkSetting get benchmarkSettings => _selectedBackend.settings;

  // this getter holds description of our config file,
  // which may not represent what backend actually used for computations
  String get backendRequestDescription => benchmarkSettings.framework;

  // Selects the backend with the given lib name.
  // Returns false if it is not one of this benchmark's candidates.
  bool selectBackend(String libName) {
    final backend = backends.firstWhereOrNull(
      (e) => e.info.libName == libName,
    );
    if (backend == null) return false;
    _selectedBackend = backend;
    return true;
  }
```

(`import 'package:mlperfbench/backend/list.dart';` must be added to the imports; `collection` is already imported. Add `import 'package:flutter/foundation.dart' show visibleForTesting;`.)

In `createRunSettings`, remove the `commonSettings` and `backendLibName` parameters and derive both from the selection; namespace the symlink dir by backend lib and benchmark id; make the customSetting merge idempotent:

```dart
  Future<RunSettings> createRunSettings({
    required BenchmarkRunMode runMode,
    required ResourceManager resourceManager,
    required String logDir,
  }) async {
    final dataset = runMode.chooseDataset(taskConfig);
    final runConfig = runMode.chooseRunConfig(taskConfig);

    int minQueryCount = runConfig.minQueryCount;
    double minDuration = runConfig.minDuration;
    double maxDuration = runConfig.maxDuration;

    final settings = pb.SettingList(
      setting: _selectedBackend.info.settings.commonSetting,
      benchmarkSetting: benchmarkSettings,
    );
    mergeCustomSettings(benchmarkSettings, taskConfig.customConfig);
    final uris = selectedDelegate.modelFile.map((e) => e.modelPath).toList();
    // Namespace the symlink dir by backend and benchmark so backends that
    // share a delegate name cannot cross-contaminate each other's models.
    final modelDirName =
        '${_selectedBackend.info.libName}_${id}_${selectedDelegate.delegateName}'
            .replaceAll(' ', '_');
    final backendModelPath = await resourceManager.getModelPath(
      uris,
      modelDirName,
    );
    return RunSettings(
      backend_model_path: backendModelPath,
      backend_lib_name: _selectedBackend.info.libName,
      backend_settings: settings,
      // ... rest of the RunSettings arguments unchanged ...
    );
  }
```

Add the top-level helper (bottom of file):

```dart
// Convert TaskConfig.CustomConfig to BenchmarkSetting.CustomSetting.
// Idempotent: settings ids that are already present are left untouched, so
// repeated runs do not append duplicates.
@visibleForTesting
void mergeCustomSettings(
  pb.BenchmarkSetting settings,
  List<pb.CustomConfig> configs,
) {
  for (final config in configs) {
    if (settings.customSetting.any((e) => e.id == config.id)) continue;
    settings.customSetting.add(
      pb.CustomSetting(id: config.id, value: config.value),
    );
  }
}
```

Replace the single-backend matching loop in `BenchmarkStore`'s constructor:

```dart
  BenchmarkStore({
    required pb.MLPerfConfig appConfig,
    required List<BackendInfo> backends,
    required Map<String, bool> taskSelection,
    Map<String, String> backendSelection = const {},
  }) {
    // sort the order of task based on BenchmarkId.allIds
    final List<pb.TaskConfig> sortedTasks = List.from(appConfig.task)
      ..sort(
        (a, b) =>
            BenchmarkId.allIds.indexOf(a.id) - BenchmarkId.allIds.indexOf(b.id),
      );
    for (final task in sortedTasks) {
      final candidates = <BenchmarkBackend>[];
      for (final backend in backends) {
        final setting = backend.settings.benchmarkSetting.singleWhereOrNull(
          (s) => s.benchmarkId == task.id,
        );
        if (setting != null) {
          candidates.add(BenchmarkBackend(info: backend, settings: setting));
        }
      }
      if (candidates.isEmpty) {
        print('No matching benchmark settings for task ${task.id}');
        continue;
      }

      final enabled = taskSelection[task.id] ?? true;
      final benchmark = Benchmark(
        backends: candidates,
        taskConfig: task,
        isActive: enabled,
      );
      final selectedLib = backendSelection[task.id];
      if (selectedLib != null && !benchmark.selectBackend(selectedLib)) {
        print('Ignoring unknown backend $selectedLib for task ${task.id}');
      }
      allBenchmarks.add(benchmark);
    }
    for (final setConfig in appConfig.taskSet) {
      benchmarkSets.add(
        BenchmarkSet(config: setConfig, allBenchmarks: allBenchmarks),
      );
    }
  }

  Map<String, String> get backendSelectionMap => {
    for (final b in allBenchmarks) b.id: b.selectedBackend.info.libName,
  };
```

- [ ] **Step 2: Update `flutter/lib/benchmark/state.dart`**

- Field: `late final BackendInfo backendInfo;` → `late final List<BackendInfo> matchedBackends;`
- Ctor: `backendInfo = BackendInfoHelper().findMatching();` → `matchedBackends = BackendInfoHelper().findMatchingBackends();` and remove `backendInfo:` from the `TaskRunner(...)` call.
- In `setTaskConfig`, after the `taskSelection` parsing block, add the analogous parse:

```dart
    Map<String, String> backendSelection = {};
    if (_store.backendSelection.isNotEmpty) {
      try {
        final map = jsonDecode(_store.backendSelection) as Map<String, dynamic>;
        for (var kv in map.entries) {
          backendSelection[kv.key] = kv.value as String;
        }
      } catch (e, t) {
        print('Backend selection parse fail: $e');
        print(t);
      }
    }

    _benchmarkStore = BenchmarkStore(
      appConfig: configManager.decodedConfig,
      backends: matchedBackends,
      taskSelection: taskSelection,
      backendSelection: backendSelection,
    );
```

- Next to `benchmarkSetDelegate`, add:

```dart
  void benchmarkSetBackend(Benchmark benchmark, String libName) {
    if (state == BenchmarkStateEnum.running ||
        state == BenchmarkStateEnum.aborting) {
      return;
    }
    if (!benchmark.selectBackend(libName)) return;
    _store.backendSelection = jsonEncode(_benchmarkStore.backendSelectionMap);
    notifyListeners();
  }
```

- [ ] **Step 3: Update `flutter/lib/state/task_runner.dart`**

- Delete the `final BackendInfo backendInfo;` field and the `required this.backendInfo,` ctor param.
- `ResultHelper(...)`: delete the `backendInfo: backendInfo,` argument.
- Both `initRunSettings(...)` call sites: delete the `commonSettings:` and `backendLibName:` arguments.
- `_NativeRunHelper.initRunSettings`: drop both params:

```dart
  Future<void> initRunSettings({
    required ResourceManager resourceManager,
  }) async {
    runSettings = await benchmark.createRunSettings(
      runMode: runMode,
      resourceManager: resourceManager,
      logDir: logDir,
    );
    // ... rest unchanged ...
  }
```

- Remove imports that become unused (`backend/list.dart`, and `protos/backend_setting.pb.dart` if the `pb.` prefix is no longer referenced) — let `flutter analyze` be the guide.

- [ ] **Step 4: Update `flutter/lib/resources/export_result_helper.dart`**

- Delete the `final BackendInfo backendInfo;` field and ctor param, and the now-unused `backend/list.dart` import.
- `_makeBackendInfo` takes the `RunInfo` so the filename comes from the run's own settings snapshot:

```dart
      backendInfo: _makeBackendInfo(runInfo),
```

```dart
  BackendReportedInfo _makeBackendInfo(RunInfo runInfo) {
    final result = runInfo.result;
    return BackendReportedInfo(
      filename: runInfo.settings.backend_lib_name,
      backendName: result.backendName,
      vendorName: result.backendVendor,
      acceleratorName: result.acceleratorName,
    );
  }
```

(Verify the exact field name on `RunInfo` — it wraps the `RunSettings` passed to the native run; adjust `runInfo.settings` accordingly after reading `flutter/lib/benchmark/run_info.dart`.)

- [ ] **Step 5: Update `flutter/integration_test/utils.dart` `canRunBenchmark`**

```dart
bool canRunBenchmark(WidgetTester tester, String benchmarkId) {
  if (benchmarkId == 'stable_diffusion') {
    final state = tester.state(find.byType(MaterialApp));
    final benchmarkState = state.context.read<BenchmarkState>();
    final benchmark = benchmarkState.allBenchmarks.firstWhereOrNull(
      (e) => e.id == benchmarkId,
    );
    return benchmark?.selectedBackend.info.libName == BackendId.qti;
  }
  return true;
}
```

Add `import 'package:collection/collection.dart';` to the file's imports.

- [ ] **Step 6: Remove the `findMatching()` shim from `flutter/lib/backend/list.dart`** (added in Task 1).

- [ ] **Step 7: Rewrite `flutter/unit_test/benchmark/benchmark_store_test.dart`**

Keep the existing fixtures; wrap them in `BackendInfo.forTest`. Replace every old `BenchmarkStore(backendConfig: [...], ...)` call with the new signature, e.g. the `match` test becomes:

```dart
    final tfliteBackend = BackendInfo.forTest(
      pb.BackendSetting(benchmarkSetting: [backendSettings1]),
      'libtflitebackend',
    );

    test('match', () async {
      final store = BenchmarkStore(
        appConfig: pb.MLPerfConfig(task: [task1, task2]),
        backends: [tfliteBackend],
        taskSelection: {},
      );
      // same expectations as before
    });
```

(`import 'package:mlperfbench/backend/list.dart';` is needed.)

Add these new tests to the group (fixtures at group level):

```dart
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
    final coremlBackend = BackendInfo.forTest(
      pb.BackendSetting(benchmarkSetting: [coremlSettings1]),
      'libcoremlbackend',
    );

    test('multi-backend: candidates in priority order, first is default', () {
      final store = BenchmarkStore(
        appConfig: pb.MLPerfConfig(task: [task1, task2]),
        backends: [coremlBackend, tfliteBackend],
        taskSelection: {},
      );
      final task1Benchmark = store.allBenchmarks.firstWhere(
        (e) => e.id == 'task1',
      );
      expect(
        task1Benchmark.backends.map((e) => e.info.libName),
        ['libcoremlbackend', 'libtflitebackend'],
      );
      expect(task1Benchmark.selectedBackend.info.libName, 'libcoremlbackend');
      expect(task1Benchmark.benchmarkSettings, coremlSettings1);
    });

    test('multi-backend: task unsupported by first backend falls back', () {
      final store = BenchmarkStore(
        appConfig: pb.MLPerfConfig(task: [task1, task2]),
        backends: [coremlBackend, tfliteBackend2],
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
        backends: [coremlBackend, tfliteBackend],
        taskSelection: {},
      );
      final benchmark = store.allBenchmarks.single;
      expect(benchmark.selectBackend('libtflitebackend'), isTrue);
      expect(benchmark.benchmarkSettings, backendSettings1);
      expect(benchmark.selectedDelegate.delegateName, 'delegate1');
      expect(benchmark.selectBackend('libunknownbackend'), isFalse);
      expect(benchmark.selectedBackend.info.libName, 'libtflitebackend');
    });

    test('persisted backend selection is applied, stale entries ignored', () {
      final store = BenchmarkStore(
        appConfig: pb.MLPerfConfig(task: [task1, task2]),
        backends: [coremlBackend, tfliteBackend],
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
      expect(
        store.backendSelectionMap['task1'],
        'libtflitebackend',
      );
    });

    test('listResources follows the selected backend', () {
      final store = BenchmarkStore(
        appConfig: pb.MLPerfConfig(task: [task1]),
        backends: [coremlBackend, tfliteBackend],
        taskSelection: {},
      );
      final modes = [BenchmarkRunModeEnum.performanceOnly.performanceRunMode];
      final before = store.listResources(
        modes: modes,
        benchmarks: store.allBenchmarks,
      );
      expect(
        before.map((e) => e.path),
        contains('coreml-model1-path'),
      );
      store.allBenchmarks.single.selectBackend('libtflitebackend');
      final after = store.listResources(
        modes: modes,
        benchmarks: store.allBenchmarks,
      );
      expect(after.map((e) => e.path), contains('model1-path'));
      expect(
        after.map((e) => e.path),
        isNot(contains('coreml-model1-path')),
      );
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
```

`tfliteBackend2` is a group-level fixture: `BackendInfo.forTest(pb.BackendSetting(benchmarkSetting: [backendSettings2]), 'libtflitebackend')`. Also give `backendSettings1` a `delegateSelected: 'delegate1'` so `selectedDelegate` resolves. `pb.CustomConfig` comes from the `mlperf_task` proto import that is already present.

- [ ] **Step 8: Run tests**

Run: `flutter test --no-pub unit_test/* -r compact` → all PASS.
Run: `flutter test -r compact` → all PASS.
Run: `flutter analyze` → No issues.

- [ ] **Step 9: Commit**

```bash
git add -A lib integration_test unit_test
git commit -m "feat: per-benchmark backend selection in benchmark store and runner"
```

---

### Task 4: Backend dropdown in the config screen

**Files:**
- Modify: `flutter/lib/ui/home/benchmark_config_section.dart`
- Test: `flutter/unit_test/ui/backend_choice_test.dart` (create)

**Interfaces:**
- Consumes: `Benchmark.backends`, `Benchmark.selectedBackend`, `BenchmarkState.benchmarkSetBackend` (Task 3).
- Produces: public `BackendChoice` StatelessWidget (in `benchmark_config_section.dart`) with `{required Benchmark benchmark, required ValueChanged<String> onChanged}`, and top-level `Map<String, String> backendChoiceLabels(Benchmark benchmark)` keyed by libName.

- [ ] **Step 1: Write the failing widget test**

Create `flutter/unit_test/ui/backend_choice_test.dart`:

```dart
import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';

import 'package:mlperfbench/backend/list.dart';
import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/protos/backend_setting.pb.dart' as pb;
import 'package:mlperfbench/protos/mlperf_task.pb.dart' as task_pb;
import 'package:mlperfbench/ui/home/benchmark_config_section.dart';

Benchmark _makeBenchmark(List<(String, String)> backendsSpec) {
  // backendsSpec: list of (libName, framework)
  final backends = backendsSpec.map((spec) {
    final (libName, framework) = spec;
    final settings = pb.BenchmarkSetting(
      benchmarkId: 'task1',
      framework: framework,
    );
    return BenchmarkBackend(
      info: BackendInfo.forTest(
        pb.BackendSetting(benchmarkSetting: [settings]),
        libName,
      ),
      settings: settings,
    );
  }).toList();
  return Benchmark(
    backends: backends,
    taskConfig: task_pb.TaskConfig(id: 'task1'),
    isActive: true,
  );
}

Future<void> _pump(WidgetTester tester, Widget child) {
  return tester.pumpWidget(
    MaterialApp(home: Scaffold(body: child)),
  );
}

void main() {
  testWidgets('single backend renders static text, no dropdown', (
    tester,
  ) async {
    final benchmark = _makeBenchmark([('libtflitebackend', 'TFLite')]);
    await _pump(
      tester,
      BackendChoice(benchmark: benchmark, onChanged: (_) {}),
    );
    expect(find.text('TFLite'), findsOneWidget);
    expect(find.byType(DropdownButton<String>), findsNothing);
  });

  testWidgets('two backends render a dropdown; selection calls back', (
    tester,
  ) async {
    final benchmark = _makeBenchmark([
      ('libcoremlbackend', 'Core ML'),
      ('libtflitebackend', 'TFLite'),
    ]);
    String? selected;
    await _pump(
      tester,
      BackendChoice(
        benchmark: benchmark,
        onChanged: (value) => selected = value,
      ),
    );
    expect(find.byType(DropdownButton<String>), findsOneWidget);
    expect(find.text('Core ML'), findsOneWidget);

    await tester.tap(find.byType(DropdownButton<String>));
    await tester.pumpAndSettle();
    await tester.tap(find.text('TFLite').last);
    await tester.pumpAndSettle();
    expect(selected, 'libtflitebackend');
  });

  test('colliding framework labels are disambiguated by lib name', () {
    final benchmark = _makeBenchmark([
      ('libqtibackend', 'TFLite'),
      ('libtflitebackend', 'TFLite'),
    ]);
    final labels = backendChoiceLabels(benchmark);
    expect(labels['libqtibackend'], 'TFLite (qti)');
    expect(labels['libtflitebackend'], 'TFLite (tflite)');
  });
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `flutter test --no-pub unit_test/ui/backend_choice_test.dart -r compact`
Expected: FAIL — `BackendChoice` not defined.

- [ ] **Step 3: Implement**

In `flutter/lib/ui/home/benchmark_config_section.dart`:

Add at the bottom of the file:

```dart
// Labels come from each backend's pbtxt framework value; when two backends
// declare the same framework name, append a cleaned libName to disambiguate.
Map<String, String> backendChoiceLabels(Benchmark benchmark) {
  final frameworks = benchmark.backends
      .map((b) => b.settings.framework)
      .toList();
  final labels = <String, String>{};
  for (final b in benchmark.backends) {
    final framework = b.settings.framework;
    final collision = frameworks.where((f) => f == framework).length > 1;
    if (collision) {
      final cleaned = b.info.libName
          .replaceFirst(RegExp('^lib'), '')
          .replaceFirst(RegExp(r'backend$'), '');
      labels[b.info.libName] = '$framework ($cleaned)';
    } else {
      labels[b.info.libName] = framework;
    }
  }
  return labels;
}

class BackendChoice extends StatelessWidget {
  final Benchmark benchmark;
  final ValueChanged<String> onChanged;

  const BackendChoice({
    super.key,
    required this.benchmark,
    required this.onChanged,
  });

  @override
  Widget build(BuildContext context) {
    final style = Theme.of(context).textTheme.labelLarge;
    if (benchmark.backends.length <= 1) {
      return Text(
        benchmark.backendRequestDescription,
        style: style,
        maxLines: 1,
        overflow: TextOverflow.ellipsis,
        softWrap: false,
      );
    }
    final labels = backendChoiceLabels(benchmark);
    return SizedBox(
      height: 24,
      child: DropdownButton<String>(
        isExpanded: false,
        isDense: false,
        padding: const EdgeInsets.only(left: 6),
        icon: const Icon(Icons.expand_more_rounded),
        borderRadius: BorderRadius.circular(WidgetSizes.borderRadius),
        underline: const SizedBox(),
        value: benchmark.selectedBackend.info.libName,
        items: benchmark.backends
            .map(
              (b) => DropdownMenuItem<String>(
                value: b.info.libName,
                child: Text(labels[b.info.libName]!, style: style),
              ),
            )
            .toList(),
        onChanged: (value) {
          if (value != null) onChanged(value);
        },
        style: style,
      ),
    );
  }
}
```

Replace the `_backendDescription` method and both of its call sites with `BackendChoice`:

```dart
        BackendChoice(
          benchmark: benchmark,
          onChanged: (libName) => state.benchmarkSetBackend(benchmark, libName),
        ),
```

(At each call site `state` is available the same way `_delegateChoice` receives it; pass it through if the enclosing helper doesn't already have it. Delete `_backendDescription` entirely.)

- [ ] **Step 4: Run tests**

Run: `flutter test --no-pub unit_test/ui/backend_choice_test.dart -r compact` → PASS.
Run: `flutter analyze` → No issues.

- [ ] **Step 5: Commit**

```bash
git add lib/ui/home/benchmark_config_section.dart unit_test/ui/backend_choice_test.dart
git commit -m "feat: backend dropdown per benchmark in config screen"
```

---

### Task 5: Mixed-backend result consumers

**Files:**
- Modify: `flutter/lib/data/result_filter.dart`
- Modify: `flutter/lib/ui/history/extended_result_screen.dart`
- Test: `flutter/unit_test/data/result_filter_test.dart`

**Interfaces:**
- Consumes: nothing new — pure consumers of `ExtendedResult`.

- [ ] **Step 1: Write the failing test**

In `flutter/unit_test/data/result_filter_test.dart`, read the existing fixtures first, then add a test that builds (or mutates a copy of) an `ExtendedResult` whose `results` contain two entries with different `backendInfo.filename` values, and assert:

```dart
    test('backend filter matches any result in a mixed-backend file', () {
      final filter = ResultFilter()..backend = secondBackendFilename;
      expect(filter.match(mixedBackendResult), isTrue);
    });
```

(Exact fixture construction must follow the patterns already in that test file — it already builds `ExtendedResult` objects for other filter tests.)

- [ ] **Step 2: Run test to verify it fails**

Run: `flutter test --no-pub unit_test/data/result_filter_test.dart -r compact`
Expected: FAIL — filter only checks `results.first`.

- [ ] **Step 3: Implement**

`flutter/lib/data/result_filter.dart`: delete the `String resultBackend = result.results.first.backendInfo.filename;` line and change:

```dart
    bool backendMatched = backend == null
        ? true
        : result.results.any((e) => e.backendInfo.filename == backend);
```

`flutter/lib/ui/history/extended_result_screen.dart` (`_makeBody`): change

```dart
    final backendName = firstResult.backendInfo.backendName;
```

to

```dart
    final backendName = res.results
        .map((e) => e.backendInfo.backendName)
        .toSet()
        .join(', ');
```

(If `firstResult` becomes unused, remove it.)

- [ ] **Step 4: Run tests**

Run: `flutter test --no-pub unit_test/* -r compact` → PASS. `flutter analyze` → No issues.

- [ ] **Step 5: Commit**

```bash
git add lib/data/result_filter.dart lib/ui/history/extended_result_screen.dart unit_test/data/result_filter_test.dart
git commit -m "feat: handle mixed-backend result files in filter and history"
```

---

### Task 6: Docs, formatting, full verification

**Files:**
- Modify: `docs/adding-custom-backend.md`

- [ ] **Step 1: Update the backend-ordering note**

In `docs/adding-custom-backend.md`, find the paragraph stating "Backends are evaluated in the order they are defined, and the app never checks backends after TFLite" and update it to describe the new behavior: all matching backends are offered; list order defines priority and the default; `libtflitebackend` is always probed last as the baseline; each benchmark defaults to the highest-priority backend that supports it and can be switched per benchmark in the app's config screen.

- [ ] **Step 2: Format and verify everything**

```bash
dart format lib unit_test test integration_test
flutter analyze
flutter test -r compact
flutter test --no-pub unit_test/* -r compact
```

All must pass with zero issues. `dart format` must report no changes needed on the second run.

- [ ] **Step 3: Commit**

```bash
git add -A
git commit -m "docs: describe per-benchmark backend selection"
```
