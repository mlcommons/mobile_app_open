import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/home/app_drawer.dart';
import 'package:mlperfbench/ui/home/benchmark_info_button.dart';
import 'package:mlperfbench/ui/home/result_circle.dart';
import 'package:mlperfbench/ui/home/share_button.dart';
import 'package:mlperfbench/ui/home/shared_styles.dart';
import 'package:mlperfbench/ui/time_utils.dart';

enum _ScreenMode { performance, accuracy }

class BenchmarkResultScreen extends StatefulWidget {
  const BenchmarkResultScreen({Key? key}) : super(key: key);

  @override
  State<BenchmarkResultScreen> createState() => _BenchmarkResultScreenState();
}

class ResultKeys {
  // list of widget keys that need to be accessed in the test code
  static const String scrollResultsButton = 'scrollResultsButton';
}

class _BenchmarkResultScreenState extends State<BenchmarkResultScreen>
    with SingleTickerProviderStateMixin {
  _ScreenMode _screenMode = _ScreenMode.performance;

  late final TabController _tabController;
  late final ScrollController _scrollController;

  late AppLocalizations l10n;
  late BenchmarkState state;

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
    _scrollController = ScrollController();
  }

  @override
  void dispose() {
    _tabController.dispose();
    _scrollController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    state = context.watch<BenchmarkState>();
    l10n = AppLocalizations.of(context);

    String title;
    title = _screenMode == _ScreenMode.performance
        ? l10n.resultsTitlePerformance
        : l10n.resultsTitleAccuracy;
    title = DartDefine.isOfficialBuild
        ? title
        : '${l10n.resultsTitleUnverified} $title';

    return Scaffold(
      appBar: AppBar(title: Text(title)),
      drawer: const AppDrawer(),
      body: LayoutBuilder(
        builder: (context, constraint) {
          return SingleChildScrollView(
            physics: const ClampingScrollPhysics(),
            controller: _scrollController,
            child: Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: <Widget>[
                (_sharingSection()),
                (_summarySection()),
                const SizedBox(height: 20),
                (_detailSection()),
              ],
            ),
          );
        },
      ),
    );
  }

  Widget _sharingSection() {
    final lastResult = state.lastResult;
    Widget infoSection = Container();
    if (lastResult != null) {
      infoSection = Column(
        mainAxisAlignment: MainAxisAlignment.spaceEvenly,
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text(lastResult.environmentInfo.modelDescription),
          Text(formatDateTime(lastResult.meta.creationDate)),
        ],
      );
    }
    Widget testAgainButton = IconButton(
      icon: const Icon(Icons.restart_alt),
      color: Colors.white,
      onPressed: () async {
        await state.resetBenchmarkState();
      },
    );
    Widget deleteResultButton = IconButton(
      icon: const Icon(Icons.delete),
      color: Colors.white,
      onPressed: () async {
        await state.resourceManager.resultManager.deleteLastResult();
        await state.resetBenchmarkState();
      },
    );
    return Container(
      padding: const EdgeInsets.fromLTRB(20, 0, 10, 0),
      color: AppColors.mediumBlue,
      child: DefaultTextStyle.merge(
        style: const TextStyle(color: Colors.white),
        child: Row(
          mainAxisAlignment: MainAxisAlignment.spaceAround,
          crossAxisAlignment: CrossAxisAlignment.center,
          children: [
            infoSection,
            const Spacer(),
            testAgainButton,
            deleteResultButton,
            const ShareButton()
          ],
        ),
      ),
    );
  }

  Widget _summarySection() {
    return Container(
      decoration: mainLinearGradientDecoration,
      child: Column(
        mainAxisAlignment: MainAxisAlignment.spaceEvenly,
        crossAxisAlignment: CrossAxisAlignment.center,
        children: <Widget>[
          TabBar(
            controller: _tabController,
            indicator: const UnderlineTabIndicator(),
            indicatorSize: TabBarIndicatorSize.label,
            tabs: [
              Tab(text: l10n.resultsTabTitlePerformance),
              Tab(text: l10n.resultsTabTitleAccuracy),
            ],
          ),
          ResultCircle(state.result),
        ],
      ),
    );
  }

  Widget _detailSection() {
    final list = <Widget>[];
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
      final perfResult = benchmark.performanceModeResult;
      var backendInfo = l10n.na;
      if (perfResult != null) {
        final backendName = perfResult.backendName;
        final delegateName = perfResult.delegateName;
        final acceleratorName = perfResult.acceleratorName;
        backendInfo = '$backendName | $delegateName | $acceleratorName';
      }
      var rowChildren = <Widget>[];
      rowChildren.add(Row(
        crossAxisAlignment: CrossAxisAlignment.start,
        mainAxisAlignment: MainAxisAlignment.spaceBetween,
        children: [
          Padding(
              padding: const EdgeInsets.only(bottom: 5),
              child: Text(backendInfo)),
          Row(
            crossAxisAlignment: CrossAxisAlignment.center,
            children: [
              Column(
                children: [
                  Text(
                    resultText ?? l10n.na,
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
            ],
          ),
        ],
      ));
      if (benchmark.info.isOffline) {
        String batchSize;
        if (resultText == null) {
          batchSize = l10n.na;
        } else {
          batchSize = benchmarkResult?.batchSize.toString() ?? '';
        }

        rowChildren.add(Row(
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
              trailing: BenchmarkInfoButton(benchmark: benchmark),
            ),
            const Divider()
          ],
        ),
      );
    }
    return Column(children: list);
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
