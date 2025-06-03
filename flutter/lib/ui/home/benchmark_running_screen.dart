import 'dart:async';
import 'dart:math';

import 'package:flutter/material.dart';

import 'package:collection/collection.dart';
import 'package:provider/provider.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/benchmark/info.dart';
import 'package:mlperfbench/benchmark/run_mode.dart';
import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/resources/utils.dart';
import 'package:mlperfbench/state/task_runner.dart';
import 'package:mlperfbench/store.dart';
import 'package:mlperfbench/ui/app_styles.dart';
import 'package:mlperfbench/ui/auto_size_text.dart';
import 'package:mlperfbench/ui/formatter.dart';
import 'package:mlperfbench/ui/home/progress_circle.dart';
import 'package:mlperfbench/ui/icons.dart';

class BenchmarkRunningScreen extends StatefulWidget {
  static final GlobalKey<ScaffoldState> scaffoldKey =
      GlobalKey<ScaffoldState>();

  const BenchmarkRunningScreen({super.key});

  @override
  State<BenchmarkRunningScreen> createState() => _BenchmarkRunningScreenState();
}

class _BenchmarkRunningScreenState extends State<BenchmarkRunningScreen> {
  late BenchmarkState state;
  late Store store;
  late AppLocalizations l10n;
  late ProgressInfo progress;

  @override
  Widget build(BuildContext context) {
    state = context.watch<BenchmarkState>();
    store = context.watch<Store>();
    l10n = AppLocalizations.of(context)!;
    progress = state.taskRunner.progressInfo;

    final backgroundGradient = BoxDecoration(
      gradient: LinearGradient(
        colors: AppGradients.fullScreen,
        begin: Alignment.topCenter,
        end: Alignment.bottomCenter,
      ),
    );

    return Scaffold(
      key: BenchmarkRunningScreen.scaffoldKey,
      body: Container(
        decoration: backgroundGradient,
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.center,
          mainAxisAlignment: MainAxisAlignment.spaceAround,
          children: [
            _title(),
            const SizedBox(height: 20),
            _circle(),
            const SizedBox(height: 20),
            Expanded(child: _taskList()),
            const SizedBox(height: 20),
            _footer(),
          ],
        ),
      ),
    );
  }

  Widget _title() {
    // The TaskRunner always run all benchmarks in performance mode first then in accuracy mode.
    var loadgenRunModeStage = '1/1';
    if (progress.runMode.selectedRunModes.length > 1) {
      loadgenRunModeStage = progress.accuracy ? '2/2' : '1/2';
    }
    final loadgenRunModeName =
        progress.accuracy ? l10n.progressAccuracy : l10n.progressPerformance;
    final benchmarkRunModeName =
        store.selectedBenchmarkRunMode.localizedName(l10n);
    return Padding(
        padding: const EdgeInsets.fromLTRB(40, 48, 40, 4),
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          crossAxisAlignment: CrossAxisAlignment.center,
          children: [
            Text(
              '($loadgenRunModeStage) $loadgenRunModeName',
              style: const TextStyle(
                fontWeight: FontWeight.w500,
                color: AppColors.lightText,
                fontSize: 20,
              ),
            ),
            Text(
              benchmarkRunModeName,
              style: const TextStyle(
                fontWeight: FontWeight.w500,
                color: AppColors.lightText,
                fontSize: 14,
              ),
            ),
          ],
        ));
  }

  Widget _circle() {
    var diameter = 0.50 * MediaQuery.of(context).size.width;
    diameter = diameter.clamp(160.0, 240.0);
    return SizedBox(
      width: diameter,
      height: diameter,
      child: Stack(
        alignment: AlignmentDirectional.center,
        children: <Widget>[
          Container(
            decoration: const BoxDecoration(
              shape: BoxShape.circle,
              color: AppColors.progressCircle,
              boxShadow: [
                BoxShadow(
                  color: Colors.black12,
                  offset: Offset(15, 15),
                  blurRadius: 10,
                )
              ],
            ),
            child: ClipOval(
              child: Padding(
                padding: const EdgeInsets.all(4),
                child: _circleContent(diameter),
              ),
            ),
          ),
          ProgressCircle(
            key: const Key(WidgetKeys.progressCircle),
            strokeWidth: 6,
            size: diameter + 20,
          ),
        ],
      ),
    );
  }

  Widget _circleContent(double diameter) {
    Widget? topWidget;
    String taskNameString;

    final double containerRadius = diameter / 2;
    final double creepFactor = lerpRange(2, 6, 160, 240, diameter)!;
    final double horizontalPadding = containerRadius -
        sqrt(pow(containerRadius, 2) -
            pow((containerRadius - (8 * creepFactor)),
                2)); // 8 is the padding from the bottom of the circle to the bottom of the text

    const textStyle = TextStyle(
      fontSize: 14,
      fontWeight: FontWeight.w500,
      color: AppColors.lightText,
    );
    if (progress.cooldown) {
      topWidget = AutoSizeText(
        l10n.progressCooldown,
        circle: true,
        textAlign: TextAlign.center,
        style: textStyle,
        circularPadding: horizontalPadding,
      );
      taskNameString = l10n.progressRemainingTime;
    } else {
      topWidget = SizedBox(
        width: 32,
        height: 32,
        child: progress.currentBenchmark!.iconWhite,
      );
      taskNameString = progress.currentBenchmark!.taskName;
    }

    return Column(
      mainAxisAlignment: MainAxisAlignment.spaceAround,
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        Expanded(
          flex: 3,
          child: Container(
            alignment: Alignment.bottomCenter,
            padding: const EdgeInsets.symmetric(vertical: 4),
            child: topWidget,
          ),
        ),
        Expanded(
          flex: 4,
          child: Container(
            alignment: Alignment.center,
            child: _StageProgressText(),
          ),
        ),
        Expanded(
          flex: 3,
          child: Container(
            alignment: Alignment.topCenter,
            padding: const EdgeInsets.symmetric(vertical: 4),
            child: AutoSizeText(
              taskNameString,
              circle: true,
              textAlign: TextAlign.center,
              style: textStyle,
              maxLines: 2,
              overflow: TextOverflow.ellipsis,
              circularPadding: horizontalPadding,
            ),
          ),
        ),
      ],
    );
  }

  Widget _taskList() {
    final childrenList = <Widget>[];
    progress.activeBenchmarks.forEachIndexed((idx, benchmark) {
      childrenList.add(_listTile(benchmark, idx % 2 == 0));
    });
    return Material(
      type: MaterialType.transparency,
      child: ListView(
        shrinkWrap: true,
        children: childrenList,
      ),
    );
  }

  Widget _listTile(BenchmarkInfo benchmarkInfo, bool isEven) {
    final leadingWidth = 0.10 * MediaQuery.of(context).size.width;
    final titleWidth = 0.70 * MediaQuery.of(context).size.width;
    const trailingWidth = 24.0;
    Widget? doneIcon;
    if (progress.currentBenchmark?.taskName == benchmarkInfo.taskName) {
      doneIcon = const ProgressCircle(
        strokeWidth: 2,
        size: trailingWidth - 4,
      );
    } else if (progress.completedBenchmarks.contains(benchmarkInfo)) {
      doneIcon = const Icon(
        Icons.check_circle,
        size: trailingWidth,
        color: Colors.green,
      );
    }
    return ListTile(
      contentPadding: const EdgeInsets.symmetric(vertical: 0, horizontal: 32),
      tileColor: isEven ? AppColors.progressCircle : Colors.transparent,
      textColor: AppColors.lightText,
      dense: true,
      minVerticalPadding: 0,
      leading: SizedBox(
          width: leadingWidth,
          height: leadingWidth,
          child: Padding(
            padding: const EdgeInsets.all(8),
            child: benchmarkInfo.iconWhite,
          )),
      title: SizedBox(
        width: titleWidth,
        child: Text(
          benchmarkInfo.taskName,
          style: const TextStyle(
            fontSize: 14,
            fontWeight: FontWeight.bold,
          ),
        ),
      ),
      trailing: SizedBox(
        width: trailingWidth,
        height: trailingWidth,
        child: Center(child: doneIcon),
      ),
    );
  }

  Widget _footer() {
    if (state.state == BenchmarkStateEnum.aborting) {
      return _abortingHint();
    } else {
      return Column(
        mainAxisAlignment: MainAxisAlignment.center,
        crossAxisAlignment: CrossAxisAlignment.center,
        mainAxisSize: MainAxisSize.min,
        children: [
          _footerText(),
          _cancelButton(),
        ],
      );
    }
  }

  Widget _footerText() {
    return Wrap(
      crossAxisAlignment: WrapCrossAlignment.center,
      children: [
        const Icon(
          Icons.warning,
          color: AppColors.lightText,
        ),
        const SizedBox(width: 8),
        Text(
          l10n.progressDontClose,
          style: const TextStyle(
            color: AppColors.lightText,
            fontSize: 17,
          ),
        )
      ],
    );
  }

  Widget _cancelButton() {
    return Padding(
      padding: const EdgeInsets.fromLTRB(20, 10, 20, 20),
      child: TextButton(
        style: ButtonStyle(
          shape: MaterialStateProperty.all(
            RoundedRectangleBorder(
              borderRadius: BorderRadius.circular(18.0),
              side: const BorderSide(color: Colors.white),
            ),
          ),
        ),
        onPressed: () async => await state.taskRunner.abortBenchmarks(),
        child: Padding(
          padding: const EdgeInsets.fromLTRB(10, 0, 10, 0),
          child: Text(
            l10n.progressCancel,
            style: const TextStyle(
              fontSize: 15.0,
              color: AppColors.lightText,
            ),
          ),
        ),
      ),
    );
  }

  Widget _abortingHint() {
    return Column(
      mainAxisAlignment: MainAxisAlignment.center,
      crossAxisAlignment: CrossAxisAlignment.center,
      children: [
        Row(
          mainAxisAlignment: MainAxisAlignment.center,
          crossAxisAlignment: CrossAxisAlignment.center,
          children: [
            Container(
              width: 18,
              alignment: Alignment.center,
              child: AppIcons.waiting,
            ),
            const SizedBox(width: 8),
            Text(
              l10n.progressAborting,
              style: const TextStyle(
                fontSize: 18,
                color: AppColors.lightText,
              ),
            )
          ],
        ),
        Text(
          l10n.progressWaiting,
          style: const TextStyle(
            fontSize: 14,
            color: AppColors.lightText,
          ),
        ),
      ],
    );
  }
}

class _StageProgressText extends StatefulWidget {
  @override
  _StageProgressTextState createState() => _StageProgressTextState();
}

class _StageProgressTextState extends State<_StageProgressText> {
  late final Timer _timer;

  @override
  void initState() {
    // UI should update every 1 second to refresh stageProgress
    _timer = Timer.periodic(const Duration(seconds: 1), (_) => setState(() {}));
    super.initState();
  }

  @override
  void dispose() {
    _timer.cancel();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final state = context.watch<BenchmarkState>();
    final progress = state.taskRunner.progressInfo;
    String progressStr;
    if (progress.cooldown) {
      progressStr = (progress.cooldownDuration * (1.0 - progress.stageProgress))
          .toDurationUIString();
    } else {
      progressStr = '${(progress.stageProgress * 100).round().clamp(0, 100)}%';
    }
    return Text(
      progressStr,
      style: const TextStyle(
        fontSize: 54,
        height: 1.0,
        fontWeight: FontWeight.bold,
        color: AppColors.lightText,
      ),
    );
  }
}
