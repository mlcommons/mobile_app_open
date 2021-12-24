import 'dart:math';

import 'package:flutter/cupertino.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart' hide Icons;

import 'package:provider/provider.dart';

import 'package:mlcommons_ios_app/benchmark/benchmark.dart';
import 'package:mlcommons_ios_app/icons.dart';
import 'package:mlcommons_ios_app/localizations/app_localizations.dart';
import 'package:mlcommons_ios_app/ui/app_bar.dart';
import 'package:mlcommons_ios_app/ui/confirm_dialog.dart';
import 'package:mlcommons_ios_app/ui/error_dialog.dart';
import 'package:mlcommons_ios_app/ui/list_of_benchmark_items.dart';
import 'package:mlcommons_ios_app/ui/progress_screen.dart';
import 'package:mlcommons_ios_app/ui/result_screen.dart';

class MainKeys {
  // list of widget keys that need to be accessed in the test code
  static const String goButton = 'goButton';
}

class MyHomePage extends StatelessWidget {
  MyHomePage({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    final state = context.watch<BenchmarkState>();
    final stringResources = AppLocalizations.of(context);

    PreferredSizeWidget? appBar;

    switch (state.state) {
      case BenchmarkStateEnum.downloading:
      case BenchmarkStateEnum.waiting:
        appBar = MyAppBar.buildAppBar(stringResources.mainTitle, context, true);
        break;
      case BenchmarkStateEnum.aborting:
        appBar =
            MyAppBar.buildAppBar(stringResources.mainTitle, context, false);
        break;
      case BenchmarkStateEnum.cooldown:
      case BenchmarkStateEnum.running:
        return ProgressScreen();
      case BenchmarkStateEnum.done:
        return ResultScreen();
    }

    return Scaffold(
      appBar: appBar,
      body: SafeArea(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            Expanded(flex: 6, child: _getContainer(context, state.state)),
            Padding(
              padding: EdgeInsets.all(30),
              child: Text(stringResources.measureCapability,
                  style: TextStyle(fontSize: 16, color: Colors.black)),
            ),
            Expanded(
              flex: 5,
              child: Align(
                  alignment: Alignment.topCenter,
                  child: createListOfBenchmarkItemsWidgets(context, state)),
            ),
          ],
        ),
      ),
    );
  }

  Widget _getContainer(BuildContext context, BenchmarkStateEnum state) {
    if (state == BenchmarkStateEnum.aborting) {
      return _waitContainer(context);
    }

    if (state == BenchmarkStateEnum.waiting) {
      return _goContainer(context);
    }

    return _downloadContainer(context);
  }

  Widget _waitContainer(BuildContext context) {
    final stringResources = AppLocalizations.of(context);

    return _circleContainerWithContent(
        context, Icons.waiting, stringResources.waitBenchmarks);
  }

  Widget _goContainer(BuildContext context) {
    final state = context.watch<BenchmarkState>();
    final stringResources = AppLocalizations.of(context);

    return CustomPaint(
      painter: MyPaintBottom(),
      child: GoButtonGradient(() async {
        final wrongPathError = await state.validateExternalResourcesDirectory(
            stringResources.incorrectDatasetsPath);
        if (wrongPathError.isNotEmpty) {
          await showErrorDialog(context, [wrongPathError]);
          return;
        }
        final offlineError = await state
            .validateOfflineMode(stringResources.warningOfflineModeEnabled);
        if (offlineError.isNotEmpty) {
          switch (await showConfirmDialog(context, offlineError)) {
            case ConfirmDialogAction.ok:
              break;
            case ConfirmDialogAction.cancel:
              return;
            default:
              break;
          }
        }
        state.runBenchmarks();
      }),
    );
  }

  Widget _downloadContainer(BuildContext context) {
    final stringResources = AppLocalizations.of(context);
    final textLabel = Text(context.watch<BenchmarkState>().downloadingProgress,
        style: TextStyle(color: Colors.white, fontSize: 40));

    return _circleContainerWithContent(
        context, textLabel, stringResources.loadingContent);
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
        colors: [
          Color(0xFF31A3E2),
          Color(0xFF31A3E2),
          Color(0xFF31A3E2),
          Color(0xFF3189E2),
          Color(0xFF0B4A7F),
        ],
      ).createShader(rect);
    canvas.drawArc(rect, 0, pi, true, paint);
  } // paint

  @override
  bool shouldRepaint(MyPaintBottom old) => false;
}

class GoButtonGradient extends StatelessWidget {
  final AsyncCallback onPressed;

  GoButtonGradient(this.onPressed);

  @override
  Widget build(BuildContext context) {
    final stringResources = AppLocalizations.of(context);

    var decoration = BoxDecoration(
      shape: BoxShape.circle,
      gradient: LinearGradient(
        colors: [
          Color.lerp(Color(0xFF0DB526), Colors.white, 0.65)!,
          Color(0xFF0DB526), // 0DB526
        ],
        begin: Alignment.topCenter,
        end: Alignment.bottomCenter,
      ),
      boxShadow: [
        BoxShadow(
          color: Colors.black12,
          offset: Offset(15, 15),
          blurRadius: 10,
        )
      ],
    );

    return Container(
      decoration: decoration,
      width: MediaQuery.of(context).size.width * 0.35,
      child: MaterialButton(
        key: Key(MainKeys.goButton),
        materialTapTargetSize: MaterialTapTargetSize.shrinkWrap,
        splashColor: Colors.black,
        shape: CircleBorder(),
        onPressed: onPressed,
        child: Text(
          stringResources.go,
          style: TextStyle(
            color: Colors.white,
            fontSize: 40,
          ),
        ),
      ),
    );
  }
}

Widget _circleContainerWithContent(
    BuildContext context, Widget contentInCircle, String label) {
  return CustomPaint(
    painter: MyPaintBottom(),
    child: Stack(alignment: Alignment.topCenter, children: [
      Container(
        child: Padding(
          padding: EdgeInsets.all(20),
          child: Text(
            label,
            style: TextStyle(color: Colors.white, fontSize: 15),
          ),
        ),
      ),
      Stack(
        children: [
          Container(
            width: MediaQuery.of(context).size.width * 0.35,
            alignment: Alignment.center,
            decoration: BoxDecoration(
              shape: BoxShape.circle,
              color: Color(0xff135384),
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
