import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/state/task_list_manager.dart';
import 'package:mlperfbench/ui/icons.dart';
import 'package:mlperfbench/ui/root/main_screen/utils.dart';
import 'package:mlperfbench/ui/run/app_bar.dart';

class MainScreenAborting extends StatelessWidget {
  const MainScreenAborting({super.key});

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context);

    final utils = MainScreenUtils();

    final appBar = MyAppBar.buildAppBar(l10n.mainScreenTitle, context, true);
    final circle = utils.circleContainerWithContent(
        context, AppIcons.waiting, l10n.mainScreenWaitFinish);

    final taskList = context.watch<TaskListManager>().taskList;

    return utils.wrapCircle(l10n, appBar, circle, context, taskList.benchmarks);
  }
}
