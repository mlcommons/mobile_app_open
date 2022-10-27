import 'package:flutter/material.dart';

import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';

ListView createListOfBenchmarkItemsWidgets(
    BuildContext context, List<Benchmark> tasks) {
  final result = <Widget>[];
  final pictureEdgeSize = 0.08 * MediaQuery.of(context).size.width;

  for (final benchmark in tasks) {
    result.add(
      InkWell(
        onTap: () => showBenchmarkInfoBottomSheet(
          context,
          benchmark,
        ),
        child: Row(
          mainAxisSize: MainAxisSize.min,
          mainAxisAlignment: MainAxisAlignment.start,
          children: [
            Padding(
              padding: const EdgeInsets.fromLTRB(40, 10, 20, 10),
              child: SizedBox(
                width: pictureEdgeSize,
                height: pictureEdgeSize,
                child: benchmark.info.icon,
              ),
            ),
            Text(benchmark.taskConfig.name),
            const Icon(
              Icons.chevron_right,
              color: Colors.grey,
            ),
          ],
        ),
      ),
    );
  }
  return ListView(padding: const EdgeInsets.only(left: 20), children: result);
}

void showBenchmarkInfoBottomSheet(BuildContext context, Benchmark benchmark) {
  final stringResources = AppLocalizations.of(context);

  final info = benchmark.info.getLocalizedInfo(stringResources);

  showModalBottomSheet(
    context: context,
    isDismissible: false,
    enableDrag: false,
    isScrollControlled: true,
    backgroundColor: Colors.white.withOpacity(0.8),
    shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(30)),
    builder: (context) => Wrap(
      children: [
        Padding(
          padding: const EdgeInsets.fromLTRB(20, 20, 20, 20),
          child: Row(
            mainAxisAlignment: MainAxisAlignment.spaceBetween,
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              Flexible(
                flex: 5,
                child: Column(
                  mainAxisAlignment: MainAxisAlignment.start,
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Text(
                      benchmark.taskConfig.name,
                      textAlign: TextAlign.left,
                      style: const TextStyle(
                        fontWeight: FontWeight.bold,
                        fontSize: 28,
                      ),
                    ),
                  ],
                ),
              ),
              Flexible(
                flex: 1,
                child: Container(
                  alignment: Alignment.topRight,
                  child: IconButton(
                    splashRadius: 24,
                    onPressed: () => Navigator.pop(context),
                    icon: const Icon(Icons.close, color: Colors.grey),
                  ),
                ),
              ),
            ],
          ),
        ),
        Padding(
          padding: const EdgeInsets.fromLTRB(20, 0, 20, 20),
          child: Text(
            info.detailsContent,
            style: const TextStyle(fontSize: 16),
          ),
        ),
      ],
    ),
  );
}
