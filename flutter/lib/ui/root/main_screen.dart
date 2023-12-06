import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/ui/home/benchmark_running_screen.dart';
import 'package:mlperfbench/ui/home/benchmark_start_screen.dart';
import 'package:mlperfbench/ui/home/resource_loading_screen.dart';
import 'package:mlperfbench/ui/home/result_screen.dart';
import 'package:mlperfbench/ui/root/resource_error_screen.dart';

class MainScreen extends StatelessWidget {
  const MainScreen({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    final state = context.watch<BenchmarkState>();

    if (state.taskConfigFailedToLoad) {
      return const ResourceErrorScreen();
    }

    switch (state.state) {
      case BenchmarkStateEnum.downloading:
        return const ResourceLoadingScreen();
      case BenchmarkStateEnum.waiting:
        return const BenchmarkStartScreen();
      case BenchmarkStateEnum.aborting:
        return const BenchmarkStartScreen();
      case BenchmarkStateEnum.running:
        return const BenchmarkRunningScreen();
      case BenchmarkStateEnum.done:
        return const ResultScreen();
    }
  }
}
