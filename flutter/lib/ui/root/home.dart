import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/ui/root/resource_error_screen.dart';
import 'package:mlperfbench/ui/run/progress_screen.dart';
import 'package:mlperfbench/ui/run/result_screen.dart';
import 'package:mlperfbench/ui/run/start_screen.dart';

class MyHome extends StatelessWidget {
  const MyHome({Key? key}) : super(key: key);

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
