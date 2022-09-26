import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:mlperfbench_common/data/extended_result.dart';
import 'package:mlperfbench_common/data/results/benchmark_result.dart';
import 'package:shared_preferences/shared_preferences.dart';

import 'expected_accuracy.dart';
import 'utils.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  SharedPreferences.setMockInitialValues({
    'test mode': true,
    'submission mode': true,
  });

  group('integration tests', () {
    final binding = IntegrationTestWidgetsFlutterBinding.ensureInitialized()
        as IntegrationTestWidgetsFlutterBinding;

    binding.framePolicy = LiveTestWidgetsFlutterBindingFramePolicy.fullyLive;

    testWidgets('run all and check', (WidgetTester tester) async {
      await runBenchmark(tester);
      final extendedResults = await obtainResult();
      printResults(extendedResults);
      checkTasks(extendedResults);
    });
  });
}

void checkTasks(ExtendedResult extendedResults) {
  final length = extendedResults.results.list.length;
  const expectedTasksCount = 5;

  expect(length, expectedTasksCount, reason: 'tasks count does not match');

  for (final benchmarkResult in extendedResults.results.list) {
    print('checking ${benchmarkResult.benchmarkId}');
    expect(benchmarkResult.performance, isNotNull);
    expect(benchmarkResult.performance!.throughput, isNotNull);

    checkAccuracy(benchmarkResult);
  }
}

void checkAccuracy(BenchmarkExportResult benchmarkResult) {
  final expectedMap = benchmarkExpectedAccuracy[benchmarkResult.benchmarkId];
  expect(
    expectedMap,
    isNotNull,
    reason: 'missing expected accuracy map for ${benchmarkResult.benchmarkId}',
  );
  expectedMap!;

  // backends are not forced to follow backendSettingsInfo values
  // we should use backendInfo.accelerator, it always represents real accelerator value
  var accelerator = benchmarkResult.backendInfo.accelerator;
  if (accelerator == 'ACCELERATOR_NAME') {
    // some backends are yet to implement accelerator reporting
    print('warning: accelerator missing, using acceleratorDesc');
    accelerator = benchmarkResult.backendSettingsInfo.acceleratorDesc;
  }
  final expectedValue =
      expectedMap['$accelerator+${benchmarkResult.backendInfo.name}'] ??
          expectedMap[accelerator];
  final tag =
      '${benchmarkResult.benchmarkId}[$accelerator] (+${benchmarkResult.backendInfo.name})';
  expect(
    expectedValue,
    isNotNull,
    reason: 'missing expected accuracy for $tag',
  );
  expectedValue!;

  final accuracyRun = benchmarkResult.accuracy;
  accuracyRun!;

  final accuracyValue = accuracyRun.accuracy;
  accuracyValue!;
  expect(
    accuracyValue.normalized,
    greaterThanOrEqualTo(expectedValue.min),
    reason: 'accuracy for $tag is too low',
  );
  expect(
    accuracyValue.normalized,
    lessThanOrEqualTo(expectedValue.max),
    reason: 'accuracy for $tag is too high',
  );
}
