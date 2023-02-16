import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:mlperfbench/benchmark/run_mode.dart';
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
    StoreConstants.selectedBenchmarkRunMode:
        BenchmarkRunModeEnum.submissionRun.name,
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

    testWidgets('run benchmarks', (WidgetTester tester) async {
      await runBenchmarks(tester);
    });

    testWidgets('check results', (WidgetTester tester) async {
      final extendedResults = await obtainResult();
      printResults(extendedResults);
      checkTasks(extendedResults);
    });
  });
}

void checkTasks(ExtendedResult extendedResults) {
  final length = extendedResults.results.length;
  const expectedTasksCount = 6;

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
