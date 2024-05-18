import 'package:flutter/material.dart';

import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/app_styles.dart';

enum ConfirmDialogAction { ok, cancel }

Future<ConfirmDialogAction?> showConfirmDialog(
  BuildContext context,
  String message, {
  String? title,
}) async {
  final l10n = AppLocalizations.of(context)!;

  Widget cancelButton = TextButton(
      style: TextButton.styleFrom(
        textStyle: const TextStyle(fontSize: 18),
      ),
      onPressed: () => Navigator.pop(context, ConfirmDialogAction.cancel),
      child: Text(l10n.dialogCancel));

  Widget okButton = TextButton(
      style: TextButton.styleFrom(
        textStyle: const TextStyle(fontSize: 18),
      ),
      onPressed: () => Navigator.pop(context, ConfirmDialogAction.ok),
      child: Text(l10n.dialogOk));

  return await showDialog<ConfirmDialogAction>(
    context: context,
    builder: (context) {
      return AlertDialog(
        backgroundColor: AppColors.dialogBackground,
        title: Text(title ?? l10n.dialogTitleConfirm),
        content: SingleChildScrollView(child: Text(message)),
        actions: [cancelButton, okButton],
      );
    },
  );
}
