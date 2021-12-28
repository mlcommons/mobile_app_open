import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:mlcommons_ios_app/benchmark/benchmark.dart';
import 'package:mlcommons_ios_app/resources/configurations_manager.dart';
import 'package:mlcommons_ios_app/localizations/app_localizations.dart';
import 'package:mlcommons_ios_app/store.dart';
import 'package:mlcommons_ios_app/ui/error_dialog.dart';

class BenchmarksConfigurationScreen extends StatelessWidget {
  final List<BenchmarksConfig> _benchmarksConfigurations;

  BenchmarksConfigurationScreen(this._benchmarksConfigurations);

  Card getOptionPattern(
    BuildContext context,
    BenchmarksConfig configuration,
    String chosenBenchmarksConfiguration,
  ) {
    final store = context.watch<Store>();
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
          final configFile = await state.handleChosenConfiguration(
            newChosenConfiguration: !wasChosen ? configuration : null,
            store: store,
          );

          if (configFile != null) {
            Navigator.of(context).popUntil((route) => route.isFirst);
            await state.loadResources(configFile);
          } else {
            await showErrorDialog(
                context, <String>[stringResources.errorConfig]);
          }
        },
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    final stringResources = AppLocalizations.of(context);
    final state = context.watch<BenchmarkState>();
    final chosenConfiguration = state.chosenBenchmarksConfiguration;

    return Scaffold(
      appBar: AppBar(
        title: Text(stringResources.benchmarksConfigurationTitle),
      ),
      body: FutureBuilder<BenchmarksConfig?>(
        future: chosenConfiguration,
        builder: (context, snapshot) => ListView.builder(
            itemCount: _benchmarksConfigurations.length,
            itemBuilder: (context, index) {
              final configuration = _benchmarksConfigurations[index];
              return getOptionPattern(
                context,
                configuration,
                snapshot.data?.name ?? '',
              );
            }),
      ),
    );
  }
}
