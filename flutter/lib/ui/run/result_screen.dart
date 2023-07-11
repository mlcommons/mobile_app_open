import 'package:flutter/material.dart';

import 'package:provider/provider.dart';
import 'package:quiver/iterables.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/store.dart';
import 'package:mlperfbench/ui/confirm_dialog.dart';
import 'package:mlperfbench/ui/error_dialog.dart';
import 'package:mlperfbench/ui/icons.dart' as app_icons;
import 'package:mlperfbench/ui/page_constraints.dart';
import 'package:mlperfbench/ui/root/main_screen.dart';
import 'package:mlperfbench/ui/run/app_drawer.dart';
import 'package:mlperfbench/ui/run/list_of_benchmark_items.dart';
import 'package:mlperfbench/ui/run/progress_screen.dart';
import 'package:mlperfbench/ui/run/result_circle.dart';
import 'package:mlperfbench/ui/run/share_button.dart';

enum _ScreenMode { performance, accuracy }

class ResultScreen extends StatefulWidget {
  const ResultScreen({Key? key}) : super(key: key);

  @override
  State<ResultScreen> createState() => _ResultScreenState();
}

class ResultKeys {
  // list of widget keys that need to be accessed in the test code
  static const String scrollResultsButton = 'scrollResultsButton';
}

class _ResultScreenState extends State<ResultScreen>
    with SingleTickerProviderStateMixin {
  late final TabController _tabController;
  _ScreenMode _screenMode = _ScreenMode.performance;

  @override
  void initState() {
    super.initState();

    _tabController = TabController(vsync: this, length: 2);
    _tabController.addListener(() {
      if (_tabController.indexIsChanging) {
        final tabIndex = _tabController.index;

        if (tabIndex == 0) {
          setState(() => _screenMode = _ScreenMode.performance);
        } else {
          setState(() => _screenMode = _ScreenMode.accuracy);
        }
      }
    });
  }

  @override
  void dispose() {
    _tabController.dispose();
    super.dispose();
  }

  Column _createListOfBenchmarkResultBottomWidgets(
      BuildContext context, BenchmarkState state) {
    final list = <Widget>[];
    final stringResources = AppLocalizations.of(context);
    final pictureEdgeSize = 0.08 * MediaQuery.of(context).size.width;

    for (final benchmark in state.benchmarks) {
      late final String? resultText;
      late final double? progressBarValue;
      late final String? resultText2;
      late final double? progressBarValue2;
      late final BenchmarkResult? benchmarkResult;
      late final bool resultIsValid;
      if (_screenMode == _ScreenMode.performance) {
        benchmarkResult = benchmark.performanceModeResult;
        final throughput = benchmarkResult?.throughput;
        resultText = throughput?.toUIString();
        progressBarValue =
            (throughput?.value ?? 0.0) / benchmark.info.maxThroughput;
        resultText2 = null;
        progressBarValue2 = null;
        resultIsValid = benchmarkResult?.validity ?? false;
      } else if (_screenMode == _ScreenMode.accuracy) {
        benchmarkResult = benchmark.accuracyModeResult;
        resultText = benchmarkResult?.accuracy?.formatted;
        progressBarValue = benchmarkResult?.accuracy?.normalized;
        resultText2 = benchmarkResult?.accuracy2?.formatted;
        progressBarValue2 = benchmarkResult?.accuracy2?.normalized;
        resultIsValid =
            (benchmarkResult?.accuracy?.normalized ?? -1.0) >= 0.0 &&
                (benchmarkResult?.accuracy?.normalized ?? -1.0) <= 1.0 &&
                (benchmarkResult?.accuracy2?.normalized ?? -1.0) <= 1.0;
      } else {
        continue;
      }
      final backendName = benchmark.performanceModeResult?.backendName ?? '';
      final acceleratorName =
          benchmark.performanceModeResult?.acceleratorName ?? '';

      var rowChildren = <Widget>[];
      rowChildren.add(Row(
        crossAxisAlignment: CrossAxisAlignment.start,
        mainAxisAlignment: MainAxisAlignment.spaceBetween,
        children: [
          Padding(
              padding: const EdgeInsets.only(bottom: 5),
              child: Text('$backendName | $acceleratorName')),
          Row(
            crossAxisAlignment: CrossAxisAlignment.center,
            children: [
              Column(
                children: [
                  Text(
                    resultText ?? 'N/A',
                    style: TextStyle(
                      color: resultIsValid
                          ? AppColors.darkText
                          : AppColors.darkRedText,
                      fontSize: 16.0,
                      fontWeight: FontWeight.bold,
                    ),
                  ),
                  if (resultText2 != null)
                    Text(
                      resultText2,
                      style: TextStyle(
                        color: resultIsValid
                            ? AppColors.darkText
                            : AppColors.darkRedText,
                        fontSize: 16.0,
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                ],
              ),
              const Icon(Icons.chevron_right, color: Colors.grey),
            ],
          ),
        ],
      ));
      if (benchmark.info.isOffline) {
        String batchSize;
        if (resultText == null) {
          batchSize = 'N/A';
        } else {
          batchSize = benchmarkResult?.batchSize.toString() ?? '';
        }

        rowChildren.add(Row(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text(
              stringResources.resultsBatchSize
                  .replaceAll('<batchSize>', batchSize),
              style: const TextStyle(
                color: Colors.grey,
                fontSize: 14.0,
              ),
            ),
          ],
        ));
      }
      rowChildren.add(FractionallySizedBox(
          widthFactor: 0.9, child: BlueProgressLine(progressBarValue ?? 0.0)));
      if (progressBarValue2 != null) {
        rowChildren.add(FractionallySizedBox(
            widthFactor: 0.9, child: BlueProgressLine(progressBarValue2)));
      }
      list.add(
        Column(
          children: [
            ListTile(
              minVerticalPadding: 0,
              leading: SizedBox(
                  width: pictureEdgeSize,
                  height: pictureEdgeSize,
                  child: benchmark.info.icon),
              title: Padding(
                padding: const EdgeInsets.fromLTRB(0, 5, 0, 5),
                child: Text(benchmark.taskConfig.name),
              ),
              subtitle: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: rowChildren,
              ),
              onTap: () => showBenchmarkInfoBottomSheet(context, benchmark),
            ),
            const Divider()
          ],
        ),
      );
    }
    return Column(children: list);
  }

  List<Widget> _createListOfBenchmarkResultTopWidgets(
      BenchmarkState state, BuildContext context) {
    final widgets = state.benchmarks.map((benchmark) {
      final result = _screenMode == _ScreenMode.performance
          ? benchmark.performanceModeResult
          : benchmark.accuracyModeResult;
      final text = _screenMode == _ScreenMode.performance
          ? result?.throughput?.toUIString()
          : result?.accuracy?.toUIString();
      final text2 = _screenMode == _ScreenMode.performance
          ? null
          : result?.accuracy2?.toUIString();
      final resultIsValid = _screenMode == _ScreenMode.performance
          ? (result?.validity ?? false)
          : ((result?.accuracy?.normalized ?? -1.0) >= 0.0 &&
              (result?.accuracy?.normalized ?? -1.0) <= 1.0 &&
              (result?.accuracy2?.normalized ?? -1.0) <= 1.0);
      return Padding(
        padding: const EdgeInsets.only(bottom: 10),
        child:
            Column(mainAxisAlignment: MainAxisAlignment.spaceEvenly, children: [
          Padding(
            padding: const EdgeInsets.only(bottom: 8),
            child: Text(
              benchmark.taskConfig.name.split(' ').join('\n').toUpperCase(),
              textAlign: TextAlign.center,
              style:
                  const TextStyle(fontSize: 12.0, color: AppColors.lightText),
            ),
          ),
          Text(
            text ?? 'N/A',
            style: TextStyle(
                fontSize: 32.0,
                color: resultIsValid
                    ? AppColors.lightText
                    : AppColors.lightRedText,
                fontWeight: FontWeight.bold),
          ),
          if (text2 != null)
            Text(
              text2,
              style: TextStyle(
                  fontSize: 32.0,
                  color: resultIsValid
                      ? AppColors.lightText
                      : AppColors.lightRedText,
                  fontWeight: FontWeight.bold),
            ),
        ]),
      );
    });

    return partition(widgets, 3)
        .map((row) => Row(
              mainAxisAlignment: MainAxisAlignment.spaceEvenly,
              children: row,
            ))
        .toList();
  }

  @override
  Widget build(BuildContext context) {
    final state = context.watch<BenchmarkState>();
    final store = context.watch<Store>();
    final l10n = AppLocalizations.of(context);
    final scrollController = ScrollController();

    final resultsPage = Column(
      children: [
        Expanded(
          child: CustomPaint(
            painter: MyPaintBottom(),
            child: Column(
              children: <Widget>[
                SizedBox(
                  width: MediaQuery.of(context).size.width * 0.75,
                  child: TabBar(
                    controller: _tabController,
                    indicator: const UnderlineTabIndicator(),
                    indicatorSize: TabBarIndicatorSize.label,
                    tabs: [
                      Tab(text: l10n.resultsTabTitlePerformance),
                      Tab(text: l10n.resultsTabTitleAccuracy),
                    ],
                  ),
                ),
                Expanded(
                  child: ResultCircle(state.result),
                ),
                Expanded(
                  child: Column(
                    mainAxisAlignment: MainAxisAlignment.center,
                    children:
                        _createListOfBenchmarkResultTopWidgets(state, context),
                  ),
                ),
              ],
            ),
          ),
        ),
        Padding(
          padding: const EdgeInsets.fromLTRB(30, 20, 30, 20),
          child: Row(
            mainAxisAlignment: MainAxisAlignment.spaceBetween,
            children: [
              Text(
                l10n.resultsTitleDetails,
                textAlign: TextAlign.left,
                style: const TextStyle(
                    fontSize: 17.0, fontWeight: FontWeight.bold),
              ),
              IconButton(
                  key: const Key(ResultKeys.scrollResultsButton),
                  icon: app_icons.AppIcons.arrow,
                  onPressed: () {
                    scrollController.animateTo(
                        scrollController.position.maxScrollExtent,
                        duration: const Duration(milliseconds: 200),
                        curve: Curves.ease);
                  })
            ],
          ),
        ),
      ],
    );

    final minimumShareButtonWidth = MediaQuery.of(context).size.width - 40;
    final buttonStyle = ButtonStyle(
        backgroundColor:
            MaterialStateProperty.all<Color>(AppColors.runBenchmarkRectangle),
        shape: MaterialStateProperty.all(RoundedRectangleBorder(
            borderRadius: BorderRadius.circular(14.0),
            side: const BorderSide(color: Colors.white))),
        minimumSize:
            MaterialStateProperty.all<Size>(Size(minimumShareButtonWidth, 0)));

    final detailedResultsPage = Column(children: [
      _createListOfBenchmarkResultBottomWidgets(context, state),
      Padding(
          padding: const EdgeInsets.fromLTRB(10, 10, 10, 10),
          child: TextButton(
            style: buttonStyle,
            onPressed: () async {
              // TODO (anhappdev) Refactor the code here to avoid duplicated code.
              // The checks before calling state.runBenchmarks() in main_screen and result_screen are similar.
              final wrongPathError = await state.validator
                  .validateExternalResourcesDirectory(
                      l10n.dialogContentMissingFiles);
              if (wrongPathError.isNotEmpty) {
                if (!mounted) return;
                await showErrorDialog(context, [wrongPathError]);
                return;
              }
              if (store.offlineMode) {
                final offlineError = await state.validator
                    .validateOfflineMode(l10n.dialogContentOfflineWarning);
                if (offlineError.isNotEmpty) {
                  if (!mounted) return;
                  switch (await showConfirmDialog(context, offlineError)) {
                    case ConfirmDialogAction.ok:
                      break;
                    case ConfirmDialogAction.cancel:
                      return;
                    default:
                      break;
                  }
                }
              }
              try {
                await state.runBenchmarks();
              } catch (e, t) {
                print(t);
                // current context may no longer be valid if runBenchmarks requested progress screen
                await showErrorDialog(
                    ProgressScreen.scaffoldKey.currentContext ?? context,
                    ['${l10n.runFail}:', e.toString()]);
                return;
              }
            },
            child: Padding(
              padding: const EdgeInsets.fromLTRB(0, 10, 0, 10),
              child: Text(
                l10n.resultsButtonTestAgain,
                style: const TextStyle(
                  fontSize: 20.0,
                  color: AppColors.lightText,
                ),
              ),
            ),
          )),
      const Padding(
        padding: EdgeInsets.fromLTRB(10, 10, 10, 10),
        child: ShareButton(),
      ),
      const SizedBox(height: 20)
    ]);

    String title;
    title = _screenMode == _ScreenMode.performance
        ? l10n.resultsTitlePerformance
        : l10n.resultsTitleAccuracy;
    title = isOfficialBuild ? title : '${l10n.resultsTitleUnverified} $title';

    return Scaffold(
      appBar: AppBar(title: Text(title)),
      drawer: const AppDrawer(),
      body: LayoutBuilder(
        builder: (context, constraint) {
          return SingleChildScrollView(
            physics: const ClampingScrollPhysics(),
            controller: scrollController,
            child: Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: <Widget>[
                getPageWidget(constraint, resultsPage),
                getPageWidget(constraint, detailedResultsPage)
              ],
            ),
          );
        },
      ),
    );
  }
}

class BlueProgressLine extends Container {
  final double _progress;

  BlueProgressLine(this._progress, {Key? key}) : super(key: key);

  double get _progressValue {
    final rangedProgress = _progress.clamp(0, 1);
    const startOffset = 0.01;

    return startOffset + (1 - startOffset) * rangedProgress;
  }

  @override
  Widget build(BuildContext context) {
    return FractionallySizedBox(
      alignment: Alignment.topLeft,
      widthFactor: _progressValue,
      child: Container(
        alignment: Alignment.topLeft,
        margin: const EdgeInsets.only(bottom: 10.0),
        height: 10,
        decoration: BoxDecoration(
          borderRadius: BorderRadius.circular(5.0),
          gradient: LinearGradient(
            colors: AppColors.resultBarGradient,
            begin: Alignment.topLeft,
            end: Alignment(1 / _progressValue, 0),
            stops: const [0, 0.36, 0.61, 0.83, 1.0],
          ),
        ),
      ),
    );
  }
}
