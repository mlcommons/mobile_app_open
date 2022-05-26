import 'package:flutter/material.dart';

const OFFICIAL_BUILD =
    bool.fromEnvironment('official-build', defaultValue: false);

const FAST_MODE = bool.fromEnvironment('fast-mode', defaultValue: false);

class AppColors {
  static const primary = Colors.blue;
  static const secondary = Colors.green;
  static const lightText = Colors.white;
  static final lightRedText = Color.fromARGB(255, 255, 100, 100);
  static const darkText = Colors.black;
  static const darkRedText = Colors.red;
  static const lightBackground = Colors.white;
  static const dartBackground = Colors.black45;
  static const dialogBackground = Colors.white;
  static const snackBarBackground = Color(0xFFEDEDED);

  static const lightAppBarBackground = Colors.white;

  static Color get darkAppBarBackground =>
      OFFICIAL_BUILD ? Color(0xFF31A3E2) : Colors.brown.shade400;

  static const lightAppBarIconTheme = Colors.white;
  static const darkAppBarIconTheme = Color(0xFF135384);

  static List<Color> get mainScreenGradient => OFFICIAL_BUILD
      ? [
          Color(0xFF31A3E2),
          Color(0xFF31A3E2),
          Color(0xFF31A3E2),
          Color(0xFF3189E2),
          Color(0xFF0B4A7F),
        ]
      : [
          Colors.brown.shade400,
          Colors.brown.shade400,
          Colors.brown.shade400,
          Colors.brown,
          Colors.brown,
        ];

  static const runBenchmarkRectangle = Color(0xFF0DB526);

  static List<Color> get runBenchmarkCircleGradient => [
        Color.lerp(Color(0xFF0DB526), Colors.white, 0.65)!,
        Color(0xFF0DB526), // 0DB526
      ];

  static List<Color> get progressScreenGradient => OFFICIAL_BUILD
      ? [Color(0xff3189E2), Color(0xff0B4A7F)]
      : [Colors.brown.shade400, Colors.brown];

  static const progressCircle = Color(0xff135384);

  static List<Color> get progressCircleGradient =>
      [Color(0xff135384), Color(0xff135384)];

  static List<Color> get resultCircleGradient => [
        Color.lerp(Color(0xFF328BE2), Colors.white, 1 - 0.65)!,
        Color(0xFF328BE2), // 328BE2
      ];

  static List<Color> get resultBarGradient => [
        Color(0xFF135384),
        Color(0xFF3183E2),
        Color(0xFF31B8E2),
        Color(0xFF7DD5F0),
        Color(0xFF6AD7F9)
      ];

  static const shareRectangle = Colors.green;

  static Color get shareTextButton => Colors.blue.shade900;
}
