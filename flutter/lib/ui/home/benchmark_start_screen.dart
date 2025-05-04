import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/device_info.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/store.dart';
import 'package:mlperfbench/ui/app_styles.dart';
import 'package:mlperfbench/ui/confirm_dialog.dart';
import 'package:mlperfbench/ui/error_dialog.dart';
import 'package:mlperfbench/ui/home/app_drawer.dart';
import 'package:mlperfbench/ui/home/benchmark_config_section.dart';

class BenchmarkStartScreen extends StatefulWidget {
  const BenchmarkStartScreen({super.key});

  @override
  State<BenchmarkStartScreen> createState() => _BenchmarkStartScreenState();
}

class _BenchmarkStartScreenState extends State<BenchmarkStartScreen> {
  late BenchmarkState state;
  late Store store;
  late AppLocalizations l10n;

  @override
  Widget build(BuildContext context) {
    state = context.watch<BenchmarkState>();
    store = context.watch<Store>();
    l10n = AppLocalizations.of(context)!;

    return Scaffold(
      appBar: AppBar(
        title: Text(l10n.menuHome),
        backgroundColor: AppColors.secondaryAppBarBackground,
      ),
      drawer: const AppDrawer(),
      body: Column(
        mainAxisAlignment: MainAxisAlignment.center,
        crossAxisAlignment: CrossAxisAlignment.center,
        children: <Widget>[
          _goButtonSection(context),
          _infoSection(),
          Expanded(
            child: AbsorbPointer(
              absorbing: state.state != BenchmarkStateEnum.waiting,
              child: const BenchmarkConfigSection(),
            ),
          )
        ],
      ),
    );
  }

  Widget _infoSection() {
    final selectedCount = state.activeBenchmarks.length.toString();
    final totalCount = state.allBenchmarks.length.toString();
    final selectedBenchmarkText = l10n.mainScreenBenchmarkSelected
        .replaceAll('<selected>', selectedCount)
        .replaceAll('<total>', totalCount);
    var deviceDescription = DeviceInfo.instance.envInfo.modelDescription;
    return Container(
      padding: const EdgeInsets.fromLTRB(20, 4, 10, 4),
      width: double.infinity,
      color: AppColors.infoSectionBackground,
      child: DefaultTextStyle.merge(
        style: const TextStyle(color: Colors.white, fontSize: 14),
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          crossAxisAlignment: CrossAxisAlignment.center,
          mainAxisSize: MainAxisSize.min,
          children: [Text(deviceDescription), Text(selectedBenchmarkText)],
        ),
      ),
    );
  }

  Widget _goButtonSection(BuildContext context) {
    final circleWidth =
        MediaQuery.of(context).size.width * WidgetSizes.circleWidthFactor;
    const double verticalPadding = 8.0;
    final sectionHeight = circleWidth + verticalPadding * 2.0;

    return SizedBox(
      width: MediaQuery.of(context).size.width,
      height: sectionHeight,
      child: Stack(
        alignment: Alignment.topCenter,
        children: [
          Container(
            alignment: Alignment.center,
            decoration: BoxDecoration(
              gradient: LinearGradient(
                begin: Alignment.topCenter,
                end: Alignment.bottomCenter,
                colors: AppGradients.halfScreen,
              ),
            ),
          ),
          Container(
            alignment: Alignment.center,
            child: ElevatedButton(
              key: const Key(WidgetKeys.goButton),
              style: ElevatedButton.styleFrom(
                  backgroundColor: AppColors.goCircle,
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
                  if (!context.mounted) return;
                  await showResourceMissingDialog(context, [wrongPathError]);
                  return;
                }
                final checksumError = await state.validator
                    .validateChecksum(l10n.dialogContentChecksumError);
                if (checksumError.isNotEmpty) {
                  if (!context.mounted) return;
                  final messages = [
                    checksumError,
                    l10n.dialogContentChecksumErrorHint
                  ];
                  await showErrorDialog(context, messages);
                  return;
                }
                if (store.offlineMode) {
                  final offlineError = await state.validator
                      .validateOfflineMode(l10n.dialogContentOfflineWarning);
                  if (offlineError.isNotEmpty) {
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
                final selectedCount = state.activeBenchmarks.length;
                if (selectedCount < 1) {
                  // Workaround for Dart linter bug. See https://github.com/dart-lang/linter/issues/4007
                  // ignore: use_build_context_synchronously
                  if (!context.mounted) return;
                  await showErrorDialog(
                      context, [l10n.dialogContentNoSelectedBenchmarkError]);
                  return;
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
      ),
    );
  }
}
