import 'dart:async';

import 'package:flutter/material.dart';

import 'package:collection/collection.dart';
import 'package:provider/provider.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/benchmark/benchmark.dart';
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
    progress.info = state.benchmarks.last.info;
    progress.currentStage = 2;
    progress.calculateStageProgress = () {
      return 0.44;
    };
    for (var e in state.benchmarks) {
      e.isActive = true;
    }
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
            Expanded(flex: 40, child: _taskList()),
            Expanded(flex: 16, child: _footer()),
          ],
        ),
      ),
    );
  }

  Widget _title() {
    return Padding(
      padding: const EdgeInsets.fromLTRB(40, 40, 40, 4),
      child: Column(
        mainAxisAlignment: MainAxisAlignment.start,
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text(
            l10n.progressMeasuring,
            style: const TextStyle(
              fontWeight: FontWeight.bold,
              color: AppColors.lightText,
              fontSize: 30,
            ),
          )
        ],
      ),
    );
  }

  Widget _circle() {
    final containerWidth = 0.68 * MediaQuery.of(context).size.width;
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
            size: containerWidth,
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
      taskNameString = progress.info!.taskName;
      taskIcon = progress.info!.iconWhite;
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
                fontSize: 60,
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
    state.benchmarks.where((e) => e.isActive).forEachIndexed((idx, benchmark) {
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

  Widget _listTile(Benchmark benchmark, bool isEven) {
    final leadingWidth = 0.16 * MediaQuery.of(context).size.width;
    final titleWidth = 0.60 * MediaQuery.of(context).size.width;
    final trailingWidth = 0.16 * MediaQuery.of(context).size.width;
    var doneIcon = const Icon(Icons.done);
    if (progress.info!.taskName == benchmark.info.taskName) {
      doneIcon = const Icon(Icons.done_outline_outlined, color: Colors.green);
    }
    return ListTile(
      tileColor: isEven ? AppColors.mediumBlue : Colors.transparent,
      textColor: AppColors.lightText,
      dense: true,
      minVerticalPadding: 0,
      leading: SizedBox(
          width: leadingWidth * 0.4,
          height: leadingWidth * 0.4,
          child: benchmark.info.iconWhite),
      title: SizedBox(
        width: titleWidth,
        child: Text(
          benchmark.info.taskName,
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
