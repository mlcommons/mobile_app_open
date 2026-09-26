import 'package:flutter/material.dart';

import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/app_styles.dart';

/// True once a benchmark has produced anything in this run.
///
/// A set fans its options out to more benchmarks than it shows, so the ones
/// left out have to read as skipped rather than as a result of "N/A".
bool benchmarkHasResult(Benchmark benchmark) =>
    benchmark.performanceModeResult != null ||
    benchmark.accuracyModeResult != null;

/// One benchmark set on the results screen.
///
/// The rows for benchmarks that ran are built by the caller, so this stays free
/// of the screen's performance/accuracy mode and can be rendered on its own.
class BenchmarkSetResultTile extends StatelessWidget {
  final BenchmarkSet benchmarkSet;
  final bool isExpanded;
  final VoidCallback onToggle;
  final Widget Function(Benchmark benchmark) resultRowBuilder;

  const BenchmarkSetResultTile({
    super.key,
    required this.benchmarkSet,
    required this.isExpanded,
    required this.onToggle,
    required this.resultRowBuilder,
  });

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context)!;
    final ran = benchmarkSet.benchmarks.where(benchmarkHasResult).toList();
    final skipped = benchmarkSet.benchmarks
        .where((b) => !benchmarkHasResult(b))
        .toList();

    return Column(
      children: [
        InkWell(
          onTap: onToggle,
          child: Padding(
            padding: const EdgeInsets.symmetric(vertical: 12, horizontal: 10),
            child: Row(
              children: [
                Container(
                  width: 40,
                  height: 40,
                  decoration: BoxDecoration(
                    color: AppColors.setIconBackground,
                    borderRadius: BorderRadius.circular(
                      WidgetSizes.borderRadius,
                    ),
                  ),
                  padding: const EdgeInsets.all(5),
                  child: benchmarkSet.info.icon,
                ),
                const SizedBox(width: 14),
                Expanded(
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      Text(
                        benchmarkSet.info.name,
                        style: const TextStyle(
                          fontWeight: FontWeight.bold,
                          fontSize: 16,
                        ),
                      ),
                      const SizedBox(height: 2),
                      Text(
                        l10n.resultsSetRanCount
                            .replaceAll('<count>', ran.length.toString())
                            .replaceAll(
                              '<total>',
                              benchmarkSet.benchmarks.length.toString(),
                            ),
                        style: const TextStyle(
                          fontSize: 12,
                          color: AppColors.mutedText,
                        ),
                      ),
                    ],
                  ),
                ),
                AnimatedRotation(
                  turns: isExpanded ? 0.5 : 0.0,
                  duration: const Duration(milliseconds: 200),
                  child: const Icon(
                    Icons.expand_more,
                    color: AppColors.subtleText,
                  ),
                ),
              ],
            ),
          ),
        ),
        AnimatedSwitcher(
          duration: const Duration(milliseconds: 200),
          transitionBuilder: (child, animation) => SizeTransition(
            sizeFactor: animation,
            alignment: Alignment.center,
            child: child,
          ),
          child: !isExpanded
              ? const SizedBox.shrink()
              : Container(
                  key: ValueKey('results_${benchmarkSet.config.id}'),
                  color: AppColors.panelBackground,
                  child: Column(
                    children: [
                      for (final benchmark in ran) resultRowBuilder(benchmark),
                      for (final benchmark in skipped)
                        _skippedRow(benchmark, l10n),
                    ],
                  ),
                ),
        ),
      ],
    );
  }

  Widget _skippedRow(Benchmark benchmark, AppLocalizations l10n) {
    return Padding(
      padding: const EdgeInsets.fromLTRB(10, 8, 14, 8),
      child: Row(
        children: [
          SizedBox(
            width: 40,
            height: 24,
            child: Opacity(opacity: 0.35, child: benchmark.info.icon),
          ),
          const SizedBox(width: 14),
          Expanded(
            child: Text(
              benchmark.info.taskName,
              style: const TextStyle(
                fontSize: 14,
                fontWeight: FontWeight.bold,
                color: AppColors.subtleText,
              ),
              overflow: TextOverflow.ellipsis,
            ),
          ),
          Text(
            l10n.resultsBenchmarkNotRun,
            style: const TextStyle(fontSize: 12, color: AppColors.subtleText),
          ),
        ],
      ),
    );
  }
}
