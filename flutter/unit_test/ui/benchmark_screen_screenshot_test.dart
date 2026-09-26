// The analyzer does not treat this non-standard test dir as a test context.
// ignore_for_file: invalid_use_of_visible_for_testing_member

import 'dart:io';
import 'dart:ui' as ui;

import 'package:flutter/material.dart';
import 'package:flutter/rendering.dart';
import 'package:flutter_test/flutter_test.dart';

import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/app_styles.dart';
import 'package:mlperfbench/ui/home/benchmark_config_section.dart';
import 'package:mlperfbench/ui/home/benchmark_loose_card.dart';
import 'package:mlperfbench/ui/home/benchmark_set_card.dart';

import 'benchmark_set_card_test.dart' show buildStore, loadFonts, screenshotDir;

/// The whole start screen at phone size.
///
/// BenchmarkState has a private constructor and late-initialised native
/// dependencies, so the real BenchmarkStartScreen cannot be pumped in a widget
/// test. The list here is the real BenchmarkConfigList holding the real cards;
/// only the chrome around it — app bar, GO section, info band — is reproduced,
/// with the values copied from benchmark_start_screen.dart.
class FakeStartScreen extends StatelessWidget {
  final BenchmarkStore store;
  final Set<String> backendsOpen;

  const FakeStartScreen({
    super.key,
    required this.store,
    this.backendsOpen = const {},
  });

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context)!;
    final width = MediaQuery.of(context).size.width;
    final circleWidth = width * WidgetSizes.circleWidthFactor;
    const double verticalPadding = 8.0;
    final sectionHeight = circleWidth + verticalPadding * 2.0;

    final active = store.activeBenchmarks.length.toString();
    final total = store.allBenchmarks.length.toString();

    return Scaffold(
      appBar: AppBar(
        title: Text(l10n.menuHome),
        backgroundColor: AppColors.secondaryAppBarBackground,
        leading: const Icon(Icons.menu, color: AppColors.appBarIcon),
      ),
      body: Column(
        children: [
          SizedBox(
            width: width,
            height: sectionHeight,
            child: Stack(
              alignment: Alignment.topCenter,
              children: [
                Container(
                  decoration: BoxDecoration(
                    gradient: LinearGradient(
                      begin: Alignment.topCenter,
                      end: Alignment.bottomCenter,
                      colors: AppGradients.halfScreen,
                    ),
                  ),
                ),
                Container(
                  alignment: Alignment.center,
                  child: ElevatedButton(
                    style: ElevatedButton.styleFrom(
                      backgroundColor: AppColors.goCircle,
                      shape: const CircleBorder(),
                      minimumSize: Size.fromWidth(circleWidth),
                    ),
                    onPressed: () {},
                    child: Text(
                      l10n.mainScreenGo,
                      style: const TextStyle(
                        color: AppColors.lightText,
                        fontSize: 40,
                      ),
                    ),
                  ),
                ),
              ],
            ),
          ),
          Container(
            padding: const EdgeInsets.fromLTRB(20, 4, 10, 4),
            width: double.infinity,
            color: AppColors.infoSectionBackground,
            child: DefaultTextStyle.merge(
              style: const TextStyle(color: Colors.white, fontSize: 14),
              child: Column(
                mainAxisSize: MainAxisSize.min,
                children: [
                  const Text('Pixel 9 Pro'),
                  Text(
                    l10n.mainScreenBenchmarkSelected
                        .replaceAll('<selected>', active)
                        .replaceAll('<total>', total),
                  ),
                ],
              ),
            ),
          ),
          Expanded(
            child: BenchmarkConfigList(
              cards: [
                for (final set in store.benchmarkSets)
                  if (set.benchmarks.isNotEmpty)
                    BenchmarkSetCard(
                      benchmarkSet: set,
                      backendsOpen: backendsOpen.contains(set.config.id),
                      onToggleBackends: () {},
                      onInfoTap: () {},
                      onDownloadTap: () {},
                      resourcesExist: Future.value(true),
                      onOptionChanged: (_, _) {},
                      onBackendChanged: (_, _) {},
                      onSetBackendChanged: (_) {},
                      onDelegateChanged: (_, _) {},
                    ),
                for (final benchmark in store.looseBenchmarks)
                  BenchmarkLooseCard(
                    benchmark: benchmark,
                    onInfoTap: () {},
                    onDownloadTap: () {},
                    resourcesExist: Future.value(true),
                    onActiveChanged: (_) {},
                    onBackendChanged: (_) {},
                    onDelegateChanged: (_) {},
                  ),
              ],
            ),
          ),
        ],
      ),
    );
  }
}

Future<void> shootScreen(WidgetTester tester, String name) async {
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

/// Mirrors the theme in lib/ui/root/app.dart, so the screenshot matches the
/// shipping app rather than a bare Material 3 default.
Widget phone(Widget child) {
  return MaterialApp(
    locale: const Locale('en'),
    localizationsDelegates: AppLocalizations.localizationsDelegates,
    supportedLocales: AppLocalizations.supportedLocales,
    theme: ThemeData(fontFamily: 'Roboto').copyWith(
      // ignore: deprecated_member_use
      useMaterial3: false,
      colorScheme: ColorScheme.fromSwatch().copyWith(
        primary: AppColors.primary,
        secondary: AppColors.secondary,
      ),
      appBarTheme: const AppBarTheme(
        centerTitle: true,
        // fontFamily is the harness's: an explicit titleTextStyle drops the
        // theme's family, and the test renderer has no default font.
        titleTextStyle: TextStyle(
          color: AppColors.lightText,
          fontSize: 20,
          fontFamily: 'Roboto',
        ),
        elevation: 0,
        backgroundColor: AppColors.primaryAppBarBackground,
        iconTheme: IconThemeData(color: AppColors.appBarIcon),
      ),
    ),
    home: RepaintBoundary(child: child),
  );
}

/// A 6.1" phone. Sets the view rather than the surface size: the latter leaves
/// MediaQuery reporting the test default, so anything measured from
/// MediaQuery — the GO circle here — lays out for the wrong screen.
void usePhoneScreen(WidgetTester tester) {
  const size = Size(390, 844);
  const ratio = 3.0;
  tester.view.devicePixelRatio = ratio;
  tester.view.physicalSize = size * ratio;
  addTearDown(tester.view.reset);
}

void main() {
  setUpAll(loadFonts);

  testWidgets('the whole screen at phone size', (tester) async {
    usePhoneScreen(tester);

    final store = buildStore({
      'llm': {'1b': true, '3b': false, '8b': false},
      'image_classification': {'offline': false, 'online': true},
    });
    await tester.pumpWidget(phone(FakeStartScreen(store: store)));
    await tester.pumpAndSettle();

    expect(find.text('Runs 2 of 6 benchmarks'), findsOneWidget);
    expect(find.text('Runs 1 of 2 benchmarks'), findsOneWidget);

    await shootScreen(tester, '10-screen-top');
  });

  testWidgets('the whole screen scrolled to the loose benchmarks', (
    tester,
  ) async {
    usePhoneScreen(tester);

    final store = buildStore({
      'llm': {'1b': true, '3b': false, '8b': false},
      'image_classification': {'offline': false, 'online': true},
    });
    await tester.pumpWidget(phone(FakeStartScreen(store: store)));
    await tester.pumpAndSettle();
    await tester.drag(find.byType(ListView), const Offset(0, -260));
    await tester.pumpAndSettle();

    await shootScreen(tester, '11-screen-scrolled');
  });

  testWidgets('the whole screen with the backends panel open', (tester) async {
    usePhoneScreen(tester);

    final store = buildStore({
      'llm': {'1b': true, '3b': false, '8b': false},
      'image_classification': {'offline': false, 'online': true},
    });
    await tester.pumpWidget(
      phone(FakeStartScreen(store: store, backendsOpen: const {'llm'})),
    );
    await tester.pumpAndSettle();
    await tester.drag(find.byType(ListView), const Offset(0, -200));
    await tester.pumpAndSettle();

    await shootScreen(tester, '12-screen-backends');
  });
}
