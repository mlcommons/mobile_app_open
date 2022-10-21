import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:mlperfbench/store.dart';
import 'package:mlperfbench_common/data/extended_result.dart';
import 'package:mlperfbench_common/data/results/benchmark_result.dart';
import 'package:shared_preferences/shared_preferences.dart';

import 'expected_accuracy.dart';
import 'utils.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  SharedPreferences.setMockInitialValues({
    StoreConstants.testMode: true,
    StoreConstants.submissionMode: true,
  });

  group('integration tests', () {
    final binding = IntegrationTestWidgetsFlutterBinding.ensureInitialized();

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
  final length = extendedResults.results.length;
  const expectedTasksCount = 5;

  expect(length, expectedTasksCount, reason: 'tasks count does not match');

  for (final benchmarkResult in extendedResults.results) {
    print('checking ${benchmarkResult.benchmarkId}');
    expect(benchmarkResult.performanceRun, isNotNull);
    expect(benchmarkResult.performanceRun!.throughput, isNotNull);

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
  var accelerator = benchmarkResult.backendInfo.acceleratorName;
  if (accelerator == 'ACCELERATOR_NAME') {
    // some backends are yet to implement accelerator reporting
    print('warning: accelerator missing, using acceleratorDesc');
    accelerator = benchmarkResult.backendSettings.acceleratorDesc;
  }
  final expectedValue =
      expectedMap['$accelerator+${benchmarkResult.backendInfo.backendName}'] ??
          expectedMap[accelerator];
  final tag =
      '${benchmarkResult.benchmarkId}[$accelerator] (+${benchmarkResult.backendInfo.backendName})';
  expect(
    expectedValue,
    isNotNull,
    reason: 'missing expected accuracy for $tag',
  );
  expectedValue!;

  final accuracyRun = benchmarkResult.accuracyRun;
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
