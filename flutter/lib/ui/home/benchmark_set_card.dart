import 'package:flutter/material.dart';

import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/app_styles.dart';
import 'package:mlperfbench/ui/home/backend_choice.dart';

/// Sentinel for the set-wide backend control when its benchmarks disagree.
const String kMixedBackends = '__mixed__';

/// One benchmark set on the config screen.
///
/// Free of [BenchmarkState] so it can be rendered on its own in tests: every
/// mutation goes out through a callback.
class BenchmarkSetCard extends StatelessWidget {
  final BenchmarkSet benchmarkSet;
  final bool backendsOpen;
  final VoidCallback onToggleBackends;
  final VoidCallback onInfoTap;
  final VoidCallback onDownloadTap;

  /// Resolves once resource validation for the active benchmarks completes.
  final Future<bool> resourcesExist;

  final void Function(String optionId, bool value) onOptionChanged;
  final void Function(Benchmark benchmark, String libName) onBackendChanged;
  final void Function(String libName) onSetBackendChanged;
  final void Function(Benchmark benchmark, String delegate) onDelegateChanged;

  const BenchmarkSetCard({
    super.key,
    required this.benchmarkSet,
    required this.backendsOpen,
    required this.onToggleBackends,
    required this.onInfoTap,
    required this.onDownloadTap,
    required this.resourcesExist,
    required this.onOptionChanged,
    required this.onBackendChanged,
    required this.onSetBackendChanged,
    required this.onDelegateChanged,
  });

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context)!;
    final active = benchmarkSet.activeBenchmarks;

    return Container(
      margin: const EdgeInsets.only(bottom: 12),
      decoration: BoxDecoration(
        color: Colors.white,
        border: Border.all(color: AppColors.cardBorder),
        borderRadius: BorderRadius.circular(14),
      ),
      clipBehavior: Clip.antiAlias,
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          _header(active, l10n),
          if (active.isEmpty) _nothingSelected(l10n) else _runList(active),
          _options(l10n),
          _footer(active, l10n),
          AnimatedSwitcher(
            duration: const Duration(milliseconds: 200),
            switchInCurve: Curves.fastOutSlowIn,
            switchOutCurve: Curves.fastOutSlowIn,
            transitionBuilder: (child, animation) =>
                SizeTransition(sizeFactor: animation, child: child),
            child: !backendsOpen
                ? const SizedBox.shrink()
                : Container(
                    key: ValueKey('backends_${benchmarkSet.config.id}'),
                    child: _backends(context, l10n),
                  ),
          ),
        ],
      ),
    );
  }

  Widget _header(List<Benchmark> active, AppLocalizations l10n) {
    final summary = active.isEmpty
        ? l10n.mainScreenSetRunsNothing
        : l10n.mainScreenSetRunsCount
              .replaceAll('<count>', active.length.toString())
              .replaceAll('<total>', benchmarkSet.benchmarks.length.toString());

    return Padding(
      padding: const EdgeInsets.fromLTRB(14, 12, 6, 8),
      child: Row(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Container(
            width: 40,
            height: 40,
            decoration: BoxDecoration(
              color: AppColors.setIconBackground,
              borderRadius: BorderRadius.circular(WidgetSizes.borderRadius),
            ),
            padding: const EdgeInsets.all(6),
            child: benchmarkSet.info.icon,
          ),
          const SizedBox(width: 12),
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
                  overflow: TextOverflow.ellipsis,
                ),
                const SizedBox(height: 3),
                Text(
                  summary,
                  style: TextStyle(
                    fontSize: 12.5,
                    fontWeight: active.isEmpty
                        ? FontWeight.w600
                        : FontWeight.w500,
                    color: active.isEmpty
                        ? AppColors.warningText
                        : AppColors.mutedText,
                  ),
                ),
              ],
            ),
          ),
          IconButton(
            splashRadius: 22,
            tooltip: benchmarkSet.info.name,
            onPressed: onInfoTap,
            icon: const Icon(
              Icons.info_outline,
              size: 20,
              color: AppColors.mutedText,
            ),
          ),
        ],
      ),
    );
  }

  /// What the current selection actually produces. A set fans its options out
  /// to more benchmarks than it shows, so the card has to name them.
  Widget _runList(List<Benchmark> active) {
    return Padding(
      padding: const EdgeInsets.fromLTRB(66, 0, 14, 10),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          for (final benchmark in active)
            Padding(
              padding: const EdgeInsets.symmetric(vertical: 2),
              child: Row(
                children: [
                  Container(
                    width: 5,
                    height: 5,
                    decoration: const BoxDecoration(
                      color: AppColors.primary,
                      shape: BoxShape.circle,
                    ),
                  ),
                  const SizedBox(width: 8),
                  Expanded(
                    child: Text(
                      benchmark.info.taskName,
                      style: const TextStyle(fontSize: 13),
                      overflow: TextOverflow.ellipsis,
                    ),
                  ),
                  const SizedBox(width: 8),
                  Text(
                    chainLabel(benchmark),
                    style: const TextStyle(
                      fontSize: 11,
                      color: AppColors.mutedText,
                    ),
                  ),
                ],
              ),
            ),
        ],
      ),
    );
  }

  Widget _nothingSelected(AppLocalizations l10n) {
    return Container(
      margin: const EdgeInsets.fromLTRB(14, 0, 14, 10),
      padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 9),
      decoration: BoxDecoration(
        color: AppColors.warningBackground,
        border: Border.all(color: AppColors.warningBorder),
        borderRadius: BorderRadius.circular(9),
      ),
      child: Row(
        children: [
          const Icon(
            Icons.warning_amber_rounded,
            size: 18,
            color: AppColors.warningText,
          ),
          const SizedBox(width: 9),
          Expanded(
            child: Text(
              l10n.mainScreenSetNothingSelected,
              style: const TextStyle(
                fontSize: 12.5,
                fontWeight: FontWeight.w600,
                color: AppColors.warningText,
              ),
            ),
          ),
        ],
      ),
    );
  }

  /// Options are the primary control for a set, so they stay visible rather
  /// than hiding behind a disclosure.
  Widget _options(AppLocalizations l10n) {
    final availableIds = benchmarkSet
        .availableOptions()
        .map((e) => e.id)
        .toSet();
    final groups = benchmarkSet.optionSets
        .where((e) => !e.config.hidden)
        .where((e) => e.options.keys.any(availableIds.contains))
        .toList();
    if (groups.isEmpty) return const SizedBox.shrink();

    return Container(
      decoration: const BoxDecoration(
        border: Border(top: BorderSide(color: AppColors.cardDivider)),
      ),
      padding: const EdgeInsets.fromLTRB(14, 10, 14, 4),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          for (final optionSet in groups)
            Padding(
              padding: const EdgeInsets.only(bottom: 10),
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  Row(
                    crossAxisAlignment: CrossAxisAlignment.baseline,
                    textBaseline: TextBaseline.alphabetic,
                    children: [
                      Text(
                        optionSet.config.name.toUpperCase(),
                        style: const TextStyle(
                          fontSize: 11,
                          fontWeight: FontWeight.bold,
                          letterSpacing: 0.5,
                          color: AppColors.mutedText,
                        ),
                      ),
                      const SizedBox(width: 8),
                      Text(
                        optionSet.isSingleChoice
                            ? l10n.mainScreenOptionRulePickOne
                            : l10n.mainScreenOptionRuleAny,
                        style: const TextStyle(
                          fontSize: 11,
                          color: AppColors.subtleText,
                        ),
                      ),
                    ],
                  ),
                  const SizedBox(height: 8),
                  Wrap(
                    spacing: 8,
                    runSpacing: 8,
                    children: [
                      for (final option in optionSet.options.values)
                        if (availableIds.contains(option.id))
                          _optionChip(optionSet, option),
                    ],
                  ),
                ],
              ),
            ),
        ],
      ),
    );
  }

  Widget _optionChip(BenchmarkOptionSet optionSet, BenchmarkOption option) {
    // A set bounded to one choice reads as a radio group, not as checkboxes.
    final marker = optionSet.isSingleChoice
        ? (option.enabled ? Icons.radio_button_checked : Icons.radio_button_off)
        : (option.enabled ? Icons.check : null);

    return InkWell(
      key: Key(option.id),
      borderRadius: BorderRadius.circular(22),
      onTap: () => onOptionChanged(option.id, !option.enabled),
      child: Container(
        constraints: const BoxConstraints(minHeight: 44),
        // Generous side padding so a short label like "3B" does not collapse
        // into a circle at this corner radius.
        padding: EdgeInsets.only(left: marker == null ? 22 : 14, right: 22),
        decoration: BoxDecoration(
          color: option.enabled ? AppColors.primary : Colors.white,
          border: Border.all(
            color: option.enabled ? AppColors.primary : AppColors.chipBorder,
            width: 1.5,
          ),
          borderRadius: BorderRadius.circular(22),
        ),
        child: Row(
          mainAxisSize: MainAxisSize.min,
          children: [
            if (marker != null) ...[
              Icon(
                marker,
                size: 16,
                color: option.enabled ? Colors.white : AppColors.subtleText,
              ),
              const SizedBox(width: 6),
            ],
            Text(
              option.name,
              style: TextStyle(
                fontSize: 14,
                fontWeight: FontWeight.w600,
                color: option.enabled ? Colors.white : AppColors.bodyText,
              ),
            ),
          ],
        ),
      ),
    );
  }

  Widget _footer(List<Benchmark> active, AppLocalizations l10n) {
    return Container(
      decoration: const BoxDecoration(
        color: AppColors.cardFooter,
        border: Border(top: BorderSide(color: AppColors.cardDivider)),
      ),
      padding: const EdgeInsets.fromLTRB(14, 4, 6, 4),
      child: Row(
        children: [
          Expanded(
            child: FutureBuilder<bool>(
              future: resourcesExist,
              initialData: true,
              builder: (context, snapshot) {
                if (active.isEmpty || snapshot.data!) {
                  return const SizedBox.shrink();
                }
                return InkWell(
                  onTap: onDownloadTap,
                  child: Padding(
                    padding: const EdgeInsets.symmetric(vertical: 8),
                    child: Row(
                      children: [
                        const Icon(
                          Icons.downloading_rounded,
                          size: 20,
                          color: AppColors.warningText,
                        ),
                        const SizedBox(width: 8),
                        Flexible(
                          child: Text(
                            l10n.mainScreenSetFilesMissing,
                            style: const TextStyle(
                              fontSize: 12,
                              fontWeight: FontWeight.w600,
                              color: AppColors.warningText,
                            ),
                            overflow: TextOverflow.ellipsis,
                          ),
                        ),
                      ],
                    ),
                  ),
                );
              },
            ),
          ),
          TextButton(
            onPressed: onToggleBackends,
            style: TextButton.styleFrom(foregroundColor: AppColors.primary),
            child: Row(
              mainAxisSize: MainAxisSize.min,
              children: [
                Text(
                  l10n.mainScreenSetBackends,
                  style: const TextStyle(
                    fontSize: 12,
                    fontWeight: FontWeight.w600,
                  ),
                ),
                const SizedBox(width: 4),
                AnimatedRotation(
                  turns: backendsOpen ? 0.5 : 0.0,
                  duration: const Duration(milliseconds: 200),
                  child: const Icon(Icons.expand_more, size: 18),
                ),
              ],
            ),
          ),
        ],
      ),
    );
  }

  Widget _backends(BuildContext context, AppLocalizations l10n) {
    return Container(
      color: AppColors.panelBackground,
      padding: const EdgeInsets.fromLTRB(14, 10, 14, 12),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          _setWideBackend(context, l10n),
          for (final benchmark in benchmarkSet.benchmarks)
            _backendRow(benchmark, context, l10n),
        ],
      ),
    );
  }

  /// Writes one backend to every benchmark in the set that offers it. A set can
  /// hold six benchmarks, and picking the same backend six times is the common
  /// case; the per-benchmark rows below stay the override.
  Widget _setWideBackend(BuildContext context, AppLocalizations l10n) {
    final choices = setWideBackendChoices(benchmarkSet);
    if (choices.length <= 1) return const SizedBox.shrink();

    final selected = benchmarkSet.benchmarks
        .map((b) => b.selectedBackend.info.libName)
        .toSet();
    final value = selected.length == 1 ? selected.single : kMixedBackends;
    // Labels are per benchmark; merge them so a backend only some benchmarks
    // offer is still named.
    final labels = <String, String>{};
    for (final benchmark in benchmarkSet.benchmarks) {
      labels.addAll(backendChoiceLabels(benchmark));
    }

    return Container(
      margin: const EdgeInsets.only(bottom: 10),
      padding: const EdgeInsets.fromLTRB(10, 4, 6, 4),
      decoration: BoxDecoration(
        color: Colors.white,
        border: Border.all(color: AppColors.cardBorder),
        borderRadius: BorderRadius.circular(9),
      ),
      child: Row(
        children: [
          Expanded(
            child: Text(
              l10n.mainScreenSetBackendForAll,
              style: const TextStyle(fontSize: 12, fontWeight: FontWeight.w600),
            ),
          ),
          DropdownButton<String>(
            value: value,
            isDense: true,
            underline: const SizedBox(),
            borderRadius: BorderRadius.circular(WidgetSizes.borderRadius),
            icon: const Icon(Icons.expand_more_rounded),
            style: Theme.of(context).textTheme.labelLarge,
            items: [
              if (value == kMixedBackends)
                DropdownMenuItem<String>(
                  value: kMixedBackends,
                  child: Text(l10n.mainScreenSetBackendMixed),
                ),
              for (final libName in choices)
                DropdownMenuItem<String>(
                  value: libName,
                  child: Text(labels[libName] ?? libName),
                ),
            ],
            onChanged: (libName) {
              if (libName == null || libName == kMixedBackends) return;
              onSetBackendChanged(libName);
            },
          ),
        ],
      ),
    );
  }

  Widget _backendRow(
    Benchmark benchmark,
    BuildContext context,
    AppLocalizations l10n,
  ) {
    return Container(
      decoration: const BoxDecoration(
        border: Border(top: BorderSide(color: AppColors.panelDivider)),
      ),
      padding: const EdgeInsets.only(top: 8, bottom: 9),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Row(
            children: [
              Container(
                width: 6,
                height: 6,
                decoration: BoxDecoration(
                  color: benchmark.isActive
                      ? AppColors.primary
                      : AppColors.chipBorder,
                  shape: BoxShape.circle,
                ),
              ),
              const SizedBox(width: 8),
              Expanded(
                child: Text(
                  benchmark.info.taskName,
                  style: TextStyle(
                    fontSize: 13,
                    fontWeight: FontWeight.w600,
                    color: benchmark.isActive
                        ? AppColors.bodyText
                        : AppColors.subtleText,
                  ),
                  overflow: TextOverflow.ellipsis,
                ),
              ),
              if (!benchmark.isActive)
                Container(
                  padding: const EdgeInsets.symmetric(
                    horizontal: 5,
                    vertical: 1,
                  ),
                  decoration: BoxDecoration(
                    border: Border.all(color: AppColors.cardBorder),
                    borderRadius: BorderRadius.circular(5),
                  ),
                  child: Text(
                    l10n.mainScreenBenchmarkNotSelected,
                    style: const TextStyle(
                      fontSize: 10.5,
                      color: AppColors.subtleText,
                    ),
                  ),
                ),
            ],
          ),
          const SizedBox(height: 4),
          Padding(
            padding: const EdgeInsets.only(left: 14),
            child: Row(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                _labelledControl(
                  l10n.mainScreenBackendLabel,
                  BackendChoice(
                    benchmark: benchmark,
                    onChanged: (libName) =>
                        onBackendChanged(benchmark, libName),
                  ),
                ),
                if (hasDelegateChoice(benchmark)) ...[
                  const SizedBox(width: 18),
                  _labelledControl(
                    l10n.mainScreenDelegateLabel,
                    DelegateChoice(
                      benchmark: benchmark,
                      onChanged: (delegate) =>
                          onDelegateChanged(benchmark, delegate),
                    ),
                  ),
                ],
              ],
            ),
          ),
        ],
      ),
    );
  }

  Widget _labelledControl(String label, Widget control) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Text(
          label.toUpperCase(),
          style: const TextStyle(
            fontSize: 10,
            fontWeight: FontWeight.bold,
            letterSpacing: 0.4,
            color: AppColors.subtleText,
          ),
        ),
        const SizedBox(height: 2),
        control,
      ],
    );
  }
}

/// Every backend at least one benchmark in the set offers.
///
/// Not the intersection: a set is routinely heterogeneous — a vendor backend
/// may implement four of six LLM sizes — and an intersection would hide the
/// set-wide control in exactly that case. Applying one writes it wherever it
/// is supported and leaves the rest, which the per-benchmark rows below show
/// immediately, and the control then reads as mixed.
List<String> setWideBackendChoices(BenchmarkSet benchmarkSet) {
  final seen = <String>[];
  for (final benchmark in benchmarkSet.benchmarks) {
    for (final candidate in benchmark.backends) {
      if (!seen.contains(candidate.info.libName)) {
        seen.add(candidate.info.libName);
      }
    }
  }
  return seen;
}

String chainLabel(Benchmark benchmark) {
  final backend = benchmark.selectedBackend.settings.framework;
  final delegate = benchmark.benchmarkSettings.delegateSelected;
  if (delegate.isEmpty) return backend;
  return '$backend · $delegate';
}
