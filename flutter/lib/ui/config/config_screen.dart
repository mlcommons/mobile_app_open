import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/state/task_list_manager.dart';

class ConfigScreen extends StatefulWidget {
  const ConfigScreen({Key? key}) : super(key: key);

  @override
  State<ConfigScreen> createState() => _ConfigScreen();
}

class _ConfigScreen extends State<ConfigScreen> {
  @override
  void dispose() {
    context.read<BenchmarkState>().saveTaskSelection();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final taskList = context.watch<TaskListManager>().taskList;
    final stringResources = AppLocalizations.of(context);
    final childrenList = <Widget>[];

    for (var benchmark in taskList.benchmarks) {
      childrenList.add(ListTile(
        title: Padding(
          padding: const EdgeInsets.only(bottom: 5),
          child: Text(
            benchmark.info.taskName,
          ),
        ),
        subtitle:
            Text('${benchmark.id} | ${benchmark.backendRequestDescription}'),
        leading: Checkbox(
            value: benchmark.isActive,
            onChanged: (bool? value) {
              setState(() {
                benchmark.isActive = value!;
              });
            }),
        onTap: () {
          setState(() {
            benchmark.isActive = !benchmark.isActive;
          });
        },
      ));
    }

    return Scaffold(
      appBar: AppBar(
        title: Text(
          stringResources.benchConfigTitle,
        ),
      ),
      body: ListView(
        padding: const EdgeInsets.only(top: 20),
        children: childrenList,
      ),
    );
  }
}
