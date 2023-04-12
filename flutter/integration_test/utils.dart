import 'dart:convert';

import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:provider/provider.dart';

import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/ui/root/main_screen.dart';
import 'package:mlperfbench/ui/run/result_screen.dart';
import 'package:mlperfbench_common/data/extended_result.dart';
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
  for (var benchmark in benchmarkState.benchmarks) {
    final selected = benchmark.benchmarkSettings.delegateSelected;
    final choices = benchmark.benchmarkSettings.delegateChoice
        .map((e) => e.delegateName)
        .toList();
    expect(choices.isEmpty == selected.isEmpty, isTrue);
    if (choices.isNotEmpty) {
      final reason =
          'delegate_selected=$selected must be one of delegate_choice=$choices';
      expect(choices.contains(selected), isTrue, reason: reason);
    }
  }
}

Future<void> runBenchmarks(WidgetTester tester) async {
  const runTimeLimitMinutes = 30;
  const downloadTimeLimitMinutes = 20;

  var goButtonIsPresented = await waitFor(
      tester, downloadTimeLimitMinutes, const Key(MainKeys.goButton));

  expect(goButtonIsPresented, true,
      reason: 'Problems with downloading of datasets or models');
  final goButton = find.byKey(const Key(MainKeys.goButton));
  await tester.tap(goButton);

  var scrollButtonIsPresented = await waitFor(
      tester, runTimeLimitMinutes, const Key(ResultKeys.scrollResultsButton));

  expect(scrollButtonIsPresented, true, reason: 'Test results were not found');
}

Future<ExtendedResult> obtainResult() async {
  final applicationDirectory =
      await resource_manager.ResourceManager.getApplicationDirectory();

  final rm = await result_manager.ResultManager.create(applicationDirectory);
  return rm.getLastResult();
}

Future<bool> waitFor(WidgetTester tester, int timeLimitMinutes, Key key) async {
  var element = false;

  for (var counter = 0;
      counter < timeLimitMinutes * Duration.secondsPerMinute;
      counter++) {
    await tester.pumpAndSettle(const Duration(seconds: 1));
    final searchResult = find.byKey(key);

    if (tester.any(searchResult)) {
      element = true;
      break;
    }
  }

  return element;
}

void printResults(ExtendedResult extendedResults) {
  print('benchmark result json:');
  for (final line in const JsonEncoder.withIndent('  ')
      .convert(extendedResults)
      .split('\n')) {
    print(line);
  }
}
