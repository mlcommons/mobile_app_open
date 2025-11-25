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
        for (var benchmark in state.allBenchmarks) ...[
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
