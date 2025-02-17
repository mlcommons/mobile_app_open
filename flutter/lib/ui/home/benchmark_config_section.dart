import 'package:flutter/material.dart';

import 'package:collection/collection.dart';
import 'package:provider/provider.dart';

import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/app_styles.dart';
import 'package:mlperfbench/ui/home/benchmark_info_button.dart';
import 'package:mlperfbench/ui/nil.dart';

class BenchmarkConfigSection extends StatefulWidget {
  const BenchmarkConfigSection({super.key});

  @override
  State<BenchmarkConfigSection> createState() => _BenchmarkConfigSectionState();
}

class _BenchmarkConfigSectionState extends State<BenchmarkConfigSection> {
  late BenchmarkState state;
  late AppLocalizations l10n;

  @override
  void dispose() {
    state.saveTaskSelection();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    state = context.watch<BenchmarkState>();
    l10n = AppLocalizations.of(context)!;
    final childrenList = <Widget>[];

    for (var benchmark in state.allBenchmarks) {
      childrenList.add(_listTile(benchmark));
      childrenList.add(const Divider(height: 20));
    }

    return Container(
      color: Colors.white,
      child: ListView(
        padding: const EdgeInsets.fromLTRB(0, 20, 0, 20),
        children: childrenList,
      ),
    );
  }

  Widget _listTile(Benchmark benchmark) {
    final leadingWidth = 0.10 * MediaQuery.of(context).size.width;
    final subtitleWidth = 0.70 * MediaQuery.of(context).size.width;
    final trailingWidth = 0.20 * MediaQuery.of(context).size.width;
    return ListTile(
      leading: SizedBox(
          width: leadingWidth,
          height: leadingWidth,
          child: Padding(
            padding: const EdgeInsets.all(4),
            child: benchmark.info.icon,
          )),
      title: _name(benchmark),
      subtitle: SizedBox(
        width: subtitleWidth,
        child: Column(
          mainAxisAlignment: MainAxisAlignment.start,
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            _backendDescription(benchmark),
            _delegateChoice(benchmark),
          ],
        ),
      ),
      trailing: SizedBox(
        width: trailingWidth,
        child: Row(
          mainAxisSize: MainAxisSize.min,
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            Expanded(flex: 2, child: _activeToggle(benchmark)),
            Expanded(flex: 1, child: _infoButton(benchmark)),
          ],
        ),
      ),
    );
  }

  Widget _name(Benchmark benchmark) {
    return Text(benchmark.info.taskName);
  }

  Widget _backendDescription(Benchmark benchmark) {
    return Padding(
      padding: const EdgeInsets.only(top: 8),
      child: Text(
        benchmark.backendRequestDescription,
        style: Theme.of(context).textTheme.labelLarge,
      ),
    );
  }

  Widget _activeToggle(Benchmark benchmark) {
    return Switch(
      value: benchmark.isActive,
      onChanged: (flag) {
        setState(() {
          benchmark.isActive = flag;
          state.notifyListeners();
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
      return nil;
    }
    if (choices.length == 1 && choices.first.isEmpty) {
      return nil;
    }
    if (!choices.contains(selected)) {
      throw 'delegate_selected=$selected must be one of delegate_choice=$choices';
    }
    return DropdownButton<String>(
      isExpanded: true,
      isDense: false,
      borderRadius: BorderRadius.circular(WidgetSizes.borderRadius),
      underline: const SizedBox(),
      value: selected,
      items: choices
          .map((item) => DropdownMenuItem<String>(
                value: item,
                child: Text(
                  'Delegate: $item',
                  style: Theme.of(context).textTheme.labelLarge,
                ),
              ))
          .toList(),
      onChanged: (value) => setState(() {
        benchmark.benchmarkSettings.delegateSelected = value!;
      }),
    );
  }
}
