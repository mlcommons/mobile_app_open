import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/resources/resource_manager.dart';
import 'package:mlperfbench/state/task_list_manager.dart';
import 'package:mlperfbench/ui/root/main_screen/utils.dart';
import 'package:mlperfbench/ui/run/app_bar.dart';

class MainScreenDownloading extends StatelessWidget {
  const MainScreenDownloading({super.key});

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context);
    final resourceManager = context.watch<ResourceManager>();
    final textLabel = Text(
      resourceManager.progress,
      style: const TextStyle(color: AppColors.lightText, fontSize: 40),
    );

    final utils = MainScreenUtils();

    final appBar = MyAppBar.buildAppBar(l10n.mainScreenTitle, context, true);
    final circle = utils.circleContainerWithContent(
        context, textLabel, l10n.mainScreenLoading);

    final taskList = context.watch<TaskListManager>().taskList;

    return utils.wrapCircle(l10n, appBar, circle, context, taskList.benchmarks);
  }
}
