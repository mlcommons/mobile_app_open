import 'package:flutter/material.dart';

import 'package:mlperfbench/app_constants.dart';

class AppColors {
  AppColors._();

  static const _blue1 = Color(0XFF2C92CB);
  static const _blue2 = Color(0XFF2980B7);
  static const _blue3 = Color(0XFF166299);
  static const _blue4 = Color(0XFF135384);
  static const _blue5 = Color(0xFF0B3A61);
  static const _green = Color(0xFF41C555);
  static const _brown = Colors.brown;

  static const lightText = Colors.white;
  static const darkText = Colors.black;
  static const resultValidText = Colors.indigo;
  static const resultInvalidText = Colors.red;
  static const resultSemiValidText = Colors.purple;
  static const errorText = Colors.red;

  static const primary = _blue2;
  static const secondary = _blue1;

  static const primaryAppBarBackground =
      DartDefine.isOfficialBuild ? _blue3 : _brown;
  static const secondaryAppBarBackground =
      DartDefine.isOfficialBuild ? _blue1 : _brown;
  static const appBarIcon = Colors.white;
  static const drawerBackground = _blue4;
  static const drawerForeground = Colors.white;
  static const dialogBackground = Colors.white;
  static const shareTextButton = _blue2;
  static const shareSectionBackground = _blue4;
  static const infoSectionBackground = _blue5;
  static const progressCircle = _blue4;
  static const goCircle = _green;
}

class AppGradients {
  AppGradients._();

  static List<Color> fullScreen = [
    AppColors._blue1,
    AppColors._blue2,
    AppColors._blue3,
    AppColors._blue4,
  ];

  static List<Color> halfScreen = [
    AppColors._blue1,
    AppColors._blue2,
    AppColors._blue3,
  ];

  static List<Color> get scoreBar => [
        AppColors._blue3,
        AppColors._blue2,
        AppColors._blue1,
      ];
}

class WidgetSizes {
  WidgetSizes._();

  static const circleWidthFactor = 0.32;
  static const borderRadius = 8.0;
}
