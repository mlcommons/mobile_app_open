import 'package:flutter/material.dart';

import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/app_styles.dart';

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

Future<void> showSuccessDialog(
    BuildContext context, List<String> messages) async {
  final l10n = AppLocalizations.of(context)!;
  await _showPopupDialog(
      context, DialogTypeEnum.success, l10n.dialogTitleSuccess, messages);
}
