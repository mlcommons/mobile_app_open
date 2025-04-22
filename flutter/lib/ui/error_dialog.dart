import 'package:flutter/material.dart';

import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/app_styles.dart';
import 'package:mlperfbench/ui/settings/resources_screen.dart';

enum DialogTypeEnum { error, warning, success }

Future<void> _showPopupDialog(BuildContext context, DialogTypeEnum type,
    String title, List<String> messages) async {
  final l10n = AppLocalizations.of(context)!;

  Icon? icon;
  Color titleColor;
  switch (type) {
    case DialogTypeEnum.error:
      icon = const Icon(Icons.error, color: Colors.red);
      titleColor = Colors.red;
      break;
    case DialogTypeEnum.warning:
      icon = const Icon(Icons.warning, color: Colors.yellow);
      titleColor = Colors.yellow;
      break;
    case DialogTypeEnum.success:
      icon = const Icon(Icons.done, color: Colors.green);
      titleColor = Colors.green;
      break;
  }

  await showDialog(
    context: context,
    barrierDismissible: false,
    builder: (context) {
      return AlertDialog(
        backgroundColor: AppColors.dialogBackground,
        titlePadding: const EdgeInsets.all(10),
        contentPadding: const EdgeInsets.fromLTRB(15, 10, 10, 10),
        title: Row(
          mainAxisAlignment: MainAxisAlignment.spaceBetween,
          children: [
            Text(title, style: TextStyle(color: titleColor)),
            Align(alignment: Alignment.topRight, child: icon),
          ],
        ),
        content: SingleChildScrollView(
          child: ListBody(
            children: [
              ...messages.map((e) => Text(
                    e,
                    style: const TextStyle(fontSize: 14),
                  ))
            ],
          ),
        ),
        actions: [
          TextButton(
              onPressed: () => Navigator.of(context).pop(),
              child: Text(l10n.dialogOk))
        ],
      );
    },
  );
}

Future<void> showWarningDialog(
    BuildContext context, List<String> messages) async {
  final l10n = AppLocalizations.of(context)!;
  await _showPopupDialog(
      context, DialogTypeEnum.warning, l10n.dialogTitleWarning, messages);
}

Future<void> showErrorDialog(
    BuildContext context, List<String> messages) async {
  final l10n = AppLocalizations.of(context)!;
  await _showPopupDialog(
      context, DialogTypeEnum.error, l10n.dialogTitleError, messages);
}

Future<void> showResourceErrorDialog(
    BuildContext context, List<String> messages) async {
  final l10n = AppLocalizations.of(context)!;

  Icon icon = const Icon(Icons.error_outline, color: Colors.red, size: 32);
  Color titleColor = Colors.red;

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
                Expanded(
                  child: Text(
                    l10n.dialogTitleError,
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
              Container(
                padding: const EdgeInsets.all(4),
                decoration: BoxDecoration(
                  color: Colors.amber.withOpacity(0.2),
                  borderRadius: BorderRadius.circular(8),
                  border: Border.all(color: Colors.amber, width: 1),
                ),
                child: Row(
                  children: [
                    const Icon(Icons.info_outline, color: Colors.amber),
                    const SizedBox(width: 10),
                    Expanded(
                      child: Text(
                        l10n.dialogContentMissingFilesHint,
                        style: const TextStyle(
                          fontSize: 14,
                          color: Colors.black87,
                        ),
                      ),
                    ),
                  ],
                ),
              ),
              const SizedBox(height: 10),
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
                        builder: (context) => const ResourcesScreen(),
                      ),
                    );
                  },
                  style: ElevatedButton.styleFrom(
                    backgroundColor: Colors.blue,
                    foregroundColor: Colors.white,
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
