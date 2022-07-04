import 'dart:async';

import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/run/progress_circles.dart';

class ProgressScreen extends StatefulWidget {
  static final GlobalKey<ScaffoldState> scaffoldKey =
      GlobalKey<ScaffoldState>();

  const ProgressScreen({Key? key}) : super(key: key);

  @override
  _ProgressScreenState createState() => _ProgressScreenState();
}

class _ProgressScreenState extends State<ProgressScreen> {
  static const double progressCircleEdgeSize = 150;
  late final Timer _timer;

  @override
  Widget build(BuildContext context) {
    final state = context.watch<BenchmarkState>();
    final l10n = AppLocalizations.of(context);
    final progress = state.progressInfo;

    final backgroundGradient = BoxDecoration(
      gradient: LinearGradient(
        colors: AppColors.progressScreenGradient,
        begin: Alignment.topCenter,
        end: Alignment.bottomCenter,
      ),
    );
    final title = Padding(
      padding: const EdgeInsets.fromLTRB(40, 80, 20, 40),
      child: Column(
        mainAxisAlignment: MainAxisAlignment.start,
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text(
            l10n.measuring,
            style: const TextStyle(
              fontWeight: FontWeight.bold,
              color: AppColors.lightText,
              fontSize: 30,
            ),
          ),
          Text(
            l10n.dontCloseApp,
            style: const TextStyle(
              color: AppColors.lightText,
              fontSize: 17,
            ),
          ),
        ],
      ),
    );
    final circle = Stack(
      alignment: AlignmentDirectional.centerStart,
      children: <Widget>[
        Center(
          child: Container(
            width: progressCircleEdgeSize,
            height: progressCircleEdgeSize,
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
        const Center(
          child: ProgressCircles(
            Size(progressCircleEdgeSize + 40, progressCircleEdgeSize + 40),
          ),
        ),
      ],
    );
    final namedIcon = Column(children: [
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
              ? l10n.cooldownStatus
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
        l10n.progressScreenStage.replaceFirst('<percent>',
            (progress.stageProgress * 100).round().clamp(0, 100).toString()),
        style: const TextStyle(
          fontWeight: FontWeight.bold,
          color: AppColors.lightText,
        ),
      ),
    ]);
    final cancelButton = Padding(
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
        onPressed: () async => await state.abortBenchmarks(),
        child: Padding(
          padding: const EdgeInsets.fromLTRB(10, 0, 10, 0),
          child: Text(
            l10n.cancel,
            style: const TextStyle(
              fontSize: 15.0,
              color: AppColors.lightText,
            ),
          ),
        ),
      ),
    );

    return Scaffold(
      key: ProgressScreen.scaffoldKey,
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
