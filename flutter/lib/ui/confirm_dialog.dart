import 'package:flutter/material.dart' hide Icons;

import 'package:mlperfbench/localizations/app_localizations.dart';

enum ConfirmDialogAction { ok, cancel }

Future<ConfirmDialogAction?> showConfirmDialog(
    BuildContext context, String message) async {
  final stringResources = AppLocalizations.of(context);

  Widget cancelButton = TextButton(
      style: TextButton.styleFrom(
        textStyle: const TextStyle(fontSize: 18),
      ),
      onPressed: () => Navigator.pop(context, ConfirmDialogAction.cancel),
      child: Text(stringResources.cancel));

  Widget okButton = TextButton(
      style: TextButton.styleFrom(
        textStyle: const TextStyle(fontSize: 18),
      ),
      onPressed: () => Navigator.pop(context, ConfirmDialogAction.ok),
      child: Text(stringResources.ok));

  return await showDialog<ConfirmDialogAction>(
    context: context,
    builder: (context) {
      return AlertDialog(
        backgroundColor: Colors.white,
        title: Text(stringResources.confirmDialogTitle),
        content: SingleChildScrollView(child: Text(message)),
        actions: [cancelButton, okButton],
      );
    },
  );
}
