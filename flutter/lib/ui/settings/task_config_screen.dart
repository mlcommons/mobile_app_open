import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/resources/config_manager.dart';
import 'package:mlperfbench/store.dart';
import 'package:mlperfbench/ui/error_dialog.dart';

class TaskConfigScreen extends StatelessWidget {
  final List<TaskConfigDescription> _configs;

  const TaskConfigScreen(this._configs);

  Card getOptionPattern(
    BuildContext context,
    TaskConfigDescription configuration,
    String chosenConfigName,
  ) {
    final stringResources = AppLocalizations.of(context);
    final state = context.watch<BenchmarkState>();
    final wasChosen = chosenConfigName == configuration.name;

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
        title: Text(stringResources.taskConfigSettingsEntry),
      ),
      body: ListView.builder(
        itemCount: _configs.length,
        itemBuilder: (context, index) {
          final configuration = _configs[index];
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
