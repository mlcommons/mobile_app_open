import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';

import 'package:provider/provider.dart';
import 'package:quiver/iterables.dart';
import 'package:share_plus/share_plus.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/icons.dart' as app_icons;
import 'package:mlperfbench/info.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/store.dart';
import 'package:mlperfbench/ui/app_bar.dart';
import 'package:mlperfbench/ui/confirm_dialog.dart';
import 'package:mlperfbench/ui/error_dialog.dart';
import 'package:mlperfbench/ui/list_of_benchmark_items.dart';
import 'package:mlperfbench/ui/page_constraints.dart';
import 'package:mlperfbench/ui/result_circle.dart';
import 'main_screen.dart';

enum _ScreenMode { performance, accuracy }

class ResultScreen extends StatefulWidget {
  @override
  _ResultScreenState createState() => _ResultScreenState();
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

  String _getFormattedAccuracyValue(Benchmark benchmark) {
    final accuracy = benchmark.accuracy;
    if (accuracy == null) return 'N/A';
    final numeric = _getNumericAccuracy(accuracy);
    switch (benchmark.type) {
      // if the benchmark type is unknown, just show the original string
      // so we know that this need to be fixed
      case BenchmarkTypeEnum.unknown:
        return _getAccuracyValue(benchmark.accuracy);
      case BenchmarkTypeEnum.imageClassification:
        return (numeric * 100).toStringAsFixed(2);
      case BenchmarkTypeEnum.objectDetection:
        return (numeric * 100).toStringAsFixed(2);
      case BenchmarkTypeEnum.imageSegmentation:
        return (numeric * 100).toStringAsFixed(2);
      case BenchmarkTypeEnum.languageUnderstanding:
        return numeric.toStringAsFixed(2);
    }
  }

  double _getNumericAccuracy(String accuracy) {
    final percentPattern = '%';

    accuracy = _getAccuracyValue(accuracy);

    if (!accuracy.endsWith(percentPattern)) {
      return double.tryParse(accuracy) ?? 0.0;
    }

    return (double.tryParse(accuracy.replaceAll(RegExp('%'), '')) ?? 0.0) / 100;
  }

  String _getAccuracyValue(String? accuracy) {
    final onlyNumbersWithPercentPattern = '[^.%0-9]';
    final notAvailable = 'N/A';

    if (accuracy != null) {
      if (accuracy == notAvailable) {
        return accuracy;
      }
      return accuracy.replaceAll(RegExp(onlyNumbersWithPercentPattern), '');
    }

    return notAvailable;
  }

  Column _createListOfBenchmarkResultWidgets(
      BuildContext context, BenchmarkState state) {
    final list = <Widget>[];
    final stringResources = AppLocalizations.of(context);
    final pictureEdgeSize = 0.08 * MediaQuery.of(context).size.width;

    for (final benchmark in state.benchmarks) {
      late final String textResult;
      late final double numericResult;
      if (_screenMode == _ScreenMode.performance) {
        textResult = benchmark.score?.toStringAsFixed(2) ?? 'N/A';
        numericResult = (benchmark.score ?? 0) / benchmark.maxScore;
      } else if (_screenMode == _ScreenMode.accuracy) {
        textResult = benchmark.accuracy ?? 'N/A';
        numericResult = _getNumericAccuracy(textResult);
      } else {
        continue;
      }
      var rowChildren = <Widget>[];
      rowChildren.add(Row(
        crossAxisAlignment: CrossAxisAlignment.start,
        mainAxisAlignment: MainAxisAlignment.spaceBetween,
        children: [
          Padding(
              padding: const EdgeInsets.only(bottom: 5),
              child: Text(benchmark.backendDescription)),
          Row(
            crossAxisAlignment: CrossAxisAlignment.center,
            children: [
              Text(
                textResult,
                style: TextStyle(
                  color: AppColors.darkText,
                  fontSize: 16.0,
                  fontWeight: FontWeight.bold,
                ),
              ),
              Icon(Icons.chevron_right, color: Colors.grey),
            ],
          ),
        ],
      ));
      if (benchmark.benchmarkSetting.batchSize > 0) {
        String shardsNum;
        String batchSize;
        if (textResult == 'N/A') {
          shardsNum = 'N/A';
          batchSize = 'N/A';
        } else {
          try {
            shardsNum = benchmark.benchmarkSetting.customSetting
                .firstWhere((element) => element.id == 'shards_num')
                .value;
            batchSize = benchmark.benchmarkSetting.customSetting
                .firstWhere((element) => element.id == 'batch_size')
                .value;
          } catch (_) {
            shardsNum = 'N/A';
            batchSize = 'N/A';
          }
        }

        rowChildren.add(Row(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text(
              stringResources.resultsThreadsNumber
                  .replaceAll('<threadsNumber>', shardsNum),
              style: TextStyle(
                color: Colors.grey,
                fontSize: 14.0,
              ),
            ),
          ],
        ));
        rowChildren.add(Row(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text(
              stringResources.resultsBatchSize
                  .replaceAll('<batchSize>', batchSize),
              style: TextStyle(
                color: Colors.grey,
                fontSize: 14.0,
              ),
            ),
          ],
        ));
      }
      rowChildren.add(FractionallySizedBox(
          widthFactor: 0.9, child: BlueProgressLine(numericResult)));
      list.add(
        Column(
          children: [
            ListTile(
              leading: Container(
                  width: pictureEdgeSize,
                  height: pictureEdgeSize,
                  child: benchmark.icon),
              title: Padding(
                padding: const EdgeInsets.fromLTRB(0, 5, 0, 5),
                child: Text(getBenchmarkName(benchmark, stringResources)),
              ),
              subtitle: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: rowChildren,
              ),
              onTap: () => showBenchmarkInfoBottomSheet(context, benchmark),
            ),
            Divider()
          ],
        ),
      );
    }
    return Column(children: list);
  }

  List<Widget> _createListOfBenchmarkResultTopWidgets(
      BenchmarkState state, BuildContext context) {
    final stringResources = AppLocalizations.of(context);

    final widgets = state.benchmarks.map((benchmark) => Padding(
          padding: const EdgeInsets.only(bottom: 10),
          child: Column(
              mainAxisAlignment: MainAxisAlignment.spaceEvenly,
              children: [
                Padding(
                  padding: const EdgeInsets.only(bottom: 8),
                  child: Text(
                    getBenchmarkName(benchmark, stringResources)
                        .split(' ')
                        .join('\n')
                        .toUpperCase(),
                    textAlign: TextAlign.center,
                    style:
                        TextStyle(fontSize: 12.0, color: AppColors.lightText),
                  ),
                ),
                Text(
                  _screenMode == _ScreenMode.performance
                      ? benchmark.score?.toStringAsFixed(2) ?? 'N/A'
                      : _getFormattedAccuracyValue(benchmark),
                  style: TextStyle(
                      fontSize: 32.0,
                      color: AppColors.lightText,
                      fontWeight: FontWeight.bold),
                ),
              ]),
        ));

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
    final stringResources = AppLocalizations.of(context);
    final scrollController = ScrollController();

    final resultsPage = Column(
      children: [
        Expanded(
          child: CustomPaint(
            painter: MyPaintBottom(),
            child: Column(
              children: <Widget>[
                Container(
                  width: MediaQuery.of(context).size.width * 0.75,
                  child: TabBar(
                    controller: _tabController,
                    indicator: UnderlineTabIndicator(),
                    indicatorSize: TabBarIndicatorSize.label,
                    tabs: [
                      Tab(text: stringResources.performance),
                      Tab(text: stringResources.accuracy),
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
                stringResources.detailedResults,
                textAlign: TextAlign.left,
                style: TextStyle(fontSize: 17.0, fontWeight: FontWeight.bold),
              ),
              IconButton(
                  key: Key(ResultKeys.scrollResultsButton),
                  icon: app_icons.Icons.arrow,
                  onPressed: () {
                    scrollController.animateTo(
                        scrollController.position.maxScrollExtent,
                        duration: Duration(milliseconds: 200),
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
            side: BorderSide(color: Colors.white))),
        minimumSize:
            MaterialStateProperty.all<Size>(Size(minimumShareButtonWidth, 0)));

    final detailedResultsPage = Column(children: [
      _createListOfBenchmarkResultWidgets(context, state),
      Padding(
          padding: EdgeInsets.fromLTRB(10, 10, 10, 10),
          child: TextButton(
            style: buttonStyle,
            onPressed: () async {
              final wrongPathError =
                  await state.validateExternalResourcesDirectory(
                      stringResources.incorrectDatasetsPath);
              if (wrongPathError.isNotEmpty) {
                await showErrorDialog(context, [wrongPathError]);
                return;
              }
              if (store.offlineMode) {
                final offlineError = await state.validateOfflineMode(
                    stringResources.warningOfflineModeEnabled);
                if (offlineError.isNotEmpty) {
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
              state.runBenchmarks();
            },
            child: Padding(
              padding: EdgeInsets.fromLTRB(0, 10, 0, 10),
              child: Text(
                stringResources.testAgain,
                style: TextStyle(fontSize: 20.0, color: AppColors.lightText),
              ),
            ),
          )),
      store.share
          ? Padding(
              padding: EdgeInsets.fromLTRB(10, 10, 10, 10),
              child: TextButton(
                onPressed: () async {
                  await Share.share(
                    await state.resourceManager.resultManager.read(),
                    subject: stringResources.experimentResultsSubj,
                  );
                },
                child: Text(stringResources.shareResults,
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
        ? stringResources.resultsPerformanceTitle
        : stringResources.resultsAccuracyTitle;
    title = OFFICIAL_BUILD ? title : '${stringResources.unverified} $title';

    return Scaffold(
      appBar: MyAppBar.buildAppBar(title, context, true),
      body: LayoutBuilder(
        builder: (context, constraint) {
          return SingleChildScrollView(
            physics: ClampingScrollPhysics(),
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

  BlueProgressLine(this._progress);

  double get _progressValue {
    final _rangedProgress = _progress.clamp(0, 1);
    final _startOffset = 0.01;

    return _startOffset + (1 - _startOffset) * _rangedProgress;
  }

  @override
  Widget build(BuildContext context) {
    return FractionallySizedBox(
      alignment: Alignment.topLeft,
      widthFactor: _progressValue,
      child: Container(
        alignment: Alignment.topLeft,
        margin: EdgeInsets.only(bottom: 10.0),
        height: 10,
        decoration: BoxDecoration(
          borderRadius: BorderRadius.circular(5.0),
          gradient: LinearGradient(
            colors: AppColors.resultBarGradient,
            begin: Alignment.topLeft,
            end: Alignment(1 / _progressValue, 0),
            stops: [0, 0.36, 0.61, 0.83, 1.0],
          ),
        ),
      ),
    );
  }
}
