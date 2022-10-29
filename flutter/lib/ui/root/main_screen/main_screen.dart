import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/state/app_state.dart';
import 'package:mlperfbench/state/last_result_manager.dart';
import 'package:mlperfbench/ui/root/main_screen/aborting.dart';
import 'package:mlperfbench/ui/root/main_screen/downloading.dart';
import 'package:mlperfbench/ui/root/main_screen/ready.dart';
import 'package:mlperfbench/ui/run/progress_screen.dart';
import 'package:mlperfbench/ui/run/result_screen.dart';
import '../resource_error_screen.dart';

class MyHomePage extends StatelessWidget {
  const MyHomePage({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    final state = context.watch<AppState>();
    final lastResultManager = context.watch<LastResultManager>();

    switch (state.state) {
      case AppStateEnum.downloading:
        return const MainScreenDownloading();
      case AppStateEnum.ready:
        if (lastResultManager.value == null) {
          final showError = state.pendingError != null;
          return MainScreenReady(showError: showError);
        } else {
          return const ResultScreen();
        }
      case AppStateEnum.aborting:
        return const MainScreenAborting();
      case AppStateEnum.running:
        return const ProgressScreen();
      case AppStateEnum.resourceError:
        return const ResourceErrorScreen();
      default:
        throw 'unsupported app state';
    }
  }
}
