import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/benchmark/state.dart';
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

    if (state.taskConfigFailedToLoad) {
      return const ResourceErrorScreen();
    }

    switch (state.state) {
      case BenchmarkStateEnum.downloading:
        return const MainScreenDownloading();
      case BenchmarkStateEnum.waiting:
        return const MainScreenReady();
      case BenchmarkStateEnum.aborting:
        return const MainScreenAborting();
      case BenchmarkStateEnum.running:
        return const ProgressScreen();
      case BenchmarkStateEnum.done:
        return const ResultScreen();
      default:
        throw 'unsupported app state';
    }
  }
}
