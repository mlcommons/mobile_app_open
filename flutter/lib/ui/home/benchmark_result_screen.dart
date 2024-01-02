import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/device_info.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/confirm_dialog.dart';
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

class _BenchmarkResultScreenState extends State<BenchmarkResultScreen>
    with SingleTickerProviderStateMixin {
  _ScreenMode _screenMode = _ScreenMode.performance;

  late final TabController _tabController;

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
  }

  @override
  void dispose() {
    _tabController.dispose();
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
      appBar: AppBar(
        title: Text(title),
        bottom: PreferredSize(
          preferredSize: const Size.fromHeight(56.0),
          child: _sharingSection(),
        ),
      ),
      drawer: const AppDrawer(),
      body: LayoutBuilder(
        builder: (context, constraint) {
          return SingleChildScrollView(
            physics: const ClampingScrollPhysics(),
            child: Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: <Widget>[
                _totalScoreSection(),
                const SizedBox(height: 20),
                _detailSection(),
              ],
            ),
          );
        },
      ),
    );
  }

  Widget _sharingSection() {
    final lastResult = state.lastResult;
    Text deviceInfoText;
    Text benchmarkDateText;
    if (lastResult != null) {
      deviceInfoText = Text(lastResult.environmentInfo.modelDescription);
      benchmarkDateText = Text(formatDateTime(lastResult.meta.creationDate));
    } else {
      deviceInfoText = Text(DeviceInfo.instance.envInfo.modelDescription);
      benchmarkDateText = const Text('');
    }
    final infoSection = Column(
      mainAxisAlignment: MainAxisAlignment.spaceBetween,
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        deviceInfoText,
        const SizedBox(height: 4),
        benchmarkDateText,
      ],
    );
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
        if (!context.mounted) return;
        switch (await showConfirmDialog(context, l10n.resultsDeleteConfirm)) {
          case ConfirmDialogAction.ok:
            await state.resourceManager.resultManager.deleteLastResult();
            await state.resetBenchmarkState();
            break;
          case ConfirmDialogAction.cancel:
            return;
          default:
            break;
        }
      },
    );
    return Container(
      padding: const EdgeInsets.fromLTRB(20, 8, 10, 8),
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

  Widget _totalScoreSection() {
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
          ResultCircle(
            key: const Key(WidgetKeys.totalScoreCircle),
            state.result,
          ),
        ],
      ),
    );
  }

  Widget _detailSection() {
    final children = <Widget>[];
    for (final benchmark in state.benchmarks) {
      final row = _benchmarkResultRow(benchmark);
      children.add(row);
      children.add(const Divider());
    }
    return Column(
      mainAxisAlignment: MainAxisAlignment.spaceEvenly,
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: children,
    );
  }

  Widget _benchmarkResultRow(Benchmark benchmark) {
    final leadingWidth = 0.12 * MediaQuery.of(context).size.width;
    final subtitleWidth = 0.70 * MediaQuery.of(context).size.width;
    final trailingWidth = 0.28 * MediaQuery.of(context).size.width;
    late final String? resultText;
    late final double? progressBarValue;
    late final String? resultText2;
    late final double? progressBarValue2;
    late final BenchmarkResult? benchmarkResult;
    late final bool resultIsValid;
    switch (_screenMode) {
      case _ScreenMode.performance:
        benchmarkResult = benchmark.performanceModeResult;
        final throughput = benchmarkResult?.throughput;
        resultText = throughput?.toUIString();
        progressBarValue =
            (throughput?.value ?? 0.0) / benchmark.info.maxThroughput;
        resultText2 = null;
        progressBarValue2 = null;
        resultIsValid = benchmarkResult?.validity ?? false;
        break;
      case _ScreenMode.accuracy:
        benchmarkResult = benchmark.accuracyModeResult;
        resultText = benchmarkResult?.accuracy?.formatted;
        progressBarValue = benchmarkResult?.accuracy?.normalized;
        resultText2 = benchmarkResult?.accuracy2?.formatted;
        progressBarValue2 = benchmarkResult?.accuracy2?.normalized;
        resultIsValid =
            (benchmarkResult?.accuracy?.normalized ?? -1.0) >= 0.0 &&
                (benchmarkResult?.accuracy?.normalized ?? -1.0) <= 1.0 &&
                (benchmarkResult?.accuracy2?.normalized ?? -1.0) <= 1.0;
        break;
    }
    final perfResult = benchmark.performanceModeResult;
    var backendInfo = l10n.na;
    if (perfResult != null) {
      final backendName = perfResult.backendName;
      final delegateName = perfResult.delegateName;
      final acceleratorName = perfResult.acceleratorName;
      backendInfo = '$backendName | $delegateName | $acceleratorName';
    }
    var subtitleColumnChildren = <Widget>[];
    subtitleColumnChildren.add(const SizedBox(height: 4));
    final resultTextStyle = TextStyle(
      color: resultIsValid ? AppColors.resultValid : AppColors.resultInvalid,
      fontSize: 18.0,
      fontWeight: FontWeight.bold,
    );
    final benchmarkScore = Column(
      mainAxisAlignment: MainAxisAlignment.center,
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Text(resultText ?? l10n.na, style: resultTextStyle),
        if (resultText2 != null) Text(resultText2, style: resultTextStyle),
      ],
    );
    final backendInfoRow = Text(backendInfo);
    subtitleColumnChildren.add(backendInfoRow);
    subtitleColumnChildren.add(const SizedBox(height: 8));

    if (benchmark.info.isOffline) {
      String batchSize;
      if (resultText == null) {
        batchSize = l10n.na;
      } else {
        batchSize = benchmarkResult?.batchSize.toString() ?? '';
      }
      final batchSizeRow = Row(
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
      subtitleColumnChildren.add(batchSizeRow);
      subtitleColumnChildren.add(const SizedBox(height: 8));
    }

    final progressBarRow = FractionallySizedBox(
      widthFactor: 0.9,
      child: BlueProgressLine(progressBarValue ?? 0.0),
    );
    subtitleColumnChildren.add(progressBarRow);

    if (progressBarValue2 != null) {
      final progressBarRow2 = FractionallySizedBox(
        widthFactor: 0.9,
        child: BlueProgressLine(progressBarValue2),
      );
      subtitleColumnChildren.add(progressBarRow2);
    }

    return ListTile(
      contentPadding: const EdgeInsets.fromLTRB(10, 0, 10, 0),
      minVerticalPadding: 0,
      leading: SizedBox(
          width: leadingWidth,
          height: leadingWidth,
          child: benchmark.info.icon),
      title: SizedBox(
        width: subtitleWidth,
        child: Text(benchmark.taskConfig.name),
      ),
      subtitle: SizedBox(
        width: subtitleWidth,
        child: Column(
          mainAxisAlignment: MainAxisAlignment.start,
          crossAxisAlignment: CrossAxisAlignment.start,
          children: subtitleColumnChildren,
        ),
      ),
      trailing: SizedBox(
        width: trailingWidth,
        child: Row(
          mainAxisAlignment: MainAxisAlignment.end,
          crossAxisAlignment: CrossAxisAlignment.center,
          children: [
            Flexible(
              flex: 7,
              fit: FlexFit.tight,
              child: benchmarkScore,
            ),
            Flexible(
              flex: 3,
              fit: FlexFit.tight,
              child: BenchmarkInfoButton(benchmark: benchmark),
            ),
          ],
        ),
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
