import 'dart:io';

import 'package:flutter/material.dart';
import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'config_batch_screen.dart';

class ConfigScreen extends StatefulWidget {
  @override
  _ConfigScreen createState() => _ConfigScreen();
}

class _ConfigScreen extends State<ConfigScreen> {
  @override
  Widget build(BuildContext context) {
    final state = context.watch<BenchmarkState>();
    final stringResources = AppLocalizations.of(context);
    final childrenList = <Widget>[];

    for (var benchmark in state.benchmarks) {
      final item = benchmark.config;
      childrenList.add(ListTile(
        title: Padding(
          padding: const EdgeInsets.only(bottom: 5),
          child: Text(
            benchmark.info.taskName,
          ),
        ),
        subtitle:
            Text(benchmark.id + ' | ' + benchmark.backendRequestDescription),
        leading: Checkbox(
            value: item.active,
            onChanged: (bool? value) {
              setState(() {
                item.active = value == true ? true : false;
              });
            }),
        trailing: benchmark.info.isOffline && !Platform.isAndroid
            ? IconButton(
                icon: Icon(Icons.settings),
                tooltip: 'Batch settings',
                onPressed: () {
                  Navigator.push(
                    context,
                    MaterialPageRoute(
                        builder: (context) => ConfigBatchScreen(benchmark.id)),
                  );
                })
            : null,
        onTap: () {
          setState(() {
            item.active = !item.active;
          });
        },
      ));
    }

    return Scaffold(
      appBar: AppBar(
        title: Text(
          stringResources.configTitle,
        ),
      ),
      body: ListView(
        padding: const EdgeInsets.only(top: 20),
        children: childrenList,
      ),
    );
  }
}
