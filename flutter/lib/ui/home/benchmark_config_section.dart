import 'package:flutter/material.dart';

import 'package:collection/collection.dart';
import 'package:provider/provider.dart';

import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/app_styles.dart';
import 'package:mlperfbench/ui/auto_size_text.dart';
import 'package:mlperfbench/ui/home/app_drawer.dart';
import 'package:mlperfbench/ui/nil.dart';

class BenchmarkConfigSection extends StatelessWidget {
  const BenchmarkConfigSection({super.key});

  @override
  Widget build(BuildContext context) {
    //Store store = context.watch<Store>();
    BenchmarkState state = context.watch<BenchmarkState>();
    AppLocalizations l10n = AppLocalizations.of(context)!;
    final childrenList = <Widget>[
      for (var benchmark in state.allBenchmarks) ...[
        _listTile(benchmark, state, l10n),
        if (benchmark != state.allBenchmarks.last) const Divider(height: 16)
      ],
      const SizedBox(height: 24)
    ];

    return Container(
      color: Colors.white,
      child: ListView(
        padding: const EdgeInsets.fromLTRB(0, 8, 0, 20),
        children: childrenList,
      ),
    );
  }

  Widget _listTile(
      Benchmark benchmark, BenchmarkState state, AppLocalizations l10n) {
    //final leadingWidth = 0.10 * MediaQuery.of(context).size.width;
    //final subtitleWidth = 0.70 * MediaQuery.of(context).size.width;
    //final trailingWidth = 0.20 * MediaQuery.of(context).size.width;
    //const stat = 1;
    return FutureBuilder(
        future: state.validator.validateAllResourcesExist(benchmark,
            modes: state.taskRunner.selectedRunModes),
        initialData: false,
        builder: (context, snapshot) {
          return InkWell(
            onTap: () {
              _showBottomSheet(context, benchmark);
            },
            child: Padding(
              padding: const EdgeInsets.only(bottom: 8, left: 16, right: 8),
              child: Row(
                crossAxisAlignment: CrossAxisAlignment.center,
                mainAxisSize: MainAxisSize.max,
                children: [
                  SizedBox(
                    width: 32,
                    height: 32,
                    child: benchmark.info.icon,
                  ),
                  const SizedBox(width: 14),
                  Expanded(
                    child: Column(
                      //mainAxisSize: MainAxisSize.min,
                      crossAxisAlignment: CrossAxisAlignment.start,
                      children: [
                        _name(benchmark),
                        const SizedBox(height: 10),
                        Row(
                          children: [
                            _backendDescription(benchmark),
                            const SizedBox(width: 6),
                            _delegateChoice(benchmark, context, state),
                          ],
                        ),
                        _downloadStatus(l10n, snapshot.data!)
                      ],
                    ),
                  ),
                  _activeToggle(benchmark, state),
                ],
              ),
            ),
          );
        });
  }

  Widget _downloadStatus(AppLocalizations l10n, bool status) {
    if (status) return const SizedBox();
    return Padding(
      padding: const EdgeInsets.only(top: 8.0),
      child: Row(
        children: [
          const Icon(Icons.archive_rounded, size: 18, color: Colors.red),
          const SizedBox(width: 4),
          Text(
            l10n.resourceStatusNotDownloaded,
            style: const TextStyle(
              fontSize: 13,
              color: Colors.red,
              fontWeight: FontWeight.w500,
            ),
          ),
        ],
      ),
    );
  }

  Widget _name(Benchmark benchmark) {
    return Text(
      benchmark.info.taskName,
      style: const TextStyle(
        fontWeight: FontWeight.bold,
        fontSize: 16,
        color: Color(0xFF333333),
      ),
    );
  }

  Widget _backendDescription(Benchmark benchmark) {
    return Text(
      benchmark.backendRequestDescription,
      style: const TextStyle(
        fontSize: 14,
        color: Color(0xFF666666),
      ),
    );
  }

  Widget _activeToggle(Benchmark benchmark, BenchmarkState state) {
    return Transform.scale(
      scale: 1.25,
      child: Checkbox(
        activeColor: AppColors.primary,
        value: benchmark.isActive,
        shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(4)),
        onChanged: (flag) {
          state.benchmarkSetActive(benchmark, flag ?? false);
        },
      ),
    );
  }

  void _showBottomSheet(BuildContext context, Benchmark benchmark) {
    final l10n = AppLocalizations.of(context)!;

    final info = benchmark.info.getLocalizedInfo(l10n);

    const double sidePadding = 18.0;
    const double headHeight = 48.0 + (18.0 * 2);
    const double footHeight = 36.0;

    showModalBottomSheet(
      context: context,
      isDismissible: false,
      enableDrag: false,
      isScrollControlled: true,
      shape: const RoundedRectangleBorder(
          borderRadius: BorderRadius.vertical(top: Radius.circular(24))),
      builder: (context) => Padding(
        padding: const EdgeInsets.symmetric(horizontal: sidePadding),
        child: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            SizedBox(
              height: headHeight,
              width: MediaQuery.of(context).size.width - sidePadding,
              child: Center(
                child: Row(
                  mainAxisSize: MainAxisSize.max,
                  mainAxisAlignment: MainAxisAlignment.spaceBetween,
                  crossAxisAlignment: CrossAxisAlignment.center,
                  children: [
                    Expanded(
                      child: AutoSizeText(
                        benchmark.taskConfig.name,
                        overflow: TextOverflow.ellipsis,
                        maxLines: 1, //can be changed to 2 without issue
                        textAlign: TextAlign.left,
                        style: const TextStyle(
                          fontWeight: FontWeight.bold,
                          fontSize: 28,
                        ),
                      ),
                    ),
                    IconButton(
                      splashRadius: 24,
                      onPressed: () => Navigator.pop(context),
                      icon: const Icon(Icons.close, color: Colors.grey),
                    ),
                  ],
                ),
              ),
            ),
            LayoutBuilder(builder: (context, constraints) {
              print(constraints.maxHeight);
              return ConstrainedBox(
                constraints: constraints.copyWith(
                    maxHeight: constraints.maxHeight != double.infinity
                        ? constraints.maxHeight
                        : MediaQuery.of(context).size.height - headHeight),
                child: ScrollConfiguration(
                  behavior: NoGlowScrollBehavior(),
                  child: SingleChildScrollView(
                    child: Column(
                      children: [
                        Text(
                          info.detailsContent,
                          style: const TextStyle(fontSize: 16),
                        ),
                        const SizedBox(
                          height: footHeight,
                        )
                      ],
                    ),
                  ),
                ),
              );
            }),
          ],
        ),
      ),
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
    return Container(
      height: 32,
      decoration: BoxDecoration(
        border: Border.all(color: Colors.grey),
        borderRadius: BorderRadius.circular(6),
      ),
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
        style: const TextStyle(
          fontSize: 14,
          color: Colors.black,
        ),
      ),
    );
  }
}
