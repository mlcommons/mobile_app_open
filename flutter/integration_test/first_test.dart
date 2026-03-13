import 'dart:async';

import 'package:flutter/cupertino.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/benchmark/run_mode.dart';
import 'package:mlperfbench/store.dart';
import 'package:shared_preferences/shared_preferences.dart';

import 'utils.dart';

const _runMode = BenchmarkRunModeEnum.integrationTestRun;
const _bindingKeepAliveInterval = Duration(seconds: 20);

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
    testBenchmark(binding, benchmarkId);
  }
}

Future<T> runWithBindingKeepAlive<T>(
  IntegrationTestWidgetsFlutterBinding binding,
  String description,
  Future<T> Function() action,
) async {
  var keepAliveInFlight = false;

  Future<void> pingBinding() async {
    if (keepAliveInFlight) {
      return;
    }
    keepAliveInFlight = true;
    try {
      await binding.callback(<String, String>{'command': 'get_health'});
      debugPrint('IntegrationTest keepalive ping during $description');
    } catch (error) {
      debugPrint(
        'IntegrationTest keepalive failed during $description: $error',
      );
    } finally {
      keepAliveInFlight = false;
    }
  }

  final keepAliveTimer = Timer.periodic(_bindingKeepAliveInterval, (_) {
    unawaited(pingBinding());
  });

  try {
    return await action();
  } finally {
    keepAliveTimer.cancel();
  }
}

void testBenchmark(
  IntegrationTestWidgetsFlutterBinding binding,
  String benchmarkId,
) {
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
    await runWithBindingKeepAlive(
      binding,
      'benchmark run for $benchmarkId',
      () => runBenchmarks(tester),
    );
    final extendedResult = await getLastResult(tester);
    printResult(extendedResult);
    checkResult(extendedResult);
    await deleteResources(tester);
  });
}
