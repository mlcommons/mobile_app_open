import 'package:flutter/material.dart';

import 'package:mlperfbench/app_constants.dart';

class AppColors {
  static const lightText = Colors.white;
  static const darkText = Colors.black;
  static const resultValid = Colors.indigo;
  static const resultInvalid = Colors.red;
  static const darkRedText = Colors.red;

  static const blue1 = Color(0XFF2C92CB);
  static Color mediumBlue = Colors.lightBlue.shade900;
  static const blue12 = Color(0xFF0B3A61);
  static const blue9 = Color(0xFF166299);

  static const dialogBackground = Colors.white;
  static const snackBarBackground = Color(0xFFEDEDED);
  static Color appBarBackground =
      DartDefine.isOfficialBuild ? blue9 : Colors.brown;

  static const appBarIcon = Colors.white;

  static const progressCircle = Color(0xff135384);

  static const progressCancelButton = Color(0x000B4A7F);

  static Color get shareTextButton => Colors.blue.shade900;
}

class AppGradients {
  static List<Color> fullScreen = DartDefine.isOfficialBuild
      ? [const Color(0xff3189E2), const Color(0xff0B4A7F)]
      : [Colors.brown.shade400, Colors.brown];

  static List<Color> halfScreen = [
    AppColors.blue1,
    AppColors.blue12,
  ];

  static List<Color> get resultBar => [
        const Color(0xFF135384),
        const Color(0xFF3183E2),
        const Color(0xFF31B8E2),
        const Color(0xFF7DD5F0),
        const Color(0xFF6AD7F9)
      ];

  static List<Color> progressCircle = [
    const Color(0xff135384),
    const Color(0xff135384)
  ];

  static List<Color> resultCircle = [
    Color.lerp(const Color(0xFF328BE2), Colors.white, 1 - 0.65)!,
    const Color(0xFF328BE2), // 328BE2
  ];
}

class WidgetSizes {
  static const circleWidthFactor = 0.32;
  static const borderRadius = 8.0;
}
