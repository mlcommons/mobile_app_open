import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/store.dart';
import 'package:mlperfbench/ui/confirm_dialog.dart';
import 'package:mlperfbench/ui/error_dialog.dart';
import 'package:mlperfbench/ui/icons.dart';
import 'package:mlperfbench/ui/root/main_screen/downloading.dart';
import 'package:mlperfbench/ui/root/main_screen/utils.dart';
import 'package:mlperfbench/ui/run/app_bar.dart';
import 'package:mlperfbench/ui/run/list_of_benchmark_items.dart';
import 'package:mlperfbench/ui/run/progress_screen.dart';
import 'package:mlperfbench/ui/run/result_screen.dart';
import '../resource_error_screen.dart';

class MainKeys {
  // list of widget keys that need to be accessed in the test code
  static const String goButton = 'goButton';
}

class MyHomePage extends StatelessWidget {
  const MyHomePage({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    final state = context.watch<BenchmarkState>();
    final stringResources = AppLocalizations.of(context);

    if (state.taskConfigFailedToLoad) {
      return const ResourceErrorScreen();
    }

    PreferredSizeWidget? appBar;

    switch (state.state) {
      case BenchmarkStateEnum.downloading:
        return const MainScreenDownloading();
      case BenchmarkStateEnum.waiting:
        appBar = MyAppBar.buildAppBar(
            stringResources.mainScreenTitle, context, true);
        break;
      case BenchmarkStateEnum.aborting:
        appBar = MyAppBar.buildAppBar(
            stringResources.mainScreenTitle, context, false);
        break;
      case BenchmarkStateEnum.running:
        return const ProgressScreen();
      case BenchmarkStateEnum.done:
        return const ResultScreen();
    }

    return Scaffold(
      appBar: appBar,
      body: SafeArea(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            Expanded(flex: 6, child: _getContainer(context, state.state)),
            Padding(
              padding: const EdgeInsets.all(30),
              child: Text(
                stringResources.mainScreenMeasureTitle,
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

    throw 'unexpected state';
  }

  Widget _waitContainer(BuildContext context) {
    final stringResources = AppLocalizations.of(context);

    return MainScreenUtils().circleContainerWithContent(
        context, AppIcons.waiting, stringResources.mainScreenWaitFinish);
  }

  Widget _goContainer(BuildContext context) {
    final state = context.watch<BenchmarkState>();
    final store = context.watch<Store>();
    final stringResources = AppLocalizations.of(context);

    return CustomPaint(
      painter: MyPaintBottom(),
      child: GoButtonGradient(() async {
        // TODO (anhappdev) Refactor the code here to avoid duplicated code.
        // The checks before calling state.runBenchmarks() in main_screen and result_screen are similar.
        final wrongPathError = await state.validator
            .validateExternalResourcesDirectory(
                stringResources.dialogContentMissingFiles);
        if (wrongPathError.isNotEmpty) {
          // TODO (anhappdev): Uncomment the if line and remove the ignore line, when updated to Flutter v3.4.
          // See https://github.com/flutter/flutter/issues/111488
          // if (!context.mounted) return;
          // ignore: use_build_context_synchronously
          await showErrorDialog(context, [wrongPathError]);
          return;
        }
        if (store.offlineMode) {
          final offlineError = await state.validator
              .validateOfflineMode(stringResources.dialogContentOfflineWarning);
          if (offlineError.isNotEmpty) {
            // TODO (anhappdev): Uncomment the if line and remove the ignore line, when updated to Flutter v3.4.
            // See https://github.com/flutter/flutter/issues/111488
            // if (!context.mounted) return;
            // ignore: use_build_context_synchronously
            switch (await showConfirmDialog(context, offlineError)) {
              case ConfirmDialogAction.ok:
                break;
              case ConfirmDialogAction.cancel:
                return;
              default:
                break;
            }
          }
        }
        try {
          await state.runBenchmarks();
        } catch (e, t) {
          print(t);
          // TODO (anhappdev): Uncomment the if line and remove the ignore line, when updated to Flutter v3.4.
          // See https://github.com/flutter/flutter/issues/111488
          // if (!context.mounted) return;
          // ignore: use_build_context_synchronously
          await showErrorDialog(
              context, ['${stringResources.runFail}:', e.toString()]);
          return;
        }
      }),
    );
  }
}

class GoButtonGradient extends StatelessWidget {
  final AsyncCallback onPressed;

  const GoButtonGradient(this.onPressed, {Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    final stringResources = AppLocalizations.of(context);

    var decoration = BoxDecoration(
      shape: BoxShape.circle,
      gradient: LinearGradient(
        colors: AppColors.runBenchmarkCircleGradient,
        begin: Alignment.topCenter,
        end: Alignment.bottomCenter,
      ),
      boxShadow: const [
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
        key: const Key(MainKeys.goButton),
        materialTapTargetSize: MaterialTapTargetSize.shrinkWrap,
        splashColor: Colors.black,
        shape: const CircleBorder(),
        onPressed: onPressed,
        child: Text(
          stringResources.mainScreenGo,
          style: const TextStyle(
            color: AppColors.lightText,
            fontSize: 40,
          ),
        ),
      ),
    );
  }
}
