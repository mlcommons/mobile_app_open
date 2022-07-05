import 'package:flutter/material.dart';

import 'package:mlperfbench/app_constants.dart';

SnackBar getSnackBar(String message) => SnackBar(
      duration: const Duration(seconds: 2),
      backgroundColor: AppColors.snackBarBackground,
      content: Text(
        message,
        textAlign: TextAlign.center,
        style: const TextStyle(color: AppColors.darkText),
      ),
    );
