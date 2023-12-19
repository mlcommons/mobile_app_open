import 'package:flutter/material.dart';

import 'package:collection/collection.dart';
import 'package:provider/provider.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/home/benchmark_info_button.dart';

class BenchmarkConfigScreen extends StatefulWidget {
  const BenchmarkConfigScreen({Key? key}) : super(key: key);

  @override
  State<BenchmarkConfigScreen> createState() => _BenchmarkConfigScreen();
}

class _BenchmarkConfigScreen extends State<BenchmarkConfigScreen> {
  late BenchmarkState state;
  late AppLocalizations l10n;
  late double pictureEdgeSize;

  @override
  void dispose() {
    state.saveTaskSelection();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    state = context.watch<BenchmarkState>();
    l10n = AppLocalizations.of(context);
    pictureEdgeSize = 0.1 * MediaQuery.of(context).size.width;
    final childrenList = <Widget>[];

    for (var benchmark in state.benchmarks) {
      childrenList.add(_listTile(benchmark));
      childrenList.add(const Divider(height: 20));
    }

    return Scaffold(
      backgroundColor: Colors.transparent,
      appBar: PreferredSize(
        preferredSize: const Size.fromHeight(32.0),
        child: AppBar(
          shape: Border.all(color: AppColors.darBlue),
          backgroundColor: AppColors.darBlue,
          elevation: 0,
          title: _header(),
        ),
      ),
      body: Container(
        color: Colors.white,
        child: ListView(
          padding: const EdgeInsets.fromLTRB(0, 20, 0, 20),
          children: childrenList,
        ),
      ),
    );
  }

  Widget _header() {
    final selectedCount =
        state.benchmarks.where((e) => e.isActive).length.toString();
    final totalCount = state.benchmarks.length.toString();
    final title = l10n.mainScreenBenchmarkSelected
        .replaceAll('<selected>', selectedCount)
        .replaceAll('<total>', totalCount);
    return Text(title,
        style: Theme.of(context).textTheme.titleSmall!.copyWith(
              color: Theme.of(context).colorScheme.onPrimary,
            ));
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
      trailing: Row(
        mainAxisSize: MainAxisSize.min,
        mainAxisAlignment: MainAxisAlignment.end,
        children: [
          _activeToggle(benchmark),
          _infoButton(benchmark),
        ],
      ),
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

  Widget _infoButton(Benchmark benchmark) {
    return BenchmarkInfoButton(benchmark: benchmark);
  }

  Widget _delegateChoice(Benchmark benchmark) {
    final selected = benchmark.benchmarkSettings.delegateSelected;
    final choices = benchmark.benchmarkSettings.delegateChoice
        .sorted((b, a) => a.priority.compareTo(b.priority))
        .map((e) => e.delegateName)
        .toList();
    if (choices.isEmpty) {
      return const SizedBox();
    }
    if (choices.length == 1 && choices.first.isEmpty) {
      return const SizedBox();
    }
    if (!choices.contains(selected)) {
      throw 'delegate_selected=$selected must be one of delegate_choice=$choices';
    }
    final dropDownButton = DropdownButton<String>(
        underline: const SizedBox(),
        value: selected,
        items: choices
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
