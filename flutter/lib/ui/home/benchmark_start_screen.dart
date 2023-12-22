import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/store.dart';
import 'package:mlperfbench/ui/confirm_dialog.dart';
import 'package:mlperfbench/ui/error_dialog.dart';
import 'package:mlperfbench/ui/home/app_drawer.dart';
import 'package:mlperfbench/ui/home/benchmark_config_screen.dart';
import 'package:mlperfbench/ui/home/shared_styles.dart';
import 'package:mlperfbench/ui/icons.dart';

class MainKeys {
  // list of widget keys that need to be accessed in the test code
  static const String goButton = 'goButton';
}

class BenchmarkStartScreen extends StatelessWidget {
  const BenchmarkStartScreen({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    final state = context.watch<BenchmarkState>();
    final l10n = AppLocalizations.of(context);

    return Scaffold(
      backgroundColor: AppColors.darkBlue,
      appBar: AppBar(title: Text(l10n.menuHome)),
      drawer: const AppDrawer(),
      body: SafeArea(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          crossAxisAlignment: CrossAxisAlignment.center,
          children: <Widget>[
            Expanded(
              flex: 35,
              child: _getTopContainer(context, state.state),
            ),
            Expanded(
              flex: 65,
              child: Align(
                alignment: Alignment.topCenter,
                child: AbsorbPointer(
                  absorbing: state.state != BenchmarkStateEnum.waiting,
                  child: const BenchmarkConfigScreen(),
                ),
              ),
            )
          ],
        ),
      ),
    );
  }

  Widget _getTopContainer(BuildContext context, BenchmarkStateEnum state) {
    if (state == BenchmarkStateEnum.aborting) {
      return _waitContainer(context);
    } else if (state == BenchmarkStateEnum.waiting) {
      return _goContainer(context);
    } else {
      throw 'Unknown BenchmarkState: ${state.name}';
    }
  }

  Widget _waitContainer(BuildContext context) {
    final l10n = AppLocalizations.of(context);
    final circleWidth =
        MediaQuery.of(context).size.width * WidgetSizes.circleWidthFactor;

    return Stack(
      alignment: Alignment.topCenter,
      children: [
        Container(
          width: MediaQuery.of(context).size.width,
          alignment: Alignment.center,
          decoration: mainLinearGradientDecoration,
        ),
        Padding(
          padding: const EdgeInsets.all(20),
          child: Text(
            l10n.mainScreenWaitFinish,
            style: const TextStyle(color: AppColors.lightText, fontSize: 15),
          ),
        ),
        Stack(
          children: [
            Container(
              width: circleWidth,
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
              width: circleWidth,
              alignment: Alignment.center,
              child: AppIcons.waiting,
            )
          ],
        )
      ],
    );
  }

  Widget _goContainer(BuildContext context) {
    final state = context.watch<BenchmarkState>();
    final store = context.watch<Store>();
    final l10n = AppLocalizations.of(context);
    final circleWidth =
        MediaQuery.of(context).size.width * WidgetSizes.circleWidthFactor;

    return Stack(
      alignment: Alignment.topCenter,
      children: [
        Container(
          width: MediaQuery.of(context).size.width,
          alignment: Alignment.center,
          decoration: mainLinearGradientDecoration,
        ),
        Container(
          alignment: Alignment.center,
          child: ElevatedButton(
            key: const Key(MainKeys.goButton),
            style: ElevatedButton.styleFrom(
                backgroundColor: Colors.green,
                shape: const CircleBorder(),
                minimumSize: Size.fromWidth(circleWidth)),
            child: Text(
              l10n.mainScreenGo,
              style: const TextStyle(
                color: AppColors.lightText,
                fontSize: 40,
              ),
            ),
            onPressed: () async {
              final wrongPathError = await state.validator
                  .validateExternalResourcesDirectory(
                      l10n.dialogContentMissingFiles);
              if (wrongPathError.isNotEmpty) {
                // Workaround for Dart linter bug. See https://github.com/dart-lang/linter/issues/4007
                // ignore: use_build_context_synchronously
                if (!context.mounted) return;
                await showErrorDialog(context, [wrongPathError]);
                return;
              }
              if (store.offlineMode) {
                final offlineError = await state.validator
                    .validateOfflineMode(l10n.dialogContentOfflineWarning);
                if (offlineError.isNotEmpty) {
                  // Workaround for Dart linter bug. See https://github.com/dart-lang/linter/issues/4007
                  // ignore: use_build_context_synchronously
                  if (!context.mounted) return;
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
                // Workaround for Dart linter bug. See https://github.com/dart-lang/linter/issues/4007
                // ignore: use_build_context_synchronously
                if (!context.mounted) return;
                await showErrorDialog(
                    context, ['${l10n.runFail}:', e.toString()]);
                return;
              }
            },
          ),
        ),
      ],
    );
  }
}
