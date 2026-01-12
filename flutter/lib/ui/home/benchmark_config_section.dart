import 'package:flutter/material.dart';

import 'package:collection/collection.dart';
import 'package:provider/provider.dart';

import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/app_styles.dart';
import 'package:mlperfbench/ui/error_dialog.dart';
import 'package:mlperfbench/ui/home/benchmark_info_button.dart';
import 'package:mlperfbench/ui/nil.dart';

class BenchmarkConfigSection extends StatelessWidget {
  const BenchmarkConfigSection({super.key});

  @override
  Widget build(BuildContext context) {
    //Store store = context.watch<Store>();
    BenchmarkState state = context.watch<BenchmarkState>();
    AppLocalizations l10n = AppLocalizations.of(context)!;

    return ListView(
      padding: const EdgeInsets.fromLTRB(0, 8, 0, 20),
      children: <Widget>[
        for (var benchmarkSet in state.benchmarkSets) ...[
          _setListTile(benchmarkSet, state, l10n, context),
          if (benchmarkSet != state.benchmarkSets.last ||
              state.looseBenchmarks.isNotEmpty)
            const Divider(
              height: 1,
            )
        ],
        for (var benchmark in state.looseBenchmarks) ...[
          _listTile(benchmark, state, l10n),
          if (benchmark != state.allBenchmarks.last)
            const Divider(
              height: 1,
            )
        ],
        const SizedBox(height: 24)
      ],
    );
  }

  Widget _setDownloadStatus(AppLocalizations l10n, BenchmarkSet benchmarkSet,
      bool allResourcesExist, BuildContext context) {
    // Check if any benchmark in the set is currently active
    final hasActiveBenchmarks = benchmarkSet.benchmarks.any((b) => b.isActive);

    // If no benchmarks are active, or all resources already exist, show nothing
    if (!hasActiveBenchmarks || allResourcesExist) return const SizedBox();

    return InkWell(
        onTap: () async {
          // You can pass the benchmarkSet here to trigger downloads for all active items in the set
          await showResourceMissingDialog(context, [],
              benchmarkSet: benchmarkSet);
        },
        child: const Padding(
          padding: EdgeInsets.symmetric(horizontal: 8.0),
          child: Icon(
            Icons.downloading_rounded,
            size: 28,
            color: AppColors.warningIcon,
          ),
        ));
  }

  Widget _setListTile(BenchmarkSet benchmarkSet, BenchmarkState state,
      AppLocalizations l10n, BuildContext context) {
    final totalOptions = benchmarkSet.optionMap.length;
    final activeOptions = benchmarkSet.optionSets
        .expand((e) => e.options.values)
        .where((e) => e.enabled)
        .length;

    final bool isOptionsOpen = state.isOptionsExpanded(benchmarkSet);
    final bool isAdvancedOpen = state.isAdvancedConfigOpen(benchmarkSet);

    final Future<bool> resourcesExistFuture = Future.wait(
      benchmarkSet.benchmarks.where((b) => b.isActive).map(
            (b) => state.validator.validateAllResourcesExist(b,
                modes: state.taskRunner.selectedRunModes),
          ),
    ).then((results) => results.every((exists) => exists));

    return Column(
      children: [
        InkWell(
          onTap: () => state.toggleOptionsExpanded(benchmarkSet),
          child: Padding(
            padding: EdgeInsets.zero,
            child: Row(
              crossAxisAlignment: CrossAxisAlignment.center,
              children: [
                // LEADING BUTTON
                Padding(
                  padding: const EdgeInsets.only(bottom: 20, left: 16, top: 12),
                  child: SizedBox(
                    width: 40,
                    height: 40,
                    child: TextButton(
                      //TODO use benchmarkset's id
                      onPressed: () => showBenchInfoBottomSheet(
                          context, benchmarkSet.benchmarks[0]),
                      style: ElevatedButton.styleFrom(
                        backgroundColor: Colors.white,
                        padding: EdgeInsets.zero,
                        elevation: 3,
                        shape: RoundedRectangleBorder(
                            borderRadius: BorderRadius.circular(
                                WidgetSizes.borderRadius)),
                      ),
                      //TODO use benchmarkset's icon
                      child: SizedBox(
                          width: 32,
                          height: 32,
                          child: benchmarkSet.benchmarks[0].info.icon),
                    ),
                  ),
                ),
                const SizedBox(width: 14),

                // TITLE & SUBTITLE
                Expanded(
                  child: Padding(
                    padding: const EdgeInsets.only(bottom: 20, top: 12),
                    child: Column(
                      crossAxisAlignment: CrossAxisAlignment.start,
                      children: [
                        Text(
                          benchmarkSet.config.name,
                          style: const TextStyle(
                              fontWeight: FontWeight.bold, fontSize: 16),
                          overflow: TextOverflow.ellipsis,
                        ),
                        const SizedBox(height: 8),
                        Text(
                          '$activeOptions/$totalOptions options selected',
                          style:
                              const TextStyle(fontSize: 12, color: Colors.grey),
                        ),
                      ],
                    ),
                  ),
                ),

                // TRAILING
                FutureBuilder<bool>(
                  future: resourcesExistFuture,
                  initialData: true,
                  builder: (context, snapshot) => _setDownloadStatus(
                      l10n, benchmarkSet, snapshot.data!, context),
                ),
                Padding(
                  padding: const EdgeInsets.only(right: 8),
                  child: Column(
                    mainAxisSize: MainAxisSize.max,
                    mainAxisAlignment: MainAxisAlignment.start,
                    children: [
                      // Advanced Config Button
                      AnimatedRotation(
                        turns: isAdvancedOpen ? 0.5 : 0.0,
                        duration: const Duration(milliseconds: 200),
                        curve: Curves.fastOutSlowIn,
                        child: IconButton(
                          icon: Icon(Icons.settings,
                              color: isAdvancedOpen
                                  ? AppColors.secondary
                                  : Colors.grey),
                          onPressed: () =>
                              state.toggleAdvancedConfig(benchmarkSet),
                        ),
                      ),

                      AnimatedRotation(
                        turns: isOptionsOpen ? 0.5 : 0.0,
                        duration: const Duration(milliseconds: 200),
                        curve: Curves.fastOutSlowIn,
                        child: Icon(Icons.expand_more,
                            color: isOptionsOpen
                                ? AppColors.secondary
                                : Colors.grey),
                      ),
                    ],
                  ),
                )
              ],
            ),
          ),
        ),

        // --- THE BODY (Options List) ---
        AnimatedSwitcher(
          duration: const Duration(milliseconds: 200),
          switchInCurve: Curves.fastOutSlowIn,
          switchOutCurve: Curves.fastOutSlowIn,
          transitionBuilder: (child, animation) => SizeTransition(
              sizeFactor: animation, axisAlignment: 0.0, child: child),
          child: !isOptionsOpen
              ? const SizedBox.shrink()
              : Container(
                  key: const ValueKey('options_container'),
                  child: Column(
                    children: [
                      for (int i = 0;
                          i < benchmarkSet.optionSets.length;
                          i++) ...[
                        Padding(
                          padding: const EdgeInsets.only(
                              left: 16, top: 8, bottom: 4),
                          child: Row(
                            children: [
                              Text(
                                benchmarkSet.optionSets[i].config.name,
                                style: const TextStyle(
                                    fontWeight: FontWeight.bold,
                                    color: Colors.grey,
                                    fontSize: 12),
                              ),
                              const Expanded(
                                  child: Divider(indent: 10, endIndent: 16)),
                            ],
                          ),
                        ),
                        for (final option
                            in benchmarkSet.optionSets[i].options.keys)
                          ListTile(
                            dense: true,
                            contentPadding:
                                const EdgeInsets.only(left: 24, right: 16),
                            title: Text(option),
                            onTap: () {
                              state.benchmarkSetOption(
                                  benchmarkSet,
                                  option,
                                  !benchmarkSet.optionSets[
                                          benchmarkSet.optionMap[option]!]
                                      .getOption(option)!);
                            },
                            trailing: Checkbox(
                              key: Key(option),
                              value:
                                  benchmarkSet.optionSets[i].getOption(option),
                              onChanged: (bool? value) {
                                state.benchmarkSetOption(
                                    benchmarkSet, option, value!);
                              },
                            ),
                          ),
                      ]
                    ],
                  ),
                ),
        ),

        // --- ADVANCED CONFIG (Benchmarks List) ---
        AnimatedSwitcher(
          duration: const Duration(milliseconds: 200),
          switchInCurve: Curves.fastOutSlowIn,
          switchOutCurve: Curves.fastOutSlowIn,
          transitionBuilder: (child, animation) => SizeTransition(
              sizeFactor: animation, axisAlignment: 0.0, child: child),
          child: !isAdvancedOpen
              ? const SizedBox.shrink()
              : Container(
                  key: const ValueKey('advanced_config_container'),
                  width: double.infinity,
                  color: Colors.grey[100],
                  child: Column(
                    children: [
                      ...benchmarkSet.benchmarks.map((benchmark) {
                        return FutureBuilder(
                          future: state.validator.validateAllResourcesExist(
                              benchmark,
                              modes: state.taskRunner.selectedRunModes),
                          initialData: false,
                          builder: (context, snapshot) {
                            return Padding(
                              padding: const EdgeInsets.only(
                                  bottom: 12, left: 16, right: 8, top: 8),
                              child: Row(
                                children: [
                                  Container(
                                    width: 36,
                                    height: 36,
                                    decoration: BoxDecoration(
                                      color: Colors.white,
                                      borderRadius: BorderRadius.circular(
                                          WidgetSizes.borderRadius),
                                      boxShadow: const [
                                        BoxShadow(
                                            color: Colors.black12,
                                            blurRadius: 2)
                                      ],
                                    ),
                                    child: Padding(
                                      padding: const EdgeInsets.all(6),
                                      child: benchmark.info.icon,
                                    ),
                                  ),
                                  const SizedBox(width: 14),
                                  // Benchmark Metadata
                                  Expanded(
                                    child: Column(
                                      crossAxisAlignment:
                                          CrossAxisAlignment.start,
                                      children: [
                                        _nameDynamic(benchmark, context),
                                        Row(
                                          children: [
                                            Flexible(
                                                child: _backendDescription(
                                                    benchmark, context)),
                                            const SizedBox(
                                                height: 14,
                                                child: VerticalDivider(
                                                    color: Colors.black)),
                                            _delegateChoice(
                                                benchmark, context, state),
                                          ],
                                        ),
                                      ],
                                    ),
                                  ),
                                  // Individual Download Status
                                  _downloadStatus(
                                      l10n, benchmark, snapshot.data!, context),
                                ],
                              ),
                            );
                          },
                        );
                      }),
                      const SizedBox(height: 12),
                    ],
                  ),
                ),
        ),
      ],
    );
  }
  // Widget _setListTile(BenchmarkSet benchmarkSet, BenchmarkState state, AppLocalizations l10n) {
  //   return Column(children: [
  //     Text(benchmarkSet.config.name),
  //     const Divider(height: 0.5,),
  //     for (final option in benchmarkSet.optionMap.keys) ...[
  //       Row(
  //         children: [
  //           Text(option),
  //           Checkbox(key: Key(option), value: benchmarkSet.optionSets[benchmarkSet.optionMap[option]!].getOption(option), onChanged: (bool? value) {state.benchmarkSetOption(benchmarkSet, option, value!); })
  //         ],
  //       ),
  //     ]
  //   ],);
  // }

  Widget _listTile(
      Benchmark benchmark, BenchmarkState state, AppLocalizations l10n) {
    return FutureBuilder(
        future: state.validator.validateAllResourcesExist(benchmark,
            modes: state.taskRunner.selectedRunModes),
        initialData: false,
        builder: (context, snapshot) {
          return InkWell(
            onTap: () {
              state.benchmarkSetActive(benchmark, !benchmark.isActive);
            },
            child: Padding(
              padding: const EdgeInsets.only(
                  bottom: 20, left: 16, right: 8, top: 12),
              child: Row(
                crossAxisAlignment: CrossAxisAlignment.center,
                mainAxisSize: MainAxisSize.max,
                children: [
                  SizedBox(
                    width: 40,
                    height: 40,
                    child: TextButton(
                      onPressed: () {
                        showBenchInfoBottomSheet(context, benchmark);
                      },
                      style: ElevatedButton.styleFrom(
                        backgroundColor: Colors.white,
                        padding: const EdgeInsets.all(0.0),
                        elevation: 3,
                        shape: RoundedRectangleBorder(
                          borderRadius:
                              BorderRadius.circular(WidgetSizes.borderRadius),
                        ),
                      ),
                      child: SizedBox(
                          width: 32, height: 32, child: benchmark.info.icon),
                    ),
                  ),
                  const SizedBox(width: 14),
                  Expanded(
                    child: Column(
                      //mainAxisSize: MainAxisSize.min,
                      crossAxisAlignment: CrossAxisAlignment.start,
                      children: [
                        _name(benchmark, context),
                        const SizedBox(height: 10),
                        Row(
                          children: [
                            Flexible(
                              child: _backendDescription(benchmark, context),
                            ),
                            const SizedBox(
                              height: 18,
                              child: VerticalDivider(
                                color: Colors.black,
                              ),
                            ),
                            _delegateChoice(benchmark, context, state),
                          ],
                        ),
                      ],
                    ),
                  ),
                  _downloadStatus(l10n, benchmark, snapshot.data!, context),
                  _activeToggle(benchmark, state),
                ],
              ),
            ),
          );
        });
  }

  Widget _downloadStatus(AppLocalizations l10n, Benchmark benchmark,
      bool status, BuildContext context) {
    if (!benchmark.isActive || status) return const SizedBox();
    return InkWell(
        onTap: () async {
          await showResourceMissingDialog(context, [], benchmark: benchmark);
        },
        child: const Icon(Icons.downloading_rounded,
            size: 28, color: AppColors.warningIcon));
  }

  Widget _name(Benchmark benchmark, BuildContext context) {
    return Text(
      benchmark.info.taskName,
      style: Theme.of(context)
          .textTheme
          .titleMedium!
          .copyWith(fontWeight: FontWeight.bold),
    );
  }

  Widget _nameDynamic(Benchmark benchmark, BuildContext context) {
    return Text(
      benchmark.info.taskName,
      style: Theme.of(context).textTheme.titleMedium!.copyWith(
          color: benchmark.isActive ? AppColors.primary : Colors.black,
          fontWeight: FontWeight.bold),
    );
  }

  Widget _backendDescription(Benchmark benchmark, BuildContext context) {
    return Text(
      benchmark.backendRequestDescription,
      style: Theme.of(context).textTheme.labelLarge,
      maxLines: 1,
      overflow: TextOverflow.ellipsis,
      softWrap: false,
    );
  }

  Widget _activeToggle(Benchmark benchmark, BenchmarkState state) {
    return Switch(
      activeColor: AppColors.primary,
      value: benchmark.isActive,
      onChanged: (flag) {
        state.benchmarkSetActive(benchmark, flag);
      },
    );
  }

  Widget _delegateChoice(
      Benchmark benchmark, BuildContext context, BenchmarkState state) {
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
    return SizedBox(
      height: 24,
      child: DropdownButton<String>(
        isExpanded: false,
        isDense: false,
        padding: const EdgeInsets.only(left: 6),
        icon: const Icon(Icons.expand_more_rounded),
        borderRadius: BorderRadius.circular(WidgetSizes.borderRadius),
        underline: const SizedBox(),
        value: selected,
        items: choices
            .map((item) => DropdownMenuItem<String>(
                  value: item,
                  child: Text(
                    item,
                    style: Theme.of(context).textTheme.labelLarge,
                  ),
                ))
            .toList(),
        onChanged: (value) {
          state.benchmarkSetDelegate(benchmark, value ?? '');
        },
        style: Theme.of(context).textTheme.labelLarge,
      ),
    );
  }
}
