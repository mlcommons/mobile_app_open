import 'package:flutter/cupertino.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/benchmark/run_mode.dart';
import 'package:mlperfbench/firebase/firebase_manager.dart';
import 'package:mlperfbench/firebase/firebase_options.gen.dart';
import 'package:mlperfbench/store.dart';
import 'package:mlperfbench/data/environment/environment_info.dart';
import 'package:mlperfbench/data/extended_result.dart';
import 'package:mlperfbench/data/results/benchmark_result.dart';
import 'package:shared_preferences/shared_preferences.dart';

import 'expected_accuracy.dart';
import 'expected_throughput.dart';
import 'utils.dart';

const _runMode = BenchmarkRunModeEnum.integrationTestRun;

void main() {
  final binding = IntegrationTestWidgetsFlutterBinding.ensureInitialized();
  binding.framePolicy = LiveTestWidgetsFlutterBindingFramePolicy.fullyLive;

  final prefs = <String, Object>{
    StoreConstants.selectedBenchmarkRunMode: _runMode.name,
    StoreConstants.cooldown: true,
    StoreConstants.cooldownDuration: _runMode.cooldownDuration,
  };
  SharedPreferences.setMockInitialValues(prefs);

  // Get benchmark IDs from environment variables
  const benchmarkIdsStr =
      String.fromEnvironment('BENCHMARK_IDS', defaultValue: '');

  var benchmarkIds = BenchmarkId.allIds;
  if (benchmarkIdsStr.isNotEmpty) {
    benchmarkIds = benchmarkIdsStr.split(',');
  }
  debugPrint('Benchmarks to test: $benchmarkIds');

  // Run each benchmark separately to avoid idle timout error on BrowserStack
  for (var benchmarkId in benchmarkIds) {
    testBenchmarks([benchmarkId]);
  }
}

void testBenchmarks(List<String> benchmarkIds) {
  group('integration tests for benchmarks: $benchmarkIds', () {
    testWidgets('run benchmarks', (WidgetTester tester) async {
      await startApp(tester);
      await validateSettings(tester);
      await setBenchmarks(tester, benchmarkIds);
      await downloadResources(tester);
      final cooldownDuration = _runMode.cooldownDuration;
      debugPrint('Wait $cooldownDuration seconds before running benchmarks');
      await Future.delayed(Duration(seconds: cooldownDuration));
      await runBenchmarks(tester);
    });

    testWidgets('check result', (WidgetTester tester) async {
      final extendedResult = await obtainResult();
      printResults(extendedResult);
      checkTasks(extendedResult);
    });

    testWidgets('upload result', (WidgetTester tester) async {
      final extendedResult = await obtainResult();
      await uploadResult(extendedResult);
    });

    testWidgets('clear result', (WidgetTester tester) async {
      await clearResult(tester);
    });
  });
}

void checkTasks(ExtendedResult extendedResult) {
  for (final benchmarkResult in extendedResult.results) {
    debugPrint('checking ${benchmarkResult.benchmarkId}');
    expect(benchmarkResult.performanceRun, isNotNull);
    expect(benchmarkResult.performanceRun!.throughput, isNotNull);

    checkAccuracy(benchmarkResult);
    checkThroughput(benchmarkResult, extendedResult.environmentInfo);
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

Future<void> uploadResult(ExtendedResult result) async {
  if (FirebaseManager.enabled) {
    await FirebaseManager.instance.initialize();
    if (DefaultFirebaseOptions.ciUserEmail.isNotEmpty) {
      final user = await FirebaseManager.instance.signIn(
        email: DefaultFirebaseOptions.ciUserEmail,
        password: DefaultFirebaseOptions.ciUserPassword,
      );
      debugPrint('Signed in as CI user with email: ${user.email}');
    }
    if (!FirebaseManager.instance.isSignedIn) {
      await FirebaseManager.instance.signInAnonymously();
      debugPrint('Signed in anonymously.');
    }
    await FirebaseManager.instance.uploadResult(result);
  } else {
    debugPrint('Firebase is disabled, skipping upload');
  }
}
