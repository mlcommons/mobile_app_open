import 'dart:math';

import 'package:flutter/material.dart';

import 'package:collection/collection.dart';
import 'package:mlperfbench_common/data/results/benchmark_result.dart';
import 'package:mlperfbench_common/firebase/manager.dart';
import 'package:provider/provider.dart';
import 'package:quiver/iterables.dart';
import 'package:share_plus/share_plus.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/resources/utils.dart';
import 'package:mlperfbench/state/last_result_manager.dart';
import 'package:mlperfbench/state/task_list_manager.dart';
import 'package:mlperfbench/store.dart';
import 'package:mlperfbench/ui/confirm_dialog.dart';
import 'package:mlperfbench/ui/error_dialog.dart';
import 'package:mlperfbench/ui/icons.dart' as app_icons;
import 'package:mlperfbench/ui/page_constraints.dart';
import 'package:mlperfbench/ui/root/main_screen/utils.dart';
import 'package:mlperfbench/ui/run/app_bar.dart';
import 'package:mlperfbench/ui/run/list_of_benchmark_items.dart';
import 'package:mlperfbench/ui/run/result_circle.dart';
import 'progress_screen.dart';

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

  Widget _makeBottomTaskResultView({
    required double pictureSize,
    required AppLocalizations l10n,
    required Benchmark benchmark,
    required _ResultViewHelper taskHelper,
  }) {
    final icon = SizedBox(
      width: pictureSize,
      height: pictureSize,
      child: benchmark.info.icon,
    );
    final title = Padding(
      padding: const EdgeInsets.fromLTRB(0, 5, 0, 5),
      child: Text(taskHelper.taskName),
    );

    final backendInfo = Padding(
      padding: const EdgeInsets.only(bottom: 5),
      child: Text(taskHelper.backendInfoSubtitle),
    );

    final scoreText1 = Text(
      taskHelper.textResult ?? 'N/A',
      style: TextStyle(
        color: taskHelper.resultIsValid
            ? AppColors.darkText
            : AppColors.darkRedText,
        fontSize: 16.0,
        fontWeight: FontWeight.bold,
      ),
    );
    final scoreText2 = taskHelper.textResult2 == null
        ? null
        : Text(
            taskHelper.textResult2!,
            style: TextStyle(
              color: taskHelper.resultIsValid
                  ? AppColors.darkText
                  : AppColors.darkRedText,
              fontSize: 16.0,
              fontWeight: FontWeight.bold,
            ),
          );

    final scoreText = Row(
      crossAxisAlignment: CrossAxisAlignment.center,
      children: [
        Column(
          children: [
            scoreText1,
            if (scoreText2 != null) scoreText2,
          ],
        ),
        const Icon(Icons.chevron_right, color: Colors.grey),
      ],
    );

    final firstLine = Row(
      crossAxisAlignment: CrossAxisAlignment.start,
      mainAxisAlignment: MainAxisAlignment.spaceBetween,
      children: [
        backendInfo,
        scoreText,
      ],
    );
    Widget? batchInfo;
    if (benchmark.info.isOffline) {
      final batchSize = taskHelper.batchSize;

      batchInfo = Row(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text(
            l10n.resultsBatchSize.replaceAll('<batchSize>', batchSize),
            style: const TextStyle(
              color: Colors.grey,
              fontSize: 14.0,
            ),
          ),
        ],
      );
    }
    final scoreLine1 = FractionallySizedBox(
      widthFactor: 0.9,
      child: BlueProgressLine(taskHelper.numericResult ?? 0.0),
    );
    Widget? scoreLine2;
    if (taskHelper.numericResult2 != null) {
      scoreLine2 = FractionallySizedBox(
          widthFactor: 0.9,
          child: BlueProgressLine(taskHelper.numericResult2!));
    }

    final scoreView = Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        firstLine,
        if (batchInfo != null) batchInfo,
        scoreLine1,
        if (scoreLine2 != null) scoreLine2,
      ],
    );

    return Column(
      children: [
        ListTile(
          minVerticalPadding: 0,
          leading: icon,
          title: title,
          subtitle: scoreView,
          onTap: () => showBenchmarkInfoBottomSheet(context, benchmark),
        ),
        const Divider()
      ],
    );
  }

  Column _makeBottomResultsView(
    BuildContext context,
    List<Benchmark> tasks,
    List<BenchmarkExportResult> results,
  ) {
    final list = <Widget>[];
    final l10n = AppLocalizations.of(context);
    final pictureEdgeSize = 0.08 * MediaQuery.of(context).size.width;

    for (final benchmark in tasks) {
      final taskHelper = _makeTaskHelper(benchmark, results);
      list.add(_makeBottomTaskResultView(
        pictureSize: pictureEdgeSize,
        l10n: l10n,
        benchmark: benchmark,
        taskHelper: taskHelper,
      ));
    }
    return Column(children: list);
  }

  Widget _makeTopTaskResultView(_ResultViewHelper taskHelper) {
    final upperCaseName =
        taskHelper.taskName.split(' ').join('\n').toUpperCase();
    final title = Padding(
      padding: const EdgeInsets.only(bottom: 8),
      child: Text(
        upperCaseName,
        textAlign: TextAlign.center,
        style: const TextStyle(fontSize: 12.0, color: AppColors.lightText),
      ),
    );
    final scoreText1 = Text(
      taskHelper.textResult ?? 'N/A',
      style: TextStyle(
        fontSize: 32.0,
        color: taskHelper.resultIsValid
            ? AppColors.lightText
            : AppColors.lightRedText,
        fontWeight: FontWeight.bold,
      ),
    );
    final scoreText2 = taskHelper.textResult2 == null
        ? null
        : Text(
            taskHelper.textResult!,
            style: TextStyle(
              fontSize: 32.0,
              color: taskHelper.resultIsValid
                  ? AppColors.lightText
                  : AppColors.lightRedText,
              fontWeight: FontWeight.bold,
            ),
          );
    return Padding(
      padding: const EdgeInsets.only(bottom: 10),
      child: Column(
        mainAxisAlignment: MainAxisAlignment.spaceEvenly,
        children: [
          title,
          scoreText1,
          if (scoreText2 != null) scoreText2,
        ],
      ),
    );
  }

  List<Widget> _makeTopResultsView(
    BuildContext context,
    List<Benchmark> tasks,
    List<BenchmarkExportResult> results,
  ) {
    final widgets = tasks.map((benchmark) {
      final taskHelper = _makeTaskHelper(benchmark, results);
      return _makeTopTaskResultView(taskHelper);
    });

    return partition(widgets, 3)
        .map((row) => Row(
              mainAxisAlignment: MainAxisAlignment.spaceEvenly,
              children: row,
            ))
        .toList();
  }

  _ResultViewHelper _makeTaskHelper(
    Benchmark task,
    List<BenchmarkExportResult> results,
  ) {
    final exportResult = results
        .firstWhereOrNull((e) => e.benchmarkName == task.taskConfig.name);
    if (exportResult == null) {
      return _ResultViewHelper.empty(task);
    }
    switch (_screenMode) {
      case _ScreenMode.performance:
        return _ResultViewHelper.performance(task, exportResult);
      case _ScreenMode.accuracy:
        return _ResultViewHelper.accuracy(task, exportResult);
      default:
        throw 'unsupported _ScreenMode enum value';
    }
  }

  double geomean(List<double> values) {
    return pow(
      values.fold<double>(1.0, (prev, e) {
        return prev * e;
      }),
      1.0 / values.length,
    ).toDouble();
  }

  double calculateOverallPercentage(
      List<Benchmark> tasks, List<BenchmarkExportResult> results) {
    final perfResults =
        results.where((e) => e.performanceRun?.throughput != null).toList();

    if (perfResults.isEmpty) return 0;

    // TODO maybe geometric mean is not a good choice here?
    final geomeanThroughput =
        geomean(perfResults.map((e) => e.performanceRun!.throughput!).toList());
    final geomeanMaxThroughput =
        geomean(tasks.map((e) => e.info.maxThroughput).toList());
    return geomeanThroughput / geomeanMaxThroughput;
  }

  @override
  Widget build(BuildContext context) {
    final state = context.watch<BenchmarkState>();
    final shareEnabled = context.select<Store, bool>((value) => value.share);
    final offlineMode =
        context.select<Store, bool>((value) => value.offlineMode);
    final stringResources = AppLocalizations.of(context);
    final scrollController = ScrollController();

    final lastResultManager = context.watch<LastResultManager>();
    final results = lastResultManager.value!.results;
    final taskList = context.watch<TaskListManager>().taskList;

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
                      Tab(text: stringResources.resultsTabTitlePerformance),
                      Tab(text: stringResources.resultsTabTitleAccuracy),
                    ],
                  ),
                ),
                Expanded(
                  child: ResultCircle(
                      calculateOverallPercentage(taskList.benchmarks, results)),
                ),
                Expanded(
                  child: Column(
                    mainAxisAlignment: MainAxisAlignment.center,
                    children: _makeTopResultsView(
                        context, taskList.benchmarks, results),
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
                stringResources.resultsTitleDetails,
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

    final fm = context.watch<FirebaseManager?>();

    final detailedResultsPage = Column(children: [
      _makeBottomResultsView(context, taskList.benchmarks, results),
      Padding(
          padding: const EdgeInsets.fromLTRB(10, 10, 10, 10),
          child: TextButton(
            style: buttonStyle,
            onPressed: () async {
              // TODO (anhappdev) Refactor the code here to avoid duplicated code.
              // The checks before calling state.runBenchmarks() in main_screen and result_screen are similar.
              final wrongPathError = await state.validator
                  .validateExternalResourcesDirectory(
                      stringResources.dialogContentMissingFiles);
              if (wrongPathError.isNotEmpty) {
                if (!mounted) return;
                await showErrorDialog(context, [wrongPathError]);
                return;
              }
              if (offlineMode) {
                final offlineError = await state.validator.validateOfflineMode(
                    stringResources.dialogContentOfflineWarning);
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
                    ['${stringResources.runFail}:', e.toString()]);
                return;
              }
            },
            child: Padding(
              padding: const EdgeInsets.fromLTRB(0, 10, 0, 10),
              child: Text(
                stringResources.resultsButtonTestAgain,
                style: const TextStyle(
                  fontSize: 20.0,
                  color: AppColors.lightText,
                ),
              ),
            ),
          )),
      shareEnabled
          ? Padding(
              padding: const EdgeInsets.fromLTRB(10, 10, 10, 10),
              child: TextButton(
                onPressed: () async {
                  final lastResult = lastResultManager.value;
                  if (lastResult == null) {
                    return;
                  }
                  await Share.share(
                    jsonToStringIndented(lastResult),
                    subject: stringResources.resultsShareSubject,
                  );
                },
                child: Text(stringResources.resultsButtonShare,
                    style: TextStyle(
                      color: AppColors.shareTextButton,
                      fontSize: 18,
                    )),
              ),
            )
          : Container(),
      shareEnabled && fm != null
          ? Padding(
              padding: const EdgeInsets.fromLTRB(10, 10, 10, 10),
              child: TextButton(
                onPressed: () async {
                  try {
                    final lastResult = lastResultManager.value;
                    if (lastResult == null) {
                      return;
                    }
                    await fm.restHelper.upload(lastResult);
                    if (!mounted) return;
                    await showSuccessDialog(
                        context, [stringResources.uploadSuccess]);
                  } catch (e, s) {
                    print(e);
                    print(s);
                    await showErrorDialog(
                        context, [stringResources.uploadFail, e.toString()]);
                    return;
                  }
                },
                child: Text(stringResources.resultsButtonUpload,
                    style: TextStyle(
                      color: AppColors.shareTextButton,
                      fontSize: 18,
                    )),
              ),
            )
          : Container(),
    ]);

    String title;
    title = _screenMode == _ScreenMode.performance
        ? stringResources.resultsTitlePerformance
        : stringResources.resultsTitleAccuracy;
    title = isOfficialBuild
        ? title
        : '${stringResources.resultsTitleUnverified} $title';

    return Scaffold(
      appBar: MyAppBar.buildAppBar(title, context, true),
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

class _ResultViewHelper {
  final String taskName;
  final String? textResult;
  final double? numericResult;
  final String? textResult2;
  final double? numericResult2;
  final bool resultIsValid;
  final String backendInfoSubtitle;
  final String batchSize;

  _ResultViewHelper({
    required this.taskName,
    required this.textResult,
    required this.numericResult,
    required this.textResult2,
    required this.numericResult2,
    required this.resultIsValid,
    required this.backendInfoSubtitle,
    required this.batchSize,
  });

  factory _ResultViewHelper.empty(Benchmark benchmark) {
    return _ResultViewHelper(
      taskName: benchmark.taskConfig.name,
      textResult: null,
      numericResult: 0.0,
      textResult2: null,
      numericResult2: null,
      resultIsValid: false,
      backendInfoSubtitle: 'N/A',
      batchSize: 'N/A',
    );
  }
  factory _ResultViewHelper.performance(
    Benchmark benchmark,
    BenchmarkExportResult exportResult,
  ) {
    final runResult = exportResult.performanceRun;
    if (runResult == null) {
      return _ResultViewHelper.empty(benchmark);
    }
    final throughput = runResult.throughput;
    return _ResultViewHelper(
      taskName: benchmark.taskConfig.name,
      textResult: throughput?.toStringAsFixed(2),
      numericResult: (throughput ?? 0.0) / benchmark.info.maxThroughput,
      textResult2: null,
      numericResult2: null,
      resultIsValid: runResult.loadgenInfo?.validity ?? false,
      backendInfoSubtitle: _makeBackendSubtitle(exportResult),
      batchSize: exportResult.backendSettings.batchSize.toString(),
    );
  }
  factory _ResultViewHelper.accuracy(
    Benchmark benchmark,
    BenchmarkExportResult exportResult,
  ) {
    final runResult = exportResult.accuracyRun;
    if (runResult == null) {
      return _ResultViewHelper.empty(benchmark);
    }
    return _ResultViewHelper(
      taskName: benchmark.taskConfig.name,
      textResult: runResult.accuracy?.formatted,
      numericResult: runResult.accuracy?.normalized,
      textResult2: runResult.accuracy2?.formatted,
      numericResult2: runResult.accuracy2?.normalized,
      resultIsValid: (runResult.accuracy?.normalized ?? -1.0) >= 0.0 &&
          (runResult.accuracy?.normalized ?? -1.0) <= 1.0 &&
          (runResult.accuracy2?.normalized ?? -1.0) <= 1.0,
      backendInfoSubtitle: _makeBackendSubtitle(exportResult),
      batchSize: exportResult.backendSettings.batchSize.toString(),
    );
  }

  static String _makeBackendSubtitle(BenchmarkExportResult exportResult) {
    final backendName = exportResult.backendInfo.backendName;
    final acceleratorName = exportResult.backendInfo.acceleratorName;
    return '$backendName | $acceleratorName';
  }
}
