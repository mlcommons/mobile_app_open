import 'dart:convert';

import 'package:collection/collection.dart';
import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:mlperfbench/app_constants.dart';
import 'package:provider/provider.dart';
import 'package:mlperfbench/firebase/firebase_manager.dart';
import 'package:mlperfbench/firebase/firebase_options.gen.dart';
import 'package:mlperfbench/data/environment/environment_info.dart';
import 'package:mlperfbench/data/extended_result.dart';
import 'package:mlperfbench/data/results/benchmark_result.dart';
import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/device_info.dart';
import 'package:mlperfbench/main.dart' as app;

import 'expected_accuracy.dart';
import 'expected_throughput.dart';

class Interval {
  final double min;
  final double max;

  const Interval({required this.min, required this.max});

  @override
  toString() {
    return '[$min, $max]';
  }
}

Future<void> startApp(WidgetTester tester) async {
  const splashPauseSeconds = 4;
  await app.main();
  await tester.pumpAndSettle(const Duration(seconds: splashPauseSeconds));
}

Future<void> validateSettings(WidgetTester tester) async {
  final state = tester.state(find.byType(MaterialApp));
  final benchmarkState = state.context.read<BenchmarkState>();
  for (var benchmark in benchmarkState.allBenchmarks) {
    expect(
      benchmark.selectedDelegate.batchSize,
      greaterThanOrEqualTo(0),
      reason: 'batchSize must >= 0',
    );
    for (var modelFile in benchmark.selectedDelegate.modelFile) {
      expect(
        modelFile.modelPath.isNotEmpty,
        isTrue,
        reason: 'modelPath cannot be empty',
      );
      expect(
        modelFile.modelChecksum.isNotEmpty,
        isTrue,
        reason: 'modelChecksum cannot be empty',
      );
    }
    expect(
      benchmark.selectedDelegate.acceleratorName.isNotEmpty,
      isTrue,
      reason: 'acceleratorName cannot be empty',
    );
    expect(
      benchmark.selectedDelegate.acceleratorDesc.isNotEmpty,
      isTrue,
      reason: 'acceleratorDesc cannot be empty',
    );
    expect(
      benchmark.benchmarkSettings.framework.isNotEmpty,
      isTrue,
      reason: 'framework cannot be empty',
    );
    expect(
      benchmark.selectedDelegate.delegateName,
      equals(benchmark.benchmarkSettings.delegateSelected),
      reason: 'delegateSelected must be the same as delegateName',
    );

    final selected = benchmark.benchmarkSettings.delegateSelected;
    final choices = benchmark.benchmarkSettings.delegateChoice
        .map((e) => e.delegateName)
        .toList();
    expect(
      choices.isNotEmpty,
      isTrue,
      reason: 'There must be at least one delegate choice',
    );
    expect(
      choices.contains(selected),
      isTrue,
      reason:
          'delegate_selected=$selected must be one of delegate_choice=$choices',
    );
  }
}

// Devices where the test pins every benchmark to one backend (when that
// backend is a candidate for the benchmark). The app's default selection
// follows the backend priority order, so without this the LiteRT vision
// runs would never execute in CI: the pixel backend outranks litert on
// Pixel devices.
/// The backend the Pixel 10 Pro CI jobs pin every benchmark to. Two jobs run
/// on that device — LiteRT (the default) and TFLite — so the two backends can
/// be compared on the same hardware. The backend is chosen at test-APK build
/// time via --dart-define=P10P_BACKEND (see android.mk): the BrowserStack
/// Flutter API offers no runtime channel into the test, so each job downloads
/// the unified APK variant built with its define.
const _pixel10ProBackend = String.fromEnvironment(
  'P10P_BACKEND',
  defaultValue: 'litert',
);

final deviceBackendOverride = <String, String>{
  'Pixel 10 Pro': _pixel10ProBackend == 'tflite'
      ? BackendId.tflite
      : BackendId.litert,
};

void applyDeviceBackendOverride(WidgetTester tester) {
  final deviceModel = getDeviceModel(DeviceInfo.instance.envInfo);
  final backendLib = deviceBackendOverride[deviceModel];
  if (backendLib == null) return;
  final state = tester.state(find.byType(MaterialApp));
  final benchmarkState = state.context.read<BenchmarkState>();
  for (final benchmark in benchmarkState.allBenchmarks) {
    if (benchmark.selectBackend(backendLib)) {
      debugPrint(
        'Backend override on $deviceModel: '
        '${benchmark.id} runs on $backendLib',
      );
    } else {
      debugPrint(
        'Backend override on $deviceModel: '
        '$backendLib is not a candidate for ${benchmark.id}',
      );
    }
  }
}

bool hasBenchmark(WidgetTester tester, String benchmarkId) {
  final state = tester.state(find.byType(MaterialApp));
  final benchmarkState = state.context.read<BenchmarkState>();
  return benchmarkState.allBenchmarks.map((e) => e.id).contains(benchmarkId);
}

bool canRunBenchmark(WidgetTester tester, String benchmarkId) {
  final state = tester.state(find.byType(MaterialApp));
  final benchmarkState = state.context.read<BenchmarkState>();
  final benchmark = benchmarkState.allBenchmarks.firstWhereOrNull(
    (e) => e.id == benchmarkId,
  );
  if (benchmark == null) return false;
  final selectedLib = benchmark.selectedBackend.info.libName;
  if (benchmarkId == 'stable_diffusion') {
    return selectedLib == BackendId.qti;
  }
  final overrideLib =
      deviceBackendOverride[getDeviceModel(DeviceInfo.instance.envInfo)];
  // The TFLite fallback is now offered alongside vendor backends, so
  // benchmarks the vendor backend lacks appear defaulting to TFLite.
  // Skip them here to keep device-job coverage and runtime the same as
  // before per-benchmark backends: running the fallback-only benchmarks
  // (e.g. LLM on CPU) blows the CI job timeout on vendor devices. The
  // TFLite-pinned Pixel 10 Pro comparison job is the exception: there
  // TFLite is selected on purpose.
  final primaryLib = benchmarkState.matchedBackends.first.libName;
  if (primaryLib != BackendId.tflite &&
      selectedLib == BackendId.tflite &&
      overrideLib != BackendId.tflite) {
    return false;
  }
  // The LiteRT backend fills the LLM gap on every Android device, but a
  // vendor device job should run LLM only with the vendor's own support:
  // a LiteRT CPU result says nothing about the vendor stack. LiteRT's
  // on-device LLM coverage comes from the pixel and tflite jobs.
  const llmVendorLibs = [BackendId.qti, BackendId.samsung, BackendId.mediatek];
  if (benchmarkId.startsWith('llm') &&
      llmVendorLibs.contains(primaryLib) &&
      selectedLib != primaryLib) {
    return false;
  }
  // Same reasoning for the vision/NLP benchmarks LiteRT claims: on a
  // device job dedicated to another backend, a benchmark that defaults to
  // LiteRT is one the primary backend does not claim. Skip it to keep
  // that job's coverage and runtime unchanged. Jobs pinned to LiteRT via
  // deviceBackendOverride (and the litert-only APK, where LiteRT is
  // primary) still run everything on LiteRT.
  if (!benchmarkId.startsWith('llm') &&
      selectedLib == BackendId.litert &&
      primaryLib != BackendId.litert &&
      overrideLib != BackendId.litert) {
    return false;
  }
  return true;
}

Future<void> setBenchmarks(
  WidgetTester tester,
  List<String> activeBenchmarks,
) async {
  final state = tester.state(find.byType(MaterialApp));
  final benchmarkState = state.context.read<BenchmarkState>();
  for (var benchmark in benchmarkState.allBenchmarks) {
    if (activeBenchmarks.contains(benchmark.id)) {
      benchmark.isActive = true;
      debugPrint('Benchmark ${benchmark.id} is enabled');
    } else {
      benchmark.isActive = false;
      debugPrint('Benchmark ${benchmark.id} is disabled');
    }
  }
}

Future<void> downloadResources(WidgetTester tester) async {
  final state = tester.state(find.byType(MaterialApp));
  final benchmarkState = state.context.read<BenchmarkState>();

  // Resource downloads occasionally fail on CI devices with a transient
  // network error (e.g. "HttpException: Connection closed while receiving
  // data"). loadResources() catches such errors into resourceError instead of
  // throwing, so without a retry the test proceeds and later fails with a
  // misleading "Progress screen is not presented". Retry a few times, matching
  // how a real user recovers by tapping "retry" on the resource error screen.
  const maxAttempts = 3;
  for (var attempt = 1; attempt <= maxAttempts; attempt++) {
    await benchmarkState.loadResources(
      downloadMissing: true,
      benchmarks: benchmarkState.activeBenchmarks,
    );
    if (benchmarkState.resourceError == null) {
      return;
    }
    debugPrint(
      'downloadResources attempt $attempt/$maxAttempts failed: '
      '${benchmarkState.resourceError}',
    );
    if (attempt < maxAttempts) {
      await Future.delayed(const Duration(seconds: 5));
    }
  }
  expect(
    benchmarkState.resourceError,
    isNull,
    reason: 'Failed to download resources after $maxAttempts attempts',
  );
}

Future<void> deleteResources(WidgetTester tester) async {
  final state = tester.state(find.byType(MaterialApp));
  final benchmarkState = state.context.read<BenchmarkState>();
  await benchmarkState.clearCache();
}

Future<void> runBenchmarks(WidgetTester tester) async {
  const runBenchmarkTimeout = 60 * 60; // 60 minutes

  final goButton = find.byKey(const Key(WidgetKeys.goButton));
  final testAgainButton = find.byKey(const Key(WidgetKeys.testAgainButton));
  if (tester.any(goButton)) {
    await tester.tap(goButton);
  }
  if (tester.any(testAgainButton)) {
    await tester.tap(testAgainButton);
    await waitFor(tester, 5, const Key(WidgetKeys.goButton));
    final goButton = find.byKey(const Key(WidgetKeys.goButton));
    await tester.tap(goButton);
  }

  // The go button md5-validates every resource of the active benchmarks
  // before the run starts, covering all delegate choices of the selected
  // backend. For the LLM benchmarks that is two ~1.2 GB models (CPU and
  // GPU delegates), which takes 30-60s on slower devices, so the progress
  // screen can take a while to appear.
  const progressScreenTimeout = 5 * 60;
  var progressCircleIsPresented = await waitFor(
    tester,
    progressScreenTimeout,
    const Key(WidgetKeys.progressCircle),
  );
  expect(
    progressCircleIsPresented,
    true,
    reason: 'Progress screen is not presented',
  );

  var totalScoreIsPresented = await waitFor(
    tester,
    runBenchmarkTimeout,
    const Key(WidgetKeys.totalScoreCircle),
  );
  expect(totalScoreIsPresented, true, reason: 'Result screen is not presented');
}

Future<ExtendedResult> getLastResult(WidgetTester tester) async {
  final state = tester.state(find.byType(MaterialApp));
  final benchmarkState = state.context.read<BenchmarkState>();
  return benchmarkState.resourceManager.resultManager.getLastResult();
}

String getDeviceModel(EnvironmentInfo info) {
  switch (info.platform) {
    case EnvPlatform.android:
      final value = info.value.android!;
      return value.modelCode!;
    case EnvPlatform.ios:
      final value = info.value.ios!;
      return value.modelCode!;
    case EnvPlatform.windows:
      final value = info.value.windows!;
      return value.cpuFullName;
  }
}

Future<bool> waitFor(WidgetTester tester, int timeout, Key key) async {
  var element = false;

  for (var counter = 0; counter < timeout; counter++) {
    await tester.pump(const Duration(seconds: 1));
    final searchResult = find.byKey(key);

    if (tester.any(searchResult)) {
      element = true;
      break;
    }
  }

  return element;
}

void printResult(ExtendedResult extendedResult) {
  debugPrint('Benchmark result json:');
  for (final line in const JsonEncoder.withIndent(
    '  ',
  ).convert(extendedResult).split('\n')) {
    debugPrint(line);
  }
}

void checkResult(ExtendedResult extendedResult) {
  for (final benchmarkResult in extendedResult.results) {
    debugPrint('Checking ${benchmarkResult.benchmarkId}');
    expect(benchmarkResult.performanceRun, isNotNull);
    expect(benchmarkResult.performanceRun!.throughput, isNotNull);

    checkAccuracy(benchmarkResult);
    checkThroughput(benchmarkResult, extendedResult.environmentInfo);
  }
}

void checkAccuracy(BenchmarkExportResult benchmarkResult) {
  var tag = '[benchmarkId: ${benchmarkResult.benchmarkId}';
  final expectedMap = benchmarkExpectedAccuracy[benchmarkResult.benchmarkId];
  expect(
    expectedMap,
    isNotNull,
    reason: 'missing expected accuracy map for ${benchmarkResult.benchmarkId}',
  );
  expectedMap!;

  final accelerator = benchmarkResult.backendSettings.acceleratorCode;
  tag += ' | accelerator: $accelerator';
  final backendName = benchmarkResult.backendInfo.backendName;
  tag += ' | backendName: $backendName]';
  final expectedValue =
      expectedMap['$accelerator|$backendName'] ?? expectedMap[accelerator];
  tag += ' | expectedValue: $expectedValue';

  // Skip if there is no expectedValue
  if (expectedValue == null) {
    debugPrint('No expected accuracy value; skipping accuracy check.');
    return;
  }
  expect(
    expectedValue,
    isNotNull,
    reason: 'missing expected accuracy for $tag',
  );

  final accuracyRun = benchmarkResult.accuracyRun;
  accuracyRun!;

  final accuracy = accuracyRun.accuracy;
  accuracy!;
  // The native code computes accuracy in float32, so a score exactly at a
  // bound can convert to a double just outside it
  // (e.g. float32(0.82) == 0.8199999928...). Tolerate that representation
  // error when comparing against the bounds.
  const accuracyTolerance = 1e-6;
  expect(
    accuracy.normalized,
    greaterThanOrEqualTo(expectedValue.min - accuracyTolerance),
    reason: 'accuracy for $tag is too low',
  );
  expect(
    accuracy.normalized,
    lessThanOrEqualTo(expectedValue.max + accuracyTolerance),
    reason: 'accuracy for $tag is too high',
  );
}

void checkThroughput(
  BenchmarkExportResult benchmarkResult,
  EnvironmentInfo environmentInfo,
) {
  final benchmarkId = benchmarkResult.benchmarkId;
  var tag = 'benchmarkId: $benchmarkId';
  final expectedMap = benchmarkExpectedThroughput[benchmarkId];
  expect(
    expectedMap,
    isNotNull,
    reason: 'missing expected throughput map for [$tag]',
  );
  expectedMap!;

  final backendTag = benchmarkResult.backendInfo.filename;
  tag += ' | backendTag: $backendTag';
  final backendExpectedMap = expectedMap[backendTag];
  expect(
    backendExpectedMap,
    isNotNull,
    reason: 'missing expected throughput for [$tag]',
  );
  backendExpectedMap!;

  final deviceModel = getDeviceModel(environmentInfo);
  tag += ' | deviceModel: $deviceModel';
  final expectedValue = backendExpectedMap[deviceModel];
  tag += ' | expectedValue: $expectedValue';

  // Skip if there is no expectedValue
  if (expectedValue == null) {
    debugPrint('No expected throughput value; skipping throughput check.');
    return;
  }
  expect(
    expectedValue,
    isNotNull,
    reason: 'missing expected throughput for [$tag]',
  );

  final run = benchmarkResult.performanceRun;
  run!;

  final throughput = run.throughput;
  throughput!;
  expect(
    throughput.value,
    greaterThanOrEqualTo(expectedValue.min),
    reason: 'throughput for [$tag] is too low',
  );
  expect(
    throughput.value,
    lessThanOrEqualTo(expectedValue.max),
    reason: 'throughput for [$tag] is too high',
  );
}

Future<void> uploadResult(ExtendedResult result) async {
  if (FirebaseManager.enabled) {
    await FirebaseManager.instance.initialize();
    if (DefaultFirebaseOptions.ciUserEmail.isNotEmpty) {
      final user = await FirebaseManager.instance.signIn(
        email: DefaultFirebaseOptions.ciUserEmail,
        password: DefaultFirebaseOptions.ciUserPassword,
      );
      debugPrint('Signed in as CI user with email: ${user.email}');
    }
    if (!FirebaseManager.instance.isSignedIn) {
      await FirebaseManager.instance.signInAnonymously();
      debugPrint('Signed in anonymously.');
    }
    await FirebaseManager.instance.uploadResult(result);
  } else {
    debugPrint('Firebase is disabled, skipping upload');
  }
}
