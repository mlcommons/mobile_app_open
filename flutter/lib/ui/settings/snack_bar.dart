import 'package:flutter/material.dart';

import 'package:mlperfbench/app_constants.dart';

SnackBar getSnackBar(String message) => SnackBar(
      duration: Duration(seconds: 2),
      backgroundColor: AppColors.snackBarBackground,
      content: Text(
        message,
        textAlign: TextAlign.center,
        style: TextStyle(color: AppColors.darkText),
      ),
    );
