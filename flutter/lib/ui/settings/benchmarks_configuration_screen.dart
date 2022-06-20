import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/resources/config_manager.dart';
import 'package:mlperfbench/store.dart';
import 'package:mlperfbench/ui/error_dialog.dart';

class BenchmarksConfigurationScreen extends StatelessWidget {
  final List<TaskConfigDescription> _benchmarksConfigurations;

  BenchmarksConfigurationScreen(this._benchmarksConfigurations);

  Card getOptionPattern(
    BuildContext context,
    TaskConfigDescription configuration,
    String chosenBenchmarksConfiguration,
  ) {
    final stringResources = AppLocalizations.of(context);
    final state = context.watch<BenchmarkState>();
    final wasChosen = chosenBenchmarksConfiguration == configuration.name;

    return Card(
      child: ListTile(
        selected: wasChosen,
        title: Padding(
          padding: const EdgeInsets.only(bottom: 5),
          child: Text(configuration.name),
        ),
        subtitle: Text(configuration.path),
        trailing: Text(configuration.getType(stringResources)),
        onTap: () async {
          try {
            await state.setTaskConfig(name: configuration.name);
            Navigator.of(context).popUntil((route) => route.isFirst);
            await state.loadResources();
          } catch (e) {
            await showErrorDialog(
                context, <String>[stringResources.errorConfig, e.toString()]);
          }
        },
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    final stringResources = AppLocalizations.of(context);
    final store = context.watch<Store>();

    return Scaffold(
      appBar: AppBar(
        title: Text(stringResources.benchmarksConfigurationTitle),
      ),
      body: ListView.builder(
        itemCount: _benchmarksConfigurations.length,
        itemBuilder: (context, index) {
          final configuration = _benchmarksConfigurations[index];
          return getOptionPattern(
            context,
            configuration,
            store.chosenConfigurationName,
          );
        },
      ),
    );
  }
}
