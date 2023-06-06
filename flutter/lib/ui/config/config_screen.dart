import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/settings/about_screen.dart';

class ConfigScreen extends StatefulWidget {
  const ConfigScreen({Key? key}) : super(key: key);

  @override
  State<ConfigScreen> createState() => _ConfigScreen();
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

    childrenList.addAll([
      const Divider(),
      ListTile(
          title: Text(stringResources.settingsAbout),
          trailing: const Icon(Icons.chevron_right),
          onTap: () async {
            await Navigator.of(context).push(MaterialPageRoute(
              builder: (context) => const AboutScreen(),
            ));
          }),
    ]);

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
