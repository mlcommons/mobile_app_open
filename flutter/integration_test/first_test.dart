import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:shared_preferences/shared_preferences.dart';

import 'package:mlperfbench/main.dart' as app;
import 'package:mlperfbench/ui/root/main_screen.dart';
import 'package:mlperfbench/ui/run/result_screen.dart';
import 'package:mlperfbench/resources/resource_manager.dart'
    as resource_manager;
import 'package:mlperfbench/resources/result_manager.dart' as result_manager;

import 'expected_accuracy.dart';

void main() {
  const splashPauseSeconds = 4;
  const runTimeLimitMinutes = 20;
  const downloadTimeLimitMinutes = 10;
  const secondsInMinute = 60;
  const expectedResultCount = 5; // number of tasks to be run

  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  SharedPreferences.setMockInitialValues({
    'test mode': true,
    'submission mode': true,
  });

  group('integration tests', () {
    final binding = IntegrationTestWidgetsFlutterBinding.ensureInitialized()
        as IntegrationTestWidgetsFlutterBinding;

    binding.framePolicy = LiveTestWidgetsFlutterBindingFramePolicy.fullyLive;

    testWidgets('run all and check test', (WidgetTester tester) async {
      Future<bool> waitFor(int timeLimitMinutes, Key key) async {
        var element = false;

        for (var counter = 0;
            counter < timeLimitMinutes * secondsInMinute;
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

      await app.main();
      await tester.pumpAndSettle(const Duration(seconds: splashPauseSeconds));

      var goButtonIsPresented =
          await waitFor(downloadTimeLimitMinutes, const Key(MainKeys.goButton));

      expect(goButtonIsPresented, true,
          reason: 'Problems with downloading of datasets or models');
      final goButton = find.byKey(const Key(MainKeys.goButton));
      await tester.tap(goButton);

      var scrollButtonIsPresented = await waitFor(
          runTimeLimitMinutes, const Key(ResultKeys.scrollResultsButton));

      expect(scrollButtonIsPresented, true,
          reason: 'Test results were not found');

      final applicationDirectory =
          await resource_manager.ResourceManager.getApplicationDirectory();

      final rm = result_manager.ResultManager(applicationDirectory);
      await rm.init();

      final extendedResults = rm.getLastResult();

      final length = extendedResults.results.list.length;

      expect(length, expectedResultCount,
          reason:
              'results count should be $expectedResultCount, but it is $length');

      for (final benchmarkResult in extendedResults.results.list) {
        expect(benchmarkResult.performance, isNotNull);
        expect(benchmarkResult.performance!.throughput, isNotNull);
        expect(benchmarkResult.accuracy, isNotNull);
        expect(benchmarkResult.accuracy!.accuracy, isNotNull);

        final expectedAccuracyMap =
            benchmarkExpectedAccuracy[benchmarkResult.benchmarkId];
        expect(
          expectedAccuracyMap,
          isNotNull,
          reason:
              'missing expected expected accuracy map for ${benchmarkResult.benchmarkId}',
        );
        expectedAccuracyMap!;

        // backends are not forced to follow backendSettingsInfo values
        // we should use backendInfo.accelerator, it always represents real accelerator value
        var accelerator = benchmarkResult.backendInfo.accelerator;
        if (accelerator == 'ACCELERATOR_NAME') {
          // some backends are yet to implement accelerator reporting
          print('warning: accelerator missing, using acceleratorDesc');
          accelerator = benchmarkResult.backendSettingsInfo.acceleratorDesc;
        }
        final expectedAccuracy = expectedAccuracyMap[
                '$accelerator+${benchmarkResult.backendInfo.name}'] ??
            expectedAccuracyMap[accelerator];
        final accuracyTag = '${benchmarkResult.benchmarkId}[$accelerator]';
        expect(
          expectedAccuracy,
          isNotNull,
          reason:
              'missing expected accuracy for $accuracyTag (+${benchmarkResult.backendInfo.name})',
        );
        expectedAccuracy!;

        expect(
          benchmarkResult.accuracy!.accuracy!.normalized,
          greaterThanOrEqualTo(expectedAccuracy.min),
          reason: 'accuracy for $accuracyTag is too low',
        );
        expect(
          benchmarkResult.accuracy!.accuracy!.normalized,
          lessThanOrEqualTo(expectedAccuracy.max),
          reason: 'accuracy for $accuracyTag is too high',
        );
      }
    });
  });
}
