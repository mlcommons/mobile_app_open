import 'dart:convert';

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
    expect(benchmark.selectedDelegate.batchSize, greaterThanOrEqualTo(0),
        reason: 'batchSize must >= 0');
    for (var modelFile in benchmark.selectedDelegate.modelFile) {
      expect(modelFile.modelPath.isNotEmpty, isTrue,
          reason: 'modelPath cannot be empty');
      expect(modelFile.modelChecksum.isNotEmpty, isTrue,
          reason: 'modelChecksum cannot be empty');
    }
    expect(benchmark.selectedDelegate.acceleratorName.isNotEmpty, isTrue,
        reason: 'acceleratorName cannot be empty');
    expect(benchmark.selectedDelegate.acceleratorDesc.isNotEmpty, isTrue,
        reason: 'acceleratorDesc cannot be empty');
    expect(benchmark.benchmarkSettings.framework.isNotEmpty, isTrue,
        reason: 'framework cannot be empty');
    expect(benchmark.selectedDelegate.delegateName,
        equals(benchmark.benchmarkSettings.delegateSelected),
        reason: 'delegateSelected must be the same as delegateName');

    final selected = benchmark.benchmarkSettings.delegateSelected;
    final choices = benchmark.benchmarkSettings.delegateChoice
        .map((e) => e.delegateName)
        .toList();
    expect(choices.isNotEmpty, isTrue,
        reason: 'There must be at least one delegate choice');
    expect(choices.contains(selected), isTrue,
        reason:
            'delegate_selected=$selected must be one of delegate_choice=$choices');
  }
}

bool hasBenchmark(WidgetTester tester, String benchmarkId) {
  final state = tester.state(find.byType(MaterialApp));
  final benchmarkState = state.context.read<BenchmarkState>();
  return benchmarkState.allBenchmarks.map((e) => e.id).contains(benchmarkId);
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
  await benchmarkState.loadResources(
    downloadMissing: true,
    benchmarks: benchmarkState.activeBenchmarks,
  );
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

  var progressCircleIsPresented =
      await waitFor(tester, 30, const Key(WidgetKeys.progressCircle));
  expect(progressCircleIsPresented, true,
      reason: 'Progress screen is not presented');

  var totalScoreIsPresented = await waitFor(
      tester, runBenchmarkTimeout, const Key(WidgetKeys.totalScoreCircle));
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
    default:
      throw 'unsupported platform ${info.platform}';
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

void printResults(ExtendedResult extendedResult) {
  debugPrint('Benchmark result json:');
  for (final line in const JsonEncoder.withIndent('  ')
      .convert(extendedResult)
      .split('\n')) {
    debugPrint(line);
  }
}

void checkTasks(ExtendedResult extendedResult) {
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
  expect(
    expectedValue,
    isNotNull,
    reason: 'missing expected accuracy for $tag',
  );
  expectedValue!;

  final accuracyRun = benchmarkResult.accuracyRun;
  accuracyRun!;

  final accuracy = accuracyRun.accuracy;
  accuracy!;
  expect(
    accuracy.normalized,
    greaterThanOrEqualTo(expectedValue.min),
    reason: 'accuracy for $tag is too low',
  );
  expect(
    accuracy.normalized,
    lessThanOrEqualTo(expectedValue.max),
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
  expect(
    expectedValue,
    isNotNull,
    reason: 'missing expected throughput for [$tag]',
  );
  expectedValue!;

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
