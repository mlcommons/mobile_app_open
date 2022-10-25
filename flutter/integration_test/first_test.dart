import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:mlperfbench/store.dart';
import 'package:mlperfbench_common/data/environment/environment_info.dart';
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

  final environmentInfo = extendedResults.environmentInfo;

  for (final benchmarkResult in extendedResults.results) {
    print('checking ${benchmarkResult.benchmarkId}');
    expect(benchmarkResult.performanceRun, isNotNull);
    expect(benchmarkResult.performanceRun!.throughput, isNotNull);

    checkAccelerator(environmentInfo, benchmarkResult);
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

void checkAccelerator(
    EnvironmentInfo environmentInfo, BenchmarkExportResult benchmarkResult) {
  final modelCode = environmentInfo.value.android?.modelCode ??
      environmentInfo.value.ios?.modelCode;
  final backendName = benchmarkResult.backendInfo.backendName;
  final tag =
      '[benchmarkId: ${benchmarkResult.benchmarkId} | backendName: $backendName]';
  // Currently we only have Pixel 5 with TFLite backend as a physical test device
  // with physical accelerator in our CI.
  // iOS run in simulator.
  // Pixel 6 with TFLite-Pixel backend still does not implement the accelerator reporting.
  final supportedModelCodes = ['Pixel 5'];
  if (!supportedModelCodes.contains(modelCode)) {
    return;
  }
  final actual = benchmarkResult.backendInfo.acceleratorName;
  // target is acceleratorDesc because the backend currently reported the
  // same value in accelerator_desc instead of accelerator.
  // We may need to have accelerator as an enum instead of string to have reliable match.
  final target = benchmarkResult.backendSettings.acceleratorDesc;
  expect(actual, equals(target), reason: 'accelerator does not match for $tag');
}
