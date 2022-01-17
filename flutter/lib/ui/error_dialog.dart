import 'package:flutter/material.dart' hide Icons;

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/icons.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';

Future<void> showErrorDialog(BuildContext context, List<String> errors) async {
  final stringResources = AppLocalizations.of(context);

  await showDialog(
    context: context,
    barrierDismissible: false,
    builder: (context) {
      return AlertDialog(
        backgroundColor: AppColors.dialogBackground,
        titlePadding: EdgeInsets.all(10),
        contentPadding: EdgeInsets.fromLTRB(15, 10, 10, 10),
        title:
            Row(mainAxisAlignment: MainAxisAlignment.spaceBetween, children: [
          Text(stringResources.errorDialogTitle),
          Align(alignment: Alignment.topRight, child: Icons.error),
        ]),
        content: SingleChildScrollView(
          child: ListBody(
            children: [
              ...errors.map((e) => Text(
                    e,
                    style: TextStyle(fontSize: 14),
                  ))
            ],
          ),
        ),
        actions: [
          TextButton(
              onPressed: () => Navigator.of(context).pop(),
              child: Text(stringResources.ok))
        ],
      );
    },
  );
}
