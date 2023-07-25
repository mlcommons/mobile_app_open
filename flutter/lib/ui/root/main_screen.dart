import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/ui/home/progress_screen.dart';
import 'package:mlperfbench/ui/home/result_screen.dart';
import 'package:mlperfbench/ui/home/start_screen.dart';
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
        return const StartScreen();
      case BenchmarkStateEnum.waiting:
        return const StartScreen();
      case BenchmarkStateEnum.aborting:
        return const StartScreen();
      case BenchmarkStateEnum.running:
        return const ProgressScreen();
      case BenchmarkStateEnum.done:
        return const ResultScreen();
    }
  }
}
