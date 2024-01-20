import 'dart:async';

import 'package:flutter/material.dart';

import 'package:collection/collection.dart';
import 'package:mlperfbench/benchmark/info.dart';
import 'package:mlperfbench/benchmark/run_mode.dart';
import 'package:provider/provider.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/state/task_runner.dart';
import 'package:mlperfbench/ui/home/progress_circles.dart';
import 'package:mlperfbench/ui/time_utils.dart';

class BenchmarkRunningScreen extends StatefulWidget {
  static final GlobalKey<ScaffoldState> scaffoldKey =
      GlobalKey<ScaffoldState>();

  const BenchmarkRunningScreen({Key? key}) : super(key: key);

  @override
  State<BenchmarkRunningScreen> createState() => _BenchmarkRunningScreenState();
}

class _BenchmarkRunningScreenState extends State<BenchmarkRunningScreen> {
  late final Timer _timer;
  late BenchmarkState state;
  late AppLocalizations l10n;
  late ProgressInfo progress;

  @override
  void initState() {
    // UI should update every 1 seconds to refresh stageProgress
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
    state = context.watch<BenchmarkState>();
    l10n = AppLocalizations.of(context);
    progress = state.taskRunner.progressInfo;

    // TODO: to delete mockup
    progress.runMode = BenchmarkRunModeEnum.submissionRun;
    progress.currentStage = 2;
    progress.calculateStageProgress = () {
      return 0.44;
    };
    progress.activeBenchmarks.clear();
    progress.completedBenchmarks.clear();
    for (var benchmark in state.benchmarks) {
      benchmark.isActive = true;
      progress.activeBenchmarks.add(benchmark.info);
    }
    progress.completedBenchmarks.add(state.benchmarks[0].info);
    progress.completedBenchmarks.add(state.benchmarks[1].info);
    progress.currentBenchmark = state.benchmarks[2].info;
    progress.accuracy = true;
    progress.cooldown = false;
    // END

    final backgroundGradient = BoxDecoration(
      gradient: LinearGradient(
        colors: AppColors.progressScreenGradient,
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
            Expanded(flex: 14, child: _title()),
            Expanded(flex: 30, child: _circle()),
            const SizedBox(height: 20),
            Expanded(flex: 40, child: _taskList()),
            const SizedBox(height: 20),
            Expanded(flex: 16, child: _footer()),
          ],
        ),
      ),
    );
  }

  Widget _title() {
    // The TaskRunner always run all benchmarks in performance mode first then in accuracy mode.
    var runModeStage = '1/1';
    if (progress.runMode == BenchmarkRunModeEnum.submissionRun) {
      runModeStage = progress.accuracy ? '2/2' : '1/2';
    }
    var runModeName =
        progress.accuracy ? l10n.progressAccuracy : l10n.progressPerformance;
    return Padding(
        padding: const EdgeInsets.fromLTRB(40, 48, 40, 4),
        child: Text(
          '($runModeStage) $runModeName',
          style: const TextStyle(
            fontWeight: FontWeight.w500,
            color: AppColors.lightText,
            fontSize: 20,
          ),
        ));
  }

  Widget _circle() {
    final containerWidth = 0.56 * MediaQuery.of(context).size.width;
    return Stack(
      alignment: AlignmentDirectional.centerStart,
      children: <Widget>[
        Center(
          child: Container(
              width: containerWidth,
              height: containerWidth,
              decoration: BoxDecoration(
                shape: BoxShape.circle,
                gradient: LinearGradient(
                  colors: AppColors.progressCircleGradient,
                  begin: Alignment.topCenter,
                  end: Alignment.bottomCenter,
                ),
                boxShadow: const [
                  BoxShadow(
                    color: Colors.black12,
                    offset: Offset(15, 15),
                    blurRadius: 10,
                  )
                ],
              ),
              child: Center(
                child: ClipOval(
                  child: Center(
                    child: Padding(
                      padding: const EdgeInsets.all(8),
                      child: _circleContent(),
                    ),
                  ),
                ),
              )
              // child: Text(
              //   '${progress.currentStage.toString()}/${progress.totalStages.toString()}',
              //   style: const TextStyle(
              //     fontWeight: FontWeight.bold,
              //     color: AppColors.lightText,
              //   ),
              //   textScaleFactor: 3,
              // ),
              // ),
              ),
        ),
        Center(
          child: InfiniteProgressCircle(
            size: containerWidth + 20,
            strokeWidth: 6.0,
          ),
        ),
      ],
    );
  }

  Widget _circleContent() {
    Widget? taskIcon;
    String progressString;
    String taskNameString;
    if (progress.cooldown) {
      taskIcon = null;
      progressString = l10n.progressScreenCooldown.replaceAll(
          '<remaining>',
          formatDuration(
              progress.cooldownDuration * (1.0 - progress.stageProgress)));
      taskNameString = l10n.progressCooldown;
    } else {
      progressString =
          '${(progress.stageProgress * 100).round().clamp(0, 100)}%';
      taskNameString = progress.currentBenchmark!.taskName;
      taskIcon = progress.currentBenchmark!.iconWhite;
    }

    return Column(
      mainAxisAlignment: MainAxisAlignment.spaceAround,
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        Expanded(
          flex: 3,
          child: Container(
            alignment: Alignment.bottomCenter,
            padding: const EdgeInsets.symmetric(vertical: 10, horizontal: 40),
            child: SizedBox(
              width: 32,
              height: 32,
              child: taskIcon,
            ),
          ),
        ),
        Expanded(
          flex: 4,
          child: Container(
            alignment: Alignment.center,
            child: Text(
              progressString,
              style: const TextStyle(
                fontSize: 54,
                fontWeight: FontWeight.bold,
                color: AppColors.lightText,
              ),
            ),
          ),
        ),
        Expanded(
          flex: 3,
          child: Container(
            alignment: Alignment.topCenter,
            padding: const EdgeInsets.symmetric(vertical: 10, horizontal: 40),
            child: Text(
              taskNameString,
              textAlign: TextAlign.center,
              style: const TextStyle(
                fontSize: 14,
                fontWeight: FontWeight.w500,
                color: AppColors.lightText,
              ),
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
    final leadingWidth = 0.16 * MediaQuery.of(context).size.width;
    final titleWidth = 0.60 * MediaQuery.of(context).size.width;
    const trailingWidth = 24.0;
    Widget? doneIcon;
    if (progress.currentBenchmark?.taskName == benchmarkInfo.taskName) {
      doneIcon = const InfiniteProgressCircle(
        size: trailingWidth,
        strokeWidth: 2,
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
      tileColor: isEven ? AppColors.mediumBlue : Colors.transparent,
      textColor: AppColors.lightText,
      dense: true,
      minVerticalPadding: 0,
      leading: SizedBox(
          width: leadingWidth * 0.4,
          height: leadingWidth * 0.4,
          child: benchmarkInfo.iconWhite),
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
        child: doneIcon,
      ),
    );
  }

  Widget _footer() {
    return Column(
      mainAxisAlignment: MainAxisAlignment.center,
      crossAxisAlignment: CrossAxisAlignment.center,
      children: [
        _footerText(),
        _cancelButton(),
      ],
    );
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
          backgroundColor:
              MaterialStateProperty.all(AppColors.progressCancelButton),
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
}
