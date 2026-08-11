// The analyzer does not treat this non-standard test dir as a test context.
// ignore_for_file: invalid_use_of_visible_for_testing_member

import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';

import 'package:mlperfbench/backend/list.dart';
import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/protos/backend_setting.pb.dart' as pb;
import 'package:mlperfbench/protos/mlperf_task.pb.dart' as task_pb;
import 'package:mlperfbench/ui/home/benchmark_config_section.dart';

Benchmark _makeBenchmark(List<(String, String)> backendsSpec) {
  // backendsSpec: list of (libName, framework)
  final backends = backendsSpec.map((spec) {
    final (libName, framework) = spec;
    final settings = pb.BenchmarkSetting(
      benchmarkId: 'task1',
      framework: framework,
    );
    return BenchmarkBackend(
      info: BackendInfo.forTest(
        pb.BackendSetting(benchmarkSetting: [settings]),
        libName,
      ),
      settings: settings,
    );
  }).toList();
  return Benchmark(
    backends: backends,
    taskConfig: task_pb.TaskConfig(id: 'task1'),
    isActive: true,
  );
}

Future<void> _pump(WidgetTester tester, Widget child) {
  return tester.pumpWidget(MaterialApp(home: Scaffold(body: child)));
}

void main() {
  testWidgets('single backend renders static text, no dropdown', (
    tester,
  ) async {
    final benchmark = _makeBenchmark([('libtflitebackend', 'TFLite')]);
    await _pump(tester, BackendChoice(benchmark: benchmark, onChanged: (_) {}));
    expect(find.text('TFLite'), findsOneWidget);
    expect(find.byType(DropdownButton<String>), findsNothing);
  });

  testWidgets('two backends render a dropdown; selection calls back', (
    tester,
  ) async {
    final benchmark = _makeBenchmark([
      ('libcoremlbackend', 'Core ML'),
      ('libtflitebackend', 'TFLite'),
    ]);
    String? selected;
    await _pump(
      tester,
      BackendChoice(
        benchmark: benchmark,
        onChanged: (value) => selected = value,
      ),
    );
    expect(find.byType(DropdownButton<String>), findsOneWidget);
    expect(find.text('Core ML'), findsOneWidget);

    await tester.tap(find.byType(DropdownButton<String>));
    await tester.pumpAndSettle();
    await tester.tap(find.text('TFLite').last);
    await tester.pumpAndSettle();
    expect(selected, 'libtflitebackend');
  });

  test('colliding framework labels are disambiguated by lib name', () {
    final benchmark = _makeBenchmark([
      ('libqtibackend', 'TFLite'),
      ('libtflitebackend', 'TFLite'),
    ]);
    final labels = backendChoiceLabels(benchmark);
    expect(labels['libqtibackend'], 'TFLite (qti)');
    expect(labels['libtflitebackend'], 'TFLite (tflite)');
  });

  test('distinct framework labels are used as-is', () {
    final benchmark = _makeBenchmark([
      ('libcoremlbackend', 'Core ML'),
      ('libtflitebackend', 'TFLite'),
    ]);
    final labels = backendChoiceLabels(benchmark);
    expect(labels['libcoremlbackend'], 'Core ML');
    expect(labels['libtflitebackend'], 'TFLite');
  });
}
