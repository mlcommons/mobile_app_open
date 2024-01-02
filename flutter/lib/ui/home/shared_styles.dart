import 'package:flutter/material.dart';

import 'package:mlperfbench/app_constants.dart';

const mainLinearGradientDecoration = BoxDecoration(
  gradient: LinearGradient(
    begin: Alignment.topCenter,
    end: Alignment.bottomCenter,
    colors: [
      AppColors.lightBlue,
      AppColors.darkBlue,
    ],
  ),
);
