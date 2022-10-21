import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:mlperfbench/store.dart';
import 'package:mlperfbench_common/data/environment/environment_info.dart';
import 'package:mlperfbench_common/data/extended_result.dart';
import 'package:mlperfbench_common/data/results/benchmark_result.dart';
import 'package:shared_preferences/shared_preferences.dart';

import 'expected_accuracy.dart';
import 'expected_performance.dart';
import 'utils.dart';

const enablePerfTest = bool.fromEnvironment(
  'enable-perf-test',
  defaultValue: false,
);

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  final prefs = <String, Object>{
    StoreConstants.testMode: true,
    StoreConstants.submissionMode: true,
    StoreConstants.testMinDuration: 1,
    StoreConstants.testMinQueryCount: 4,
  };
  if (enablePerfTest) {
    prefs[StoreConstants.testMinDuration] = 15;
    prefs[StoreConstants.testMinQueryCount] = 64;
    prefs[StoreConstants.testCooldown] = 10;
  }
  SharedPreferences.setMockInitialValues(prefs);

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
    if (enablePerfTest) {
      checkPerformance(benchmarkResult, extendedResults.environmentInfo);
    }
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

void checkPerformance(
  BenchmarkExportResult benchmarkResult,
  EnvironmentInfo environmentInfo,
) {
  final taskId = benchmarkResult.benchmarkId;
  final expectedMap = taskExpectedPerformance[taskId];
  expect(
    expectedMap,
    isNotNull,
    reason: 'missing expected performance map for $taskId',
  );
  expectedMap!;

  final model = environmentInfo.modelCode;
  final deviceExpectedMap = expectedMap[model];
  expect(
    deviceExpectedMap,
    isNotNull,
    reason: 'missing expected performance for $taskId+$model',
  );
  deviceExpectedMap!;

  final backendTag = benchmarkResult.backendInfo.filename;
  final expectedPerf = deviceExpectedMap[backendTag];
  expect(
    expectedPerf,
    isNotNull,
    reason: 'missing expected performance for $taskId+$model+$backendTag',
  );
  expectedPerf!;

  final run = benchmarkResult.performanceRun;
  run!;

  final deviceExpectedInstability = expectedInstabilityMap[model];
  expect(
    deviceExpectedInstability,
    isNotNull,
    reason: 'missing expected instability for $model',
  );
  deviceExpectedInstability!;
  final expectedInstability = deviceExpectedInstability[backendTag];
  expect(
    expectedInstability,
    isNotNull,
    reason: 'missing expected instability for $model+$backendTag',
  );
  expectedInstability!;

  final value = run.throughput;
  value!;
  expect(
    value,
    greaterThanOrEqualTo(expectedPerf / expectedInstability),
    reason: 'performance for $taskId+$model+$backendTag is too low',
  );
  expect(
    value,
    lessThanOrEqualTo(expectedPerf * expectedInstability),
    reason: 'performance for $taskId+$model+$backendTag is too high',
  );
}
