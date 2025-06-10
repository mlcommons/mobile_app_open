import 'package:flutter/cupertino.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/benchmark/run_mode.dart';
import 'package:mlperfbench/store.dart';
import 'package:shared_preferences/shared_preferences.dart';

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
    testBenchmark(benchmarkId);
  }
}

void testBenchmark(String benchmarkId) {
  testWidgets('Test benchmark: $benchmarkId', (WidgetTester tester) async {
    await startApp(tester);
    await validateSettings(tester);
    if (!hasBenchmark(tester, benchmarkId)) {
      markTestSkipped('Backend does not support benchmark $benchmarkId');
      return;
    }
    if (!canRunBenchmark(tester, benchmarkId)) {
      markTestSkipped('Backend can not run benchmark $benchmarkId');
      return;
    }
    await setBenchmarks(tester, [benchmarkId]);
    debugPrint('Wait 5 seconds to let the app finishing loading resources');
    await Future.delayed(const Duration(seconds: 5));
    await downloadResources(tester);
    final cooldownDuration = _runMode.cooldownDuration;
    debugPrint('Wait $cooldownDuration seconds before running benchmark');
    await Future.delayed(Duration(seconds: cooldownDuration));
    await runBenchmarks(tester);
    final extendedResult = await getLastResult(tester);
    printResults(extendedResult);
    checkTasks(extendedResult);
    await deleteResources(tester);
  });
}
