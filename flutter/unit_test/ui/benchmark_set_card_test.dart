// The analyzer does not treat this non-standard test dir as a test context.
// ignore_for_file: invalid_use_of_visible_for_testing_member

import 'dart:io';
import 'dart:ui' as ui;

import 'package:flutter/material.dart';
import 'package:flutter/rendering.dart';
import 'package:flutter/services.dart';
import 'package:flutter_test/flutter_test.dart';

import 'package:mlperfbench/backend/list.dart';
import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/data/results/benchmark_result.dart';
import 'package:mlperfbench/protos/backend_setting.pb.dart' as pb;
import 'package:mlperfbench/protos/mlperf_task.pb.dart' as pb;
import 'package:mlperfbench/ui/home/benchmark_set_card.dart';
import 'package:mlperfbench/ui/home/benchmark_set_result_tile.dart';

/// Writes a PNG of each state to SCREENSHOT_DIR when that is set, so the design
/// can be reviewed without building the app for a device. Without it these are
/// ordinary widget tests.
String? get screenshotDir => Platform.environment['SCREENSHOT_DIR'];

// --- a config shaped like the shipped tasks.pbtxt --------------------------

pb.TaskSet llmSet() => pb.TaskSet(
  id: 'llm',
  name: 'LLM',
  optionSet: [
    pb.OptionSet(
      id: 'parameters',
      name: 'Parameters',
      opt: [
        pb.Option(id: '1b', name: '1B'),
        pb.Option(id: '3b', name: '3B'),
        pb.Option(id: '8b', name: '8B'),
      ],
    ),
    pb.OptionSet(
      id: 'dataset',
      name: 'Dataset',
      hidden: true,
      opt: [
        pb.Option(id: 'mmlu', name: 'MMLU', enabled: true),
        pb.Option(id: 'ifeval', name: 'IFEval', enabled: true),
      ],
    ),
  ],
);

pb.TaskSet imageClassificationSet() => pb.TaskSet(
  id: 'image_classification',
  name: 'Image Classification v2',
  optionSet: [
    pb.OptionSet(
      id: 'options',
      name: 'Options',
      opt: [
        pb.Option(id: 'offline', name: 'Offline', enabled: true),
        pb.Option(id: 'online', name: 'Online', enabled: true),
      ],
    ),
  ],
);

pb.TaskConfig task(String id, String name, List<String> requiredOption) =>
    pb.TaskConfig(
      id: id,
      name: name,
      taskSet: id.startsWith('llm') ? 'llm' : 'image_classification',
      requiredOption: requiredOption,
    );

/// A benchmark belonging to no set, the way stable_diffusion ships.
final looseTasks = [
  pb.TaskConfig(id: 'stable_diffusion', name: 'Stable Diffusion'),
];

final llmTasks = [
  task('llm-1b', 'LLM 1B', ['1b', 'mmlu']),
  task('llm-1b-instruct', 'LLM 1B (instruct)', ['1b', 'ifeval']),
  task('llm-3b', 'LLM 3B', ['3b', 'mmlu']),
  task('llm-3b-instruct', 'LLM 3B (instruct)', ['3b', 'ifeval']),
  task('llm-8b', 'LLM 8B', ['8b', 'mmlu']),
  task('llm-8b-instruct', 'LLM 8B (instruct)', ['8b', 'ifeval']),
];

final icTasks = [
  task('image_classification_v2', 'Image Classification v2', ['online']),
  task('image_classification_offline_v2', 'Image Classification v2 (Offline)', [
    'offline',
  ]),
];

pb.DelegateSetting delegate(String name) =>
    pb.DelegateSetting(delegateName: name, priority: 1);

BackendInfo backend(String libName, String framework, List<String> taskIds) =>
    BackendInfo.forTest(
      pb.BackendSetting(
        benchmarkSetting: [
          for (final id in taskIds)
            pb.BenchmarkSetting(
              benchmarkId: id,
              framework: framework,
              delegateChoice: [delegate('gpu'), delegate('cpu')],
              delegateSelected: 'gpu',
            ),
        ],
        fallbackPolicy: pb.FallbackPolicy.FALLBACK_COEXIST,
      ),
      libName,
    );

BenchmarkStore buildStore(Map<String, Map<String, bool>> selection) {
  final allTaskIds = [
    ...llmTasks.map((e) => e.id),
    ...icTasks.map((e) => e.id),
    ...looseTasks.map((e) => e.id),
  ];
  // 8B has no CoreML build, so its rows show a single backend and no picker.
  final coremlTaskIds = allTaskIds.where((id) => !id.contains('8b')).toList();
  return BenchmarkStore(
    appConfig: pb.MLPerfConfig(
      task: [...llmTasks, ...icTasks, ...looseTasks],
      taskSet: [llmSet(), imageClassificationSet()],
    ),
    backends: [
      backend('libtflitebackend', 'TFLite', allTaskIds),
      backend('libcoremlbackend', 'CoreML', coremlTaskIds),
    ],
    taskSelection: const {},
    taskSetSelection: selection,
  );
}

BenchmarkSet setNamed(BenchmarkStore store, String id) =>
    store.benchmarkSets.firstWhere((e) => e.config.id == id);

// --- harness ---------------------------------------------------------------

Future<void> loadFonts() async {
  // Without a real font the test renderer draws boxes, which makes the
  // screenshots useless for judging the design.
  final root =
      Platform.environment['FLUTTER_ROOT'] ??
      File(Platform.resolvedExecutable).parent.parent.parent.path;
  final fonts = {
    'Roboto': [
      '$root/bin/cache/artifacts/material_fonts/Roboto-Regular.ttf',
      '$root/bin/cache/artifacts/material_fonts/Roboto-Bold.ttf',
      '$root/bin/cache/artifacts/material_fonts/Roboto-Medium.ttf',
    ],
    'MaterialIcons': [
      '$root/bin/cache/artifacts/material_fonts/MaterialIcons-Regular.otf',
    ],
  };
  for (final entry in fonts.entries) {
    final loader = FontLoader(entry.key);
    var loaded = false;
    for (final path in entry.value) {
      final file = File(path);
      if (!file.existsSync()) continue;
      loader.addFont(
        Future.value(ByteData.view(file.readAsBytesSync().buffer)),
      );
      loaded = true;
    }
    if (loaded) await loader.load();
  }
}

Widget harness(Widget child) {
  return MaterialApp(
    locale: const Locale('en'),
    localizationsDelegates: AppLocalizations.localizationsDelegates,
    supportedLocales: AppLocalizations.supportedLocales,
    theme: ThemeData(fontFamily: 'Roboto', useMaterial3: true),
    home: Scaffold(
      backgroundColor: const Color(0xFFEEF2F6),
      body: SingleChildScrollView(
        child: Padding(
          padding: const EdgeInsets.all(12),
          child: RepaintBoundary(child: child),
        ),
      ),
    ),
  );
}

Future<void> shoot(WidgetTester tester, String name) async {
  final dir = screenshotDir;
  if (dir == null) return;
  Directory(dir).createSync(recursive: true);
  final boundary =
      tester.renderObject(find.byType(RepaintBoundary).first)
          as RenderRepaintBoundary;
  // flutter_test paints elevation as a solid black outline. Repaint with real
  // shadows for the capture, and restore the flag its invariant check expects.
  debugDisableShadows = false;
  final ui.Image? image;
  try {
    boundary.reassemble();
    await tester.pump();
    image = await tester.runAsync(() => boundary.toImage(pixelRatio: 3.0));
  } finally {
    debugDisableShadows = true;
  }
  final bytes = await tester.runAsync(
    () => image!.toByteData(format: ui.ImageByteFormat.png),
  );
  File('$dir/$name.png').writeAsBytesSync(bytes!.buffer.asUint8List());
}

BenchmarkSetCard card(BenchmarkSet set, {bool backendsOpen = false}) {
  return BenchmarkSetCard(
    benchmarkSet: set,
    backendsOpen: backendsOpen,
    onToggleBackends: () {},
    onInfoTap: () {},
    onDownloadTap: () {},
    resourcesExist: Future.value(true),
    onOptionChanged: (_, _) {},
    onBackendChanged: (_, _) {},
    onSetBackendChanged: (_) {},
    onDelegateChanged: (_, _) {},
  );
}

void main() {
  setUpAll(loadFonts);

  testWidgets('a set states how many benchmarks its options produce', (
    tester,
  ) async {
    tester.view.devicePixelRatio = 3.0;
    tester.view.physicalSize = const Size(390 * 3, 520 * 3);
    addTearDown(tester.view.reset);
    final store = buildStore({
      'llm': {'1b': true, '3b': false, '8b': false},
    });
    await tester.pumpWidget(harness(card(setNamed(store, 'llm'))));
    await tester.pumpAndSettle();

    // One parameter chip produces two benchmarks via the hidden dataset set.
    expect(find.text('Runs 2 of 6 benchmarks'), findsOneWidget);
    expect(find.text('LLM 1B'), findsOneWidget);
    expect(find.text('LLM 1B (instruct)'), findsOneWidget);
    // Chips carry the option name, not the raw id.
    expect(find.text('1B'), findsOneWidget);
    expect(find.text('1b'), findsNothing);

    await shoot(tester, '01-config-llm-1b');
  });

  testWidgets('a set with nothing selected says so', (tester) async {
    tester.view.devicePixelRatio = 3.0;
    tester.view.physicalSize = const Size(390 * 3, 400 * 3);
    addTearDown(tester.view.reset);
    final store = buildStore({
      'image_classification': {'offline': false, 'online': false},
    });
    await tester.pumpWidget(
      harness(card(setNamed(store, 'image_classification'))),
    );
    await tester.pumpAndSettle();

    expect(find.text('Runs nothing'), findsOneWidget);
    expect(
      find.text('Nothing selected — this set will not run'),
      findsOneWidget,
    );

    await shoot(tester, '02-config-nothing-selected');
  });

  testWidgets('the backends panel keeps a picker per benchmark', (
    tester,
  ) async {
    tester.view.devicePixelRatio = 3.0;
    tester.view.physicalSize = const Size(390 * 3, 1000 * 3);
    addTearDown(tester.view.reset);
    final store = buildStore({
      'llm': {'1b': true, '3b': false, '8b': false},
    });
    await tester.pumpWidget(
      harness(card(setNamed(store, 'llm'), backendsOpen: true)),
    );
    await tester.pumpAndSettle();

    expect(find.text('Apply to all'), findsOneWidget);
    // Six benchmarks, four of them offering a backend choice, plus the
    // set-wide control.
    expect(find.byType(DropdownButton<String>), findsNWidgets(11));
    expect(find.text('not selected'), findsNWidgets(4));

    await shoot(tester, '03-config-backends-open');
  });

  testWidgets('results name what ran and what was skipped', (tester) async {
    tester.view.devicePixelRatio = 3.0;
    tester.view.physicalSize = const Size(390 * 3, 420 * 3);
    addTearDown(tester.view.reset);
    final store = buildStore({
      'llm': {'1b': true, '3b': false, '8b': false},
    });
    final set = setNamed(store, 'llm');
    // Stand in for a run of the two selected benchmarks.
    for (final benchmark in set.activeBenchmarks) {
      benchmark.performanceModeResult = BenchmarkResult(
        throughput: Throughput(value: 21.4),
        accuracy: null,
        accuracy2: null,
        backendName: 'TFLite',
        acceleratorName: 'gpu',
        delegateName: 'gpu',
        batchSize: 1,
        loadgenInfo: null,
      );
    }
    await tester.pumpWidget(
      harness(
        Container(
          color: Colors.white,
          child: BenchmarkSetResultTile(
            benchmarkSet: set,
            isExpanded: true,
            onToggle: () {},
            // Stands in for the screen's own row, which this change leaves
            // alone.
            resultRowBuilder: (benchmark) => ListTile(
              dense: true,
              title: Text(benchmark.info.taskName),
              trailing: const Text('21.4 QPS'),
            ),
          ),
        ),
      ),
    );
    await tester.pumpAndSettle();

    expect(find.text('2 of 6 benchmarks ran'), findsOneWidget);
    expect(find.text('Not run'), findsNWidgets(4));

    await shoot(tester, '04-results-set');
  });
}
