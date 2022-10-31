import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:mlperfbench/store.dart';
import 'package:mlperfbench_common/data/environment/environment_info.dart';
import 'package:mlperfbench_common/data/extended_result.dart';
import 'package:mlperfbench_common/data/results/benchmark_result.dart';
import 'package:shared_preferences/shared_preferences.dart';

import 'expected_accuracy.dart';
import 'expected_throughput.dart';
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
    prefs[StoreConstants.testCooldownDuration] = 10;
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
      checkThroughput(benchmarkResult, extendedResults.environmentInfo);
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

  final accelerator = benchmarkResult.backendSettings.acceleratorCode;
  final backendName = benchmarkResult.backendInfo.backendName;
  final expectedValue =
      expectedMap['$accelerator|$backendName'] ?? expectedMap[accelerator];
  final tag =
      '${benchmarkResult.benchmarkId} [accelerator: $accelerator | backendName: $backendName]';
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

void checkThroughput(
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

  final model = getDeviceModel(environmentInfo);
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

  final value = run.throughput;
  value!;
  expect(
    value,
    greaterThanOrEqualTo(expectedPerf.mean / expectedPerf.deviation),
    reason: 'performance for $taskId+$model+$backendTag is too low',
  );
  expect(
    value,
    lessThanOrEqualTo(expectedPerf.mean * expectedPerf.deviation),
    reason: 'performance for $taskId+$model+$backendTag is too high',
  );
}
