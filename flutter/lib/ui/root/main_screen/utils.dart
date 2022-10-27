import 'dart:math' as math;

import 'package:flutter/material.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/run/list_of_benchmark_items.dart';

class MainScreenUtils {
  Widget wrapCircle(AppLocalizations l10n, PreferredSizeWidget appBar,
      Widget content, BuildContext context, List<Benchmark> tasks) {
    return Scaffold(
      appBar: appBar,
      body: SafeArea(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            Expanded(flex: 6, child: content),
            Padding(
              padding: const EdgeInsets.all(30),
              child: Text(
                l10n.mainScreenMeasureTitle,
                style: const TextStyle(
                  fontSize: 16,
                  color: AppColors.darkText,
                ),
              ),
            ),
            Expanded(
              flex: 5,
              child: Align(
                  alignment: Alignment.topCenter,
                  child: createListOfBenchmarkItemsWidgets(context, tasks)),
            ),
          ],
        ),
      ),
    );
  }

  Widget circleContainerWithContent(
      BuildContext context, Widget contentInCircle, String label) {
    return CustomPaint(
      painter: MyPaintBottom(),
      child: Stack(alignment: Alignment.topCenter, children: [
        Padding(
          padding: const EdgeInsets.all(20),
          child: Text(
            label,
            style: const TextStyle(color: AppColors.lightText, fontSize: 15),
          ),
        ),
        Stack(
          children: [
            Container(
              width: MediaQuery.of(context).size.width * 0.35,
              alignment: Alignment.center,
              decoration: const BoxDecoration(
                shape: BoxShape.circle,
                color: AppColors.progressCircle,
                boxShadow: [
                  BoxShadow(
                    color: Colors.black12,
                    offset: Offset(15, 15),
                    blurRadius: 10,
                  )
                ],
              ),
            ),
            Container(
              width: MediaQuery.of(context).size.width * 0.35,
              alignment: Alignment.center,
              child: contentInCircle,
            )
          ],
        )
      ]),
    );
  }
}

class MyPaintBottom extends CustomPainter {
  @override
  void paint(Canvas canvas, Size size) {
    final rect =
        Rect.fromCircle(center: Offset(size.width / 2, 0), radius: size.height);
    final paint = Paint()
      ..shader = LinearGradient(
        begin: Alignment.topLeft,
        end: Alignment.bottomLeft,
        colors: AppColors.mainScreenGradient,
      ).createShader(rect);
    canvas.drawArc(rect, 0, math.pi, true, paint);
  } // paint

  @override
  bool shouldRepaint(MyPaintBottom oldDelegate) => false;
}
