import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';

class ConfigScreen extends StatefulWidget {
  const ConfigScreen({Key? key}) : super(key: key);

  @override
  _ConfigScreen createState() => _ConfigScreen();
}

class _ConfigScreen extends State<ConfigScreen> {
  late BenchmarkState state;

  @override
  void dispose() {
    state.saveTaskSelection();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    state = context.watch<BenchmarkState>();
    final stringResources = AppLocalizations.of(context);
    final childrenList = <Widget>[];

    for (var benchmark in state.benchmarks) {
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
