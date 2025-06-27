import 'package:flutter/material.dart';

import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/auto_size_text.dart';
import 'package:mlperfbench/ui/home/app_drawer.dart';

class BenchmarkInfoButton extends StatelessWidget {
  final Benchmark benchmark;

  const BenchmarkInfoButton({super.key, required this.benchmark});

  @override
  Widget build(BuildContext context) {
    return IconButton(
      icon: const Icon(Icons.info_outline),
      onPressed: () {
        _showBottomSheet(context, benchmark);
      },
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
}
