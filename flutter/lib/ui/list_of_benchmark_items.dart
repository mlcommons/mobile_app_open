import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';

import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/info.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';

Column createListOfBenchmarkItemsWidgets(
    BuildContext context, BenchmarkState state) {
  final stringResources = AppLocalizations.of(context);
  final list = <Widget>[];
  final pictureEdgeSize = 0.08 * MediaQuery.of(context).size.width;

  for (final benchmark in state.benchmarks) {
    list.add(
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
              padding: EdgeInsets.fromLTRB(40, 10, 20, 10),
              child: Container(
                width: pictureEdgeSize,
                height: pictureEdgeSize,
                child: benchmark.icon,
              ),
            ),
            Text(getBenchmarkName(
              benchmark,
              stringResources,
            )),
            Icon(
              Icons.chevron_right,
              color: Colors.grey,
            ),
          ],
        ),
      ),
    );
  }
  return Column(crossAxisAlignment: CrossAxisAlignment.start, children: list);
}

void showBenchmarkInfoBottomSheet(BuildContext context, Benchmark benchmark) {
  final stringResources = AppLocalizations.of(context);

  final info =
      BenchmarkInfoItem.getBenchmarkInfoItem(benchmark.code, stringResources) ??
          BenchmarkInfoItem.stub(benchmark.taskName);

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
                  padding: EdgeInsets.fromLTRB(20, 20, 20, 20),
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
                                  Text(info.title,
                                      textAlign: TextAlign.left,
                                      style: TextStyle(
                                          fontWeight: FontWeight.bold,
                                          fontSize: 28))
                                ])),
                        Flexible(
                            flex: 1,
                            child: Container(
                                alignment: Alignment.topRight,
                                child: IconButton(
                                    splashRadius: 24,
                                    onPressed: () => Navigator.pop(context),
                                    icon:
                                        Icon(Icons.close, color: Colors.grey))))
                      ])),
              Padding(
                  padding: EdgeInsets.fromLTRB(20, 0, 20, 20),
                  child: Text(info.details, style: TextStyle(fontSize: 16))),
            ],
          ));
}
