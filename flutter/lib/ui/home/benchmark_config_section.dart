import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/ui/error_dialog.dart';
import 'package:mlperfbench/ui/home/benchmark_info_button.dart';
import 'package:mlperfbench/ui/home/benchmark_loose_card.dart';
import 'package:mlperfbench/ui/home/benchmark_set_card.dart';

/// The list of benchmark sets and loose benchmarks on the start screen.
///
/// Kept free of [BenchmarkState] so the same list can be rendered in a widget
/// test; [BenchmarkConfigSection] is the adapter that wires it to state.
class BenchmarkConfigList extends StatelessWidget {
  final List<Widget> cards;

  const BenchmarkConfigList({super.key, required this.cards});

  @override
  Widget build(BuildContext context) {
    return ListView(
      padding: const EdgeInsets.fromLTRB(12, 12, 12, 20),
      children: [...cards, const SizedBox(height: 24)],
    );
  }
}

class BenchmarkConfigSection extends StatelessWidget {
  const BenchmarkConfigSection({super.key});

  @override
  Widget build(BuildContext context) {
    final state = context.watch<BenchmarkState>();

    return BenchmarkConfigList(
      cards: [
        for (final benchmarkSet in state.benchmarkSets)
          if (benchmarkSet.benchmarks.isNotEmpty)
            _setCard(benchmarkSet, state, context),
        for (final benchmark in state.looseBenchmarks)
          _looseCard(benchmark, state, context),
      ],
    );
  }

  Widget _setCard(
    BenchmarkSet benchmarkSet,
    BenchmarkState state,
    BuildContext context,
  ) {
    return BenchmarkSetCard(
      benchmarkSet: benchmarkSet,
      backendsOpen: state.isAdvancedConfigOpen(benchmarkSet),
      onToggleBackends: () => state.toggleAdvancedConfig(benchmarkSet),
      onInfoTap: () => showBenchmarkSetInfoBottomSheet(context, benchmarkSet),
      onDownloadTap: () async =>
          showResourceMissingDialog(context, [], benchmarkSet: benchmarkSet),
      resourcesExist: Future.wait(
        benchmarkSet.activeBenchmarks.map(
          (b) => state.validator.validateAllResourcesExist(
            b,
            modes: state.taskRunner.selectedRunModes,
          ),
        ),
      ).then((results) => results.every((exists) => exists)),
      onOptionChanged: (optionId, value) =>
          state.benchmarkSetOption(benchmarkSet, optionId, value),
      onBackendChanged: state.benchmarkSetBackend,
      onSetBackendChanged: (libName) =>
          state.benchmarkSetBackendForSet(benchmarkSet, libName),
      onDelegateChanged: state.benchmarkSetDelegate,
    );
  }

  Widget _looseCard(
    Benchmark benchmark,
    BenchmarkState state,
    BuildContext context,
  ) {
    return BenchmarkLooseCard(
      benchmark: benchmark,
      onInfoTap: () => showBenchInfoBottomSheet(context, benchmark),
      onDownloadTap: () async =>
          showResourceMissingDialog(context, [], benchmark: benchmark),
      resourcesExist: state.validator.validateAllResourcesExist(
        benchmark,
        modes: state.taskRunner.selectedRunModes,
      ),
      onActiveChanged: (isActive) =>
          state.benchmarkSetActive(benchmark, isActive),
      onBackendChanged: (libName) =>
          state.benchmarkSetBackend(benchmark, libName),
      onDelegateChanged: (delegate) =>
          state.benchmarkSetDelegate(benchmark, delegate),
    );
  }
}
