import 'dart:math';

import 'package:flutter/material.dart' hide Icons;
import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/resources/resource_manager.dart';
import 'package:mlperfbench/store.dart';

const maxBatchThreadsValue = 64;
const maxThreadsNumber = maxBatchThreadsValue ~/ 2;

class ConfigBatchScreen extends StatefulWidget {
  final String id;
  const ConfigBatchScreen(this.id);
  @override
  _ConfigBatchScreen createState() => _ConfigBatchScreen();
}

class _ConfigBatchScreen extends State<ConfigBatchScreen> {
  int _maxBatchSize = 0;
  BatchPreset? _selectedPreset;
  BatchPreset? _customPreset;

  @override
  Widget build(BuildContext context) {
    final store = context.watch<Store>();
    final state = context.watch<BenchmarkState>();
    final stringResources = AppLocalizations.of(context);
    final element = store
        .getBenchmarkList()
        .firstWhere((benchmark) => benchmark.id == widget.id);
    _maxBatchSize = maxBatchThreadsValue ~/ element.threadsNumber;

    var presets = state.resourceManager.getBatchPresets();
    _selectedPreset ??= element.batchPreset ?? presets[0];
    _customPreset ??= presets[1];

    final childrenList = <Widget>[
      ListTile(
        title: Padding(
          padding: const EdgeInsets.only(bottom: 5),
          child: Text(
            element.description,
            textAlign: TextAlign.center,
          ),
        ),
        subtitle: Text(stringResources.configBatchSubtitle
            .replaceAll('<maxValue>', maxBatchThreadsValue.toString())),
      ),
      Divider(),
      ListTile(
        title: Padding(
          padding: const EdgeInsets.only(bottom: 5),
          child: Text(
            stringResources.configThreadsNumber.replaceAll(
                '<threadsNumber>', element.threadsNumber.toString()),
          ),
        ),
        subtitle: Text(stringResources.configThreadsNumberSubtitle),
      ),
      Slider(
        value: (log(element.threadsNumber) / log(2) ~/ 1).toDouble(),
        min: 0,
        max: (log(maxThreadsNumber) / log(2) ~/ 1).toDouble(),
        divisions: (log(maxThreadsNumber) / log(2) ~/ 1),
        label: element.threadsNumber.toString(),
        onChanged: (double value) {
          setState(() {
            value = pow(2, value).toDouble();
            _maxBatchSize = maxBatchThreadsValue ~/ value;
            element.threadsNumber = value.toInt();
            element.batchSize = element.batchSize > _maxBatchSize
                ? _maxBatchSize
                : element.batchSize;
            _selectedPreset = _customPreset;
          });
        },
      ),
      Divider(),
      ListTile(
        title: Padding(
          padding: const EdgeInsets.only(bottom: 5),
          child: Text(
            stringResources.configBatchSize
                .replaceAll('<batchSize>', element.batchSize.toString()),
          ),
        ),
        subtitle: Text(stringResources.configBatchSizeSubtitle),
      ),
      Slider(
        value: (log(element.batchSize) / log(2) ~/ 1).toDouble(),
        min: 0,
        max: (log(_maxBatchSize) / log(2) ~/ 1).toDouble(),
        divisions: (log(_maxBatchSize) / log(2) ~/ 1),
        label: element.batchSize.toString(),
        onChanged: (double value) {
          setState(() {
            element.batchSize = pow(2, value).toInt();
            _selectedPreset = _customPreset;
          });
        },
      ),
      Divider(),
      ListTile(
        title: Padding(
          padding: const EdgeInsets.only(bottom: 5),
          child: Text(stringResources.configPreset),
        ),
        subtitle: Text(stringResources.configPresetSubtitle),
      ),
      Container(
        padding: const EdgeInsets.symmetric(horizontal: 12.0),
        height: 40.0,
        decoration: BoxDecoration(
          borderRadius: BorderRadius.circular(10.0),
          border: Border.all(color: Colors.blue),
        ),
        margin: const EdgeInsets.symmetric(horizontal: 12.0),
        child: DropdownButtonHideUnderline(
            child: DropdownButton(
          value: _selectedPreset,
          onChanged: (BatchPreset? newValue) {
            if (newValue == null) return;
            setState(() {
              _selectedPreset = newValue;
              element.batchPreset = newValue;
              if (newValue.name != 'Custom') {
                element.batchSize = newValue.batchSize;
                element.threadsNumber = newValue.shardsCount;
              }
            });
          },
          items: presets.map((preset) {
            return DropdownMenuItem(
              value: preset,
              child: Text(preset.name),
            );
          }).toList(),
        )),
      ),
    ];

    return Scaffold(
      appBar: AppBar(
        title: Text(stringResources.configBatchTitle),
      ),
      body: ListView(
        padding: const EdgeInsets.only(top: 20),
        children: childrenList,
      ),
    );
  }
}
