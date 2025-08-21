import 'dart:io';

import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/resources/config_manager.dart';
import 'package:mlperfbench/store.dart';
import 'package:mlperfbench/ui/error_dialog.dart';

class TaskConfigSection extends StatelessWidget {
  final List<TaskConfigDescription> _configs;

  const TaskConfigSection(this._configs, {super.key});

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context)!;
    final store = context.watch<Store>();
    final state = context.watch<BenchmarkState>();
    List<Widget> items = [];
    items.add(ListTile(
      leading: Text(
        l10n.settingsTaskConfigTitle,
        style: Theme.of(context).textTheme.headlineSmall,
      ),
    ));
    items.addAll(
      _configs.map(
        (c) => _getConfigChoice(context, c, store.chosenConfigurationName),
      ),
    );
    String dataFolder;
    if (Platform.isIOS) {
      dataFolder = 'Files/MLPerf Mobile/';
    } else {
      dataFolder = state.resourceManager.getDataFolder();
    }
    final subtitle = '${state.resourceManager.getDataPrefix()} = $dataFolder';
    items.addAll([
      const Divider(),
      ListTile(
          title: Text(l10n.settingsTaskDataFolderTitle),
          subtitle: Text(subtitle)),
    ]);
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: items,
    );
  }

  Widget _getConfigChoice(
    BuildContext context,
    TaskConfigDescription configuration,
    String chosenConfigName,
  ) {
    final l10n = AppLocalizations.of(context)!;
    final state = context.watch<BenchmarkState>();
    final isSelected = chosenConfigName == configuration.name;

    return AbsorbPointer(
      absorbing: _configs.length <= 1,
      child: ListTile(
        selected: isSelected,
        title: Padding(
          padding: const EdgeInsets.only(bottom: 5),
          child: Text(configuration.name),
        ),
        subtitle: Text(configuration.path),
        trailing: Text(configuration.getType(l10n)),
        onTap: () async {
          try {
            await state.setTaskConfig(name: configuration.name);
            if (!context.mounted) return;
            Navigator.of(context).popUntil((route) => route.isFirst);
            await state.loadResources(downloadMissing: false);
          } catch (e) {
            if (!context.mounted) return;
            await showErrorDialog(
              context,
              <String>[l10n.settingsTaskConfigError, e.toString()],
            );
          }
        },
      ),
    );
  }
}

class TaskConfigErrorScreen extends StatelessWidget {
  final List<TaskConfigDescription> configs;

  const TaskConfigErrorScreen({required this.configs, super.key});

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context)!;
    return Scaffold(
      appBar: AppBar(
        title: Text(l10n.resourceErrorSelectTaskFile),
      ),
      body: TaskConfigSection(configs),
    );
  }
}
