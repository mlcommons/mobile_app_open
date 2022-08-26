import 'package:flutter/material.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';

enum ConfirmDialogAction { ok, cancel }

Future<ConfirmDialogAction?> showConfirmDialog(
  BuildContext context,
  String message, {
  String? title,
}) async {
  final stringResources = AppLocalizations.of(context);

  Widget cancelButton = TextButton(
      style: TextButton.styleFrom(
        textStyle: const TextStyle(fontSize: 18),
      ),
      onPressed: () => Navigator.pop(context, ConfirmDialogAction.cancel),
      child: Text(stringResources.dialogCancel));

  Widget okButton = TextButton(
      style: TextButton.styleFrom(
        textStyle: const TextStyle(fontSize: 18),
      ),
      onPressed: () => Navigator.pop(context, ConfirmDialogAction.ok),
      child: Text(stringResources.dialogOk));

  return await showDialog<ConfirmDialogAction>(
    context: context,
    builder: (context) {
      return AlertDialog(
        backgroundColor: AppColors.dialogBackground,
        title: Text(title ?? stringResources.dialogTitleConfirm),
        content: SingleChildScrollView(child: Text(message)),
        actions: [cancelButton, okButton],
      );
    },
  );
}
