import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/run/progress_circles.dart';

class ProgressScreen extends StatefulWidget {
  @override
  _ProgressScreenState createState() => _ProgressScreenState();
}

class _ProgressScreenState extends State<ProgressScreen> {
  final double progressCircleEdgeSize = 150;

  @override
  Widget build(BuildContext context) {
    final state = context.watch<BenchmarkState>();
    final currentBenchmark = state.currentlyRunning!;
    final l10n = AppLocalizations.of(context);
    final coolingState = state.state == BenchmarkStateEnum.cooldown;

    final backgroundGradient = BoxDecoration(
      gradient: LinearGradient(
        colors: AppColors.progressScreenGradient,
        begin: Alignment.topCenter,
        end: Alignment.bottomCenter,
      ),
    );
    final title = Padding(
      padding: EdgeInsets.fromLTRB(40, 80, 20, 40),
      child: Column(
        mainAxisAlignment: MainAxisAlignment.start,
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text(
            l10n.measuring,
            style: TextStyle(
              fontWeight: FontWeight.bold,
              color: AppColors.lightText,
              fontSize: 30,
            ),
          ),
          Text(
            l10n.dontCloseApp,
            style: TextStyle(
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
              boxShadow: [
                BoxShadow(
                  color: Colors.black12,
                  offset: Offset(15, 15),
                  blurRadius: 10,
                )
              ],
            ),
            child: Center(
              child: Text(
                state.runningProgress,
                style: TextStyle(
                  fontWeight: FontWeight.bold,
                  color: AppColors.lightText,
                ),
                textScaleFactor: 3,
              ),
            ),
          ),
        ),
        Center(
          child: ProgressCircles(
            Size(progressCircleEdgeSize + 40, progressCircleEdgeSize + 40),
          ),
        ),
      ],
    );
    final namedIcon = Column(children: [
      Padding(
        padding: EdgeInsets.fromLTRB(20, 10, 20, 20),
        child: Container(
          width: 100,
          height: 100,
          child: coolingState ? null : currentBenchmark.info.iconWhite,
        ),
      ),
      Padding(
        padding: EdgeInsets.fromLTRB(20, 10, 20, 20),
        child: Text(
          coolingState ? l10n.cooldownStatus : currentBenchmark.info.taskName,
          style: TextStyle(
            fontWeight: FontWeight.bold,
            color: AppColors.lightText,
          ),
        ),
      ),
    ]);
    final cancelButton = Padding(
      padding: EdgeInsets.fromLTRB(20, 10, 20, 20),
      child: TextButton(
        style: ButtonStyle(
          backgroundColor:
              MaterialStateProperty.all(AppColors.progressCancelButton),
          shape: MaterialStateProperty.all(
            RoundedRectangleBorder(
              borderRadius: BorderRadius.circular(18.0),
              side: BorderSide(color: Colors.white),
            ),
          ),
        ),
        onPressed: () async => await state.abortBenchmarks(),
        child: Padding(
          padding: const EdgeInsets.fromLTRB(10, 0, 10, 0),
          child: Text(
            l10n.cancel,
            style: TextStyle(
              fontSize: 15.0,
              color: AppColors.lightText,
            ),
          ),
        ),
      ),
    );

    return Scaffold(
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
}
