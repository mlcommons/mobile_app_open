import 'package:flutter/material.dart';

import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/app_styles.dart';
import 'package:mlperfbench/ui/settings/resources_screen.dart';

enum DialogTypeEnum { error, warning, success }

Future<void> _showPopupDialog(BuildContext context, DialogTypeEnum type,
    String title, List<String> messages) async {
  final l10n = AppLocalizations.of(context)!;

  late Icon icon;
  late Color titleColor;
  switch (type) {
    case DialogTypeEnum.error:
      icon = const Icon(Icons.error_outline, color: Colors.red, size: 32);
      titleColor = Colors.red;
      break;
    case DialogTypeEnum.warning:
      icon = const Icon(Icons.warning_amber_outlined,
          color: Colors.amber, size: 32);
      titleColor = Colors.amber;
      break;
    case DialogTypeEnum.success:
      icon =
          const Icon(Icons.check_circle_outline, color: Colors.green, size: 32);
      titleColor = Colors.green;
      break;
  }

  await showDialog(
    context: context,
    barrierDismissible: false,
    builder: (context) {
      return AlertDialog(
        backgroundColor: AppColors.dialogBackground,
        titlePadding: const EdgeInsets.all(12),
        contentPadding: const EdgeInsets.fromLTRB(20, 4, 20, 4),
        shape: RoundedRectangleBorder(
          borderRadius: BorderRadius.circular(12),
        ),
        title: Column(
          children: [
            Row(
              mainAxisAlignment: MainAxisAlignment.spaceBetween,
              children: [
                Flexible(
                  child: Text(
                    title,
                    style: TextStyle(
                      color: titleColor,
                      fontWeight: FontWeight.bold,
                      fontSize: 18,
                    ),
                  ),
                ),
                icon,
              ],
            ),
            const Divider(color: Colors.grey, height: 12),
          ],
        ),
        content: SingleChildScrollView(
          child: Column(
            mainAxisSize: MainAxisSize.min,
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              ...messages.map((e) => Padding(
                    padding: const EdgeInsets.only(bottom: 8.0),
                    child: Text(
                      e,
                      style: const TextStyle(fontSize: 15),
                    ),
                  )),
            ],
          ),
        ),
        actions: [
          const Divider(height: 1),
          Padding(
            padding: const EdgeInsets.all(8.0),
            child: Row(
              mainAxisAlignment: MainAxisAlignment.end,
              children: [
                ElevatedButton(
                  onPressed: () => Navigator.of(context).pop(),
                  style: ElevatedButton.styleFrom(
                    backgroundColor: Theme.of(context).colorScheme.primary,
                    foregroundColor: Theme.of(context).colorScheme.onPrimary,
                  ),
                  child: Text(l10n.dialogOk),
                ),
              ],
            ),
          ),
        ],
      );
    },
  );
}

Future<void> showErrorDialog(
    BuildContext context, List<String> messages) async {
  final l10n = AppLocalizations.of(context)!;

  // Use the "WARN" prefix in messages to distinguish warnings from errors,
  // so the dialog can be styled appropriately based on the detected type.
  final isWarning = messages.any((m) => m.startsWith('WARN'));
  if (isWarning) {
    await _showPopupDialog(
        context, DialogTypeEnum.warning, l10n.dialogTitleWarning, messages);
    return;
  }

  await _showPopupDialog(
      context, DialogTypeEnum.error, l10n.dialogTitleError, messages);
}

Future<void> showResourceMissingDialog(
    BuildContext context, List<String> messages,
    {Benchmark? benchmark, BenchmarkSet? benchmarkSet}) async {
  final l10n = AppLocalizations.of(context)!;

  Icon icon = const Icon(Icons.warning_amber_rounded,
      color: AppColors.warningIcon, size: 32);
  Color titleColor = Colors.grey;

  await showDialog(
    context: context,
    barrierDismissible: false,
    builder: (context) {
      return AlertDialog(
        backgroundColor: AppColors.dialogBackground,
        titlePadding: const EdgeInsets.all(12),
        contentPadding: const EdgeInsets.fromLTRB(20, 4, 20, 4),
        shape: RoundedRectangleBorder(
          borderRadius: BorderRadius.circular(12),
        ),
        title: Column(
          children: [
            Row(
              mainAxisAlignment: MainAxisAlignment.spaceBetween,
              children: [
                Flexible(
                  child: Text(
                    l10n.dialogTitleWarning,
                    style: TextStyle(
                      color: titleColor,
                      fontWeight: FontWeight.bold,
                      fontSize: 18,
                    ),
                  ),
                ),
                icon,
              ],
            ),
            const Divider(color: Colors.grey, height: 12),
          ],
        ),
        content: SingleChildScrollView(
          child: Column(
            mainAxisSize: MainAxisSize.min,
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              Text(benchmark != null
                  ? l10n.dialogContentMissingFiles
                      .replaceAll('<name>', benchmark.info.taskName)
                  : benchmarkSet != null
                      ? l10n.dialogContentMissingFiles
                          .replaceAll('<name>', benchmarkSet.config.name)
                      : l10n.dialogContentMissingFiles
                          .replaceAll('<name>', l10n.benchNameVarious)),
            ],
          ),
        ),
        actions: [
          const Divider(height: 1),
          Padding(
            padding: const EdgeInsets.all(8.0),
            child: Row(
              mainAxisAlignment: MainAxisAlignment.spaceBetween,
              children: [
                TextButton(
                  onPressed: () => Navigator.of(context).pop(),
                  style: TextButton.styleFrom(
                    foregroundColor: Colors.grey[700],
                  ),
                  child: Text(l10n.dialogOk),
                ),
                ElevatedButton.icon(
                  icon: const Icon(Icons.download, size: 18),
                  label: Text(l10n.resourceDownload),
                  onPressed: () {
                    Navigator.of(context).pop();
                    Navigator.of(context).push(
                      MaterialPageRoute(
                        builder: (context) => ResourcesScreen(
                          autoStart: true,
                          benchmarksToDownload: (benchmark != null
                              ? <Benchmark>[benchmark]
                              : benchmarkSet?.benchmarks
                                  .where((e) => e.isActive)
                                  .toList()),
                        ),
                      ),
                    );
                  },
                  style: ElevatedButton.styleFrom(
                    backgroundColor: Theme.of(context).colorScheme.primary,
                    foregroundColor: Theme.of(context).colorScheme.onPrimary,
                    padding:
                        const EdgeInsets.symmetric(horizontal: 16, vertical: 8),
                  ),
                ),
              ],
            ),
          ),
        ],
      );
    },
  );
}

Future<void> showSuccessDialog(
    BuildContext context, List<String> messages) async {
  final l10n = AppLocalizations.of(context)!;
  await _showPopupDialog(
      context, DialogTypeEnum.success, l10n.dialogTitleSuccess, messages);
}
