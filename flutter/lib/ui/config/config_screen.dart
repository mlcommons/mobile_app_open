import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';

class ConfigScreen extends StatefulWidget {
  const ConfigScreen({Key? key}) : super(key: key);

  @override
  State<ConfigScreen> createState() => _ConfigScreen();
}

class _ConfigScreen extends State<ConfigScreen> {
  late BenchmarkState state;
  late double pictureEdgeSize;

  @override
  void dispose() {
    state.saveTaskSelection();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    state = context.watch<BenchmarkState>();
    pictureEdgeSize = 0.1 * MediaQuery.of(context).size.width;
    final stringResources = AppLocalizations.of(context);
    final childrenList = <Widget>[];

    for (var benchmark in state.benchmarks) {
      childrenList.add(_listTile(benchmark));
      childrenList.add(const Divider(height: 20));
    }

    return Scaffold(
      appBar: AppBar(
        title: Text(
          stringResources.benchConfigTitle,
        ),
      ),
      body: ListView(
        padding: const EdgeInsets.fromLTRB(8, 20, 8, 20),
        children: childrenList,
      ),
    );
  }

  Widget _listTile(Benchmark benchmark) {
    return ListTile(
      leading: SizedBox(
          width: pictureEdgeSize,
          height: pictureEdgeSize,
          child: benchmark.info.icon),
      title: _name(benchmark),
      subtitle: Column(
        mainAxisAlignment: MainAxisAlignment.start,
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          _description(benchmark),
          _delegateChoice(benchmark),
        ],
      ),
      trailing: _activeToggle(benchmark),
    );
  }

  Widget _name(Benchmark benchmark) {
    return Text(benchmark.info.taskName);
  }

  Widget _description(Benchmark benchmark) {
    return Padding(
      padding: const EdgeInsets.only(top: 8),
      child: Text(benchmark.backendRequestDescription),
    );
  }

  Widget _activeToggle(Benchmark benchmark) {
    return Switch(
      value: benchmark.isActive,
      onChanged: (flag) {
        setState(() {
          benchmark.isActive = flag;
        });
      },
    );
  }

  Widget _delegateChoice(Benchmark benchmark) {
    if (benchmark.benchmarkSettings.delegateChoice.isEmpty) {
      return const SizedBox();
    } else {
      final dropDownButton = DropdownButton<String>(
          underline: const SizedBox(),
          value: benchmark.benchmarkSettings.delegateSelected,
          items: benchmark.benchmarkSettings.delegateChoice
              .map((item) => DropdownMenuItem<String>(
                    value: item,
                    child: Text(item),
                  ))
              .toList(),
          onChanged: (value) => setState(() {
                benchmark.benchmarkSettings.delegateSelected = value!;
              }));
      return Row(
        mainAxisAlignment: MainAxisAlignment.start,
        children: [
          const Text('Delegate:'),
          const SizedBox(width: 4),
          dropDownButton,
        ],
      );
    }
  }
}
