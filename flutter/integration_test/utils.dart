import 'dart:convert';

import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:mlperfbench/app_constants.dart';
import 'package:provider/provider.dart';

import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/data/extended_result.dart';
import 'package:mlperfbench/resources/result_manager.dart' as result_manager;
import 'package:mlperfbench/resources/resource_manager.dart'
    as resource_manager;
import 'package:mlperfbench/main.dart' as app;

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

Future<void> clearResult(WidgetTester tester) async {
  final state = tester.state(find.byType(MaterialApp));
  final benchmarkState = state.context.read<BenchmarkState>();
  await benchmarkState.resourceManager.resultManager.deleteLastResult();
  await benchmarkState.resetBenchmarkState();
}

Future<ExtendedResult> obtainResult() async {
  final applicationDirectory =
      await resource_manager.ResourceManager.getApplicationDirectory();

  final rm = await result_manager.ResultManager.create(applicationDirectory);
  return rm.getLastResult();
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
  debugPrint('benchmark result json:');
  for (final line in const JsonEncoder.withIndent('  ')
      .convert(extendedResult)
      .split('\n')) {
    debugPrint(line);
  }
}
