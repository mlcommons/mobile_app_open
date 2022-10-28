import 'dart:convert';

import 'package:flutter/widgets.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:mlperfbench/ui/root/main_screen/ready.dart';
import 'package:mlperfbench/ui/run/result_screen.dart';
import 'package:mlperfbench_common/data/extended_result.dart';
import 'package:mlperfbench/resources/result_manager.dart';
import 'package:mlperfbench/resources/resource_manager.dart'
    as resource_manager;
import 'package:mlperfbench/main.dart' as app;

Future<void> runBenchmark(WidgetTester tester) async {
  const splashPauseSeconds = 4;
  const runTimeLimitMinutes = 20;
  const downloadTimeLimitMinutes = 10;

  await app.main();
  await tester.pumpAndSettle(const Duration(seconds: splashPauseSeconds));

  var goButtonIsPresented = await waitFor(tester, downloadTimeLimitMinutes,
      const Key(MainScreenReadyKeys.goButton));

  expect(goButtonIsPresented, true,
      reason: 'Problems with downloading of datasets or models');
  final goButton = find.byKey(const Key(MainScreenReadyKeys.goButton));
  await tester.tap(goButton);

  var scrollButtonIsPresented = await waitFor(
      tester, runTimeLimitMinutes, const Key(ResultKeys.scrollResultsButton));

  expect(scrollButtonIsPresented, true, reason: 'Test results were not found');
}

Future<ExtendedResult> obtainResult() async {
  final applicationDirectory =
      await resource_manager.ResourceManager.getApplicationDirectory();

  final rm = await ResultManager.create(resultDir: applicationDirectory);
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
