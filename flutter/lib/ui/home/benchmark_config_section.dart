import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/app_styles.dart';
import 'package:mlperfbench/ui/error_dialog.dart';
import 'package:mlperfbench/ui/home/backend_choice.dart';
import 'package:mlperfbench/ui/home/benchmark_info_button.dart';
import 'package:mlperfbench/ui/home/benchmark_set_card.dart';

class BenchmarkConfigSection extends StatelessWidget {
  const BenchmarkConfigSection({super.key});

  @override
  Widget build(BuildContext context) {
    BenchmarkState state = context.watch<BenchmarkState>();
    AppLocalizations l10n = AppLocalizations.of(context)!;

    return ListView(
      padding: const EdgeInsets.fromLTRB(12, 12, 12, 20),
      children: <Widget>[
        for (var benchmarkSet in state.benchmarkSets)
          if (benchmarkSet.benchmarks.isNotEmpty)
            _setCard(benchmarkSet, state, context),
        for (var benchmark in state.looseBenchmarks)
          _looseCard(benchmark, state, l10n),
        const SizedBox(height: 24),
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
    AppLocalizations l10n,
  ) {
    return Container(
      margin: const EdgeInsets.only(bottom: 12),
      decoration: BoxDecoration(
        color: Colors.white,
        border: Border.all(color: AppColors.cardBorder),
        borderRadius: BorderRadius.circular(14),
      ),
      clipBehavior: Clip.antiAlias,
      child: FutureBuilder(
        future: state.validator.validateAllResourcesExist(
          benchmark,
          modes: state.taskRunner.selectedRunModes,
        ),
        initialData: false,
        builder: (context, snapshot) {
          return Padding(
            padding: const EdgeInsets.fromLTRB(14, 10, 6, 10),
            child: Row(
              children: [
                SizedBox(
                  width: 40,
                  height: 40,
                  child: TextButton(
                    onPressed: () =>
                        showBenchInfoBottomSheet(context, benchmark),
                    style: ElevatedButton.styleFrom(
                      backgroundColor: AppColors.setIconBackground,
                      padding: EdgeInsets.zero,
                      elevation: 0,
                      shape: RoundedRectangleBorder(
                        borderRadius: BorderRadius.circular(
                          WidgetSizes.borderRadius,
                        ),
                      ),
                    ),
                    child: SizedBox(
                      width: 28,
                      height: 28,
                      child: benchmark.info.icon,
                    ),
                  ),
                ),
                const SizedBox(width: 12),
                Expanded(
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      Text(
                        benchmark.info.taskName,
                        style: const TextStyle(
                          fontWeight: FontWeight.bold,
                          fontSize: 15,
                        ),
                        overflow: TextOverflow.ellipsis,
                      ),
                      const SizedBox(height: 4),
                      Row(
                        children: [
                          Flexible(
                            child: BackendChoice(
                              benchmark: benchmark,
                              onChanged: (libName) =>
                                  state.benchmarkSetBackend(benchmark, libName),
                            ),
                          ),
                          if (hasDelegateChoice(benchmark)) ...[
                            const SizedBox(
                              height: 16,
                              child: VerticalDivider(color: Colors.black26),
                            ),
                            DelegateChoice(
                              benchmark: benchmark,
                              onChanged: (delegate) => state
                                  .benchmarkSetDelegate(benchmark, delegate),
                            ),
                          ],
                        ],
                      ),
                    ],
                  ),
                ),
                _downloadStatus(l10n, benchmark, snapshot.data!, context),
                _activeToggle(benchmark, state),
              ],
            ),
          );
        },
      ),
    );
  }

  Widget _downloadStatus(
    AppLocalizations l10n,
    Benchmark benchmark,
    bool status,
    BuildContext context,
  ) {
    if (!benchmark.isActive || status) return const SizedBox.shrink();
    return InkWell(
      onTap: () async {
        await showResourceMissingDialog(context, [], benchmark: benchmark);
      },
      child: const Icon(
        Icons.downloading_rounded,
        size: 26,
        color: AppColors.warningIcon,
      ),
    );
  }

  Widget _activeToggle(Benchmark benchmark, BenchmarkState state) {
    return Switch(
      activeThumbColor: AppColors.primary,
      value: benchmark.isActive,
      onChanged: (flag) => state.benchmarkSetActive(benchmark, flag),
    );
  }
}
