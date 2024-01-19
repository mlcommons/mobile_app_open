import 'dart:async';

import 'package:flutter/material.dart';

import 'package:mlperfbench/state/task_runner.dart';
import 'package:provider/provider.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
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

  @override
  Widget build(BuildContext context) {
    state = context.watch<BenchmarkState>();
    l10n = AppLocalizations.of(context);
    final progress = state.taskRunner.progressInfo;

    final backgroundGradient = BoxDecoration(
      gradient: LinearGradient(
        colors: AppColors.progressScreenGradient,
        begin: Alignment.topCenter,
        end: Alignment.bottomCenter,
      ),
    );
    final title = _title();
    final circle = _circle(progress);
    final namedIcon = _namedIcon(progress);
    final cancelButton = _cancelButton();

    return Scaffold(
      key: BenchmarkRunningScreen.scaffoldKey,
      body: Container(
        decoration: backgroundGradient,
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.center,
          mainAxisAlignment: MainAxisAlignment.spaceAround,
          children: [
            title,
            circle,
            namedIcon,
            cancelButton,
          ],
        ),
      ),
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

  Widget _namedIcon(ProgressInfo progress) {
    return Column(children: [
      Padding(
        padding: const EdgeInsets.fromLTRB(20, 10, 20, 20),
        child: SizedBox(
          width: 100,
          height: 100,
          child: progress.cooldown ? null : progress.info!.iconWhite,
        ),
      ),
      Padding(
        padding: const EdgeInsets.fromLTRB(20, 10, 20, 10),
        child: Text(
          progress.cooldown
              ? l10n.progressCooldown
              : (progress.accuracy
                      ? l10n.progressScreenNameAccuracy
                      : l10n.progressScreenNamePerformance)
                  .replaceFirst('<taskName>', progress.info!.taskName),
          style: const TextStyle(
            fontWeight: FontWeight.bold,
            color: AppColors.lightText,
          ),
        ),
      ),
      Text(
        progress.cooldown
            ? l10n.progressScreenCooldown.replaceAll(
                '<remaining>',
                formatDuration(
                    progress.cooldownDuration * (1.0 - progress.stageProgress)))
            : l10n.progressScreenStage.replaceFirst(
                '<percent>',
                (progress.stageProgress * 100)
                    .round()
                    .clamp(0, 100)
                    .toString()),
        style: const TextStyle(
          fontWeight: FontWeight.bold,
          color: AppColors.lightText,
        ),
      ),
    ]);
  }

  Widget _circle(ProgressInfo progress) {
    final containerWidth = 0.64 * MediaQuery.of(context).size.width;
    final progressCircleSize = Size(containerWidth + 40, containerWidth + 40);
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
              child: Text(
                '${progress.currentStage.toString()}/${progress.totalStages.toString()}',
                style: const TextStyle(
                  fontWeight: FontWeight.bold,
                  color: AppColors.lightText,
                ),
                textScaleFactor: 3,
              ),
            ),
          ),
        ),
        Center(child: ProgressCircles(progressCircleSize))
      ],
    );
  }

  Widget _title() {
    return Padding(
      padding: const EdgeInsets.fromLTRB(40, 80, 20, 40),
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
          ),
          Text(
            l10n.progressDontClose,
            style: const TextStyle(
              color: AppColors.lightText,
              fontSize: 17,
            ),
          ),
        ],
      ),
    );
  }

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
}
