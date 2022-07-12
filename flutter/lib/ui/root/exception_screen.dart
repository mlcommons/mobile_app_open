import 'package:flutter/material.dart';

import 'package:mlperfbench/app_constants.dart';

class ExceptionWidget extends StatelessWidget {
  final Object error;
  final StackTrace stackTrace;

  const ExceptionWidget(this.error, this.stackTrace, {Key? key})
      : super(key: key);

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
        title: 'MLPerf Mobile',
        home: Material(
          type: MaterialType.transparency,
          child: SafeArea(
              child: Text(
            'Error: $error\n\nStackTrace: $stackTrace',
            style: const TextStyle(
                fontSize: 16,
                fontWeight: FontWeight.normal,
                color: AppColors.lightText),
          )),
        ));
  }
}
