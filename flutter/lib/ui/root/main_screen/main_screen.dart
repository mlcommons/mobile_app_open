import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/icons.dart';
import 'package:mlperfbench/ui/root/main_screen/downloading.dart';
import 'package:mlperfbench/ui/root/main_screen/ready.dart';
import 'package:mlperfbench/ui/root/main_screen/utils.dart';
import 'package:mlperfbench/ui/run/app_bar.dart';
import 'package:mlperfbench/ui/run/list_of_benchmark_items.dart';
import 'package:mlperfbench/ui/run/progress_screen.dart';
import 'package:mlperfbench/ui/run/result_screen.dart';
import '../resource_error_screen.dart';

class MyHomePage extends StatelessWidget {
  const MyHomePage({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    final state = context.watch<BenchmarkState>();
    final stringResources = AppLocalizations.of(context);

    if (state.taskConfigFailedToLoad) {
      return const ResourceErrorScreen();
    }

    PreferredSizeWidget? appBar;

    switch (state.state) {
      case BenchmarkStateEnum.downloading:
        return const MainScreenDownloading();
      case BenchmarkStateEnum.waiting:
        return const MainScreenReady();
      case BenchmarkStateEnum.aborting:
        appBar = MyAppBar.buildAppBar(
            stringResources.mainScreenTitle, context, false);
        break;
      case BenchmarkStateEnum.running:
        return const ProgressScreen();
      case BenchmarkStateEnum.done:
        return const ResultScreen();
    }

    return Scaffold(
      appBar: appBar,
      body: SafeArea(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            Expanded(flex: 6, child: _getContainer(context, state.state)),
            Padding(
              padding: const EdgeInsets.all(30),
              child: Text(
                stringResources.mainScreenMeasureTitle,
                style: const TextStyle(
                  fontSize: 16,
                  color: AppColors.darkText,
                ),
              ),
            ),
            Expanded(
              flex: 5,
              child: Align(
                  alignment: Alignment.topCenter,
                  child: createListOfBenchmarkItemsWidgets(context, state)),
            ),
          ],
        ),
      ),
    );
  }

  Widget _getContainer(BuildContext context, BenchmarkStateEnum state) {
    if (state == BenchmarkStateEnum.aborting) {
      return _waitContainer(context);
    }

    throw 'unexpected state';
  }

  Widget _waitContainer(BuildContext context) {
    final stringResources = AppLocalizations.of(context);

    return MainScreenUtils().circleContainerWithContent(
        context, AppIcons.waiting, stringResources.mainScreenWaitFinish);
  }
}
