import 'package:flutter/material.dart' hide Icons;

import 'package:flutter_svg/svg.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/icons.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';

Future<void> showPopupDialog(BuildContext context, String header,
    SvgPicture? icon, List<String> errors) async {
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
          Text(header),
          Align(alignment: Alignment.topRight, child: icon),
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

Future<void> showErrorDialog(BuildContext context, List<String> errors) async {
  final stringResources = AppLocalizations.of(context);
  await showPopupDialog(
      context, stringResources.errorDialogTitle, Icons.error, errors);
}

Future<void> showSuccessDialog(
    BuildContext context, List<String> errors) async {
  final stringResources = AppLocalizations.of(context);
  await showPopupDialog(
      context, stringResources.successDialogTitle, null, errors);
}
