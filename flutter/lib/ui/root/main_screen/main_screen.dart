import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/benchmark/state.dart';
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
    final state = context.watch<BenchmarkState>();
    final lastResultManager = context.watch<LastResultManager>();

    switch (state.state) {
      case BenchmarkStateEnum.downloading:
        return const MainScreenDownloading();
      case BenchmarkStateEnum.ready:
        if (lastResultManager.value == null) {
          return const MainScreenReady();
        } else {
          return const ResultScreen();
        }
      case BenchmarkStateEnum.aborting:
        return const MainScreenAborting();
      case BenchmarkStateEnum.running:
        return const ProgressScreen();
      case BenchmarkStateEnum.resourceError:
        return const ResourceErrorScreen();
      default:
        throw 'unsupported app state';
    }
  }
}
