import 'package:flutter/material.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/icons.dart' show AppIcons;
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/page_constraints.dart';

class UnsupportedDeviceScreen extends StatelessWidget {
  final String backendError;

  const UnsupportedDeviceScreen({Key? key, required this.backendError})
      : super(key: key);

  @override
  Widget build(BuildContext context) {
    final stringResources = AppLocalizations.of(context);

    final iconEdgeSize = MediaQuery.of(context).size.width * 0.66;

    return Scaffold(
      body: getSinglePageView(
        Column(
          crossAxisAlignment: CrossAxisAlignment.center,
          mainAxisAlignment: MainAxisAlignment.spaceEvenly,
          children: [
            Expanded(
                flex: 3,
                child: Align(
                    alignment: Alignment.bottomCenter,
                    child: Container(
                        height: iconEdgeSize,
                        width: iconEdgeSize,
                        child: AppIcons.error))),
            Expanded(
                child: Padding(
                    padding: EdgeInsets.fromLTRB(35, 0, 35, 0),
                    child: Align(
                        alignment: Alignment.topCenter,
                        child: Column(
                          children: [
                            Text(stringResources.unsupportedMainMessage,
                                textAlign: TextAlign.center,
                                style: TextStyle(
                                    fontSize: 15, color: AppColors.darkText)),
                            Text(
                                '${stringResources.unsupportedBackendError}: $backendError',
                                textAlign: TextAlign.center,
                                style: TextStyle(
                                    fontSize: 15, color: AppColors.darkText)),
                            Text(stringResources.unsupportedTryAnotherDevice,
                                textAlign: TextAlign.center,
                                style: TextStyle(
                                    fontSize: 15, color: AppColors.darkText)),
                          ],
                        )))),
          ],
        ),
      ),
    );
  }
}
