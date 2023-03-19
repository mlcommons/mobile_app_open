import 'package:flutter/material.dart';

const isOfficialBuild =
    bool.fromEnvironment('official-build', defaultValue: false);

const isFastMode = bool.fromEnvironment('fast-mode', defaultValue: false);

const defaultCacheFolder = String.fromEnvironment('default-cache-folder');
const defaultDataFolder = String.fromEnvironment('default-data-folder');

class AppColors {
  static const primary = Colors.blue;
  static const secondary = Colors.green;
  static const lightText = Colors.white;
  static const lightRedText = Color.fromARGB(255, 255, 120, 100);
  static const darkText = Colors.black;
  static const darkRedText = Colors.red;
  static const lightBackground = Colors.white;
  static const dartBackground = Colors.black45;
  static const dialogBackground = Colors.white;
  static const snackBarBackground = Color(0xFFEDEDED);

  static const lightAppBarBackground = Colors.white;
  static const darkAppBarThemeBackground =
      isOfficialBuild ? Colors.blue : Colors.brown;

  static Color get mainScreenAppBarBackground =>
      isOfficialBuild ? const Color(0xFF31A3E2) : Colors.brown;

  static const lightAppBarIconTheme = Colors.white;
  static const darkAppBarIconTheme = Color(0xFF135384);

  static List<Color> get mainScreenGradient => isOfficialBuild
      ? [
          const Color(0xFF31A3E2),
          const Color(0xFF31A3E2),
          const Color(0xFF31A3E2),
          const Color(0xFF3189E2),
          const Color(0xFF0B4A7F),
        ]
      : [
          Colors.brown,
          Colors.brown.shade500,
          Colors.brown.shade500,
          Colors.brown.shade700,
          Colors.brown.shade800,
        ];

  static const runBenchmarkRectangle = Color(0xFF0DB526);

  static List<Color> get runBenchmarkCircleGradient => [
        Color.lerp(const Color(0xFF0DB526), Colors.white, 0.65)!,
        const Color(0xFF0DB526), // 0DB526
      ];

  static List<Color> get progressScreenGradient => isOfficialBuild
      ? [const Color(0xff3189E2), const Color(0xff0B4A7F)]
      : [Colors.brown.shade400, Colors.brown];

  static const progressCircle = Color(0xff135384);

  static const progressCancelButton = Color(0x000B4A7F);

  static List<Color> get progressCircleGradient =>
      [const Color(0xff135384), const Color(0xff135384)];

  static List<Color> get resultCircleGradient => [
        Color.lerp(const Color(0xFF328BE2), Colors.white, 1 - 0.65)!,
        const Color(0xFF328BE2), // 328BE2
      ];

  static List<Color> get resultBarGradient => [
        const Color(0xFF135384),
        const Color(0xFF3183E2),
        const Color(0xFF31B8E2),
        const Color(0xFF7DD5F0),
        const Color(0xFF6AD7F9)
      ];

  static const shareRectangle = Colors.green;

  static Color get shareTextButton => Colors.blue.shade900;
}

class BenchmarkId {
  static const imageClassification = 'image_classification';
  static const objectDetection = 'object_detection';
  static const imageSegmentationV2 = 'image_segmentation_v2';
  static const naturalLanguageProcessing = 'natural_language_processing';
  static const imageClassificationOffline = 'image_classification_offline';
  static const superResolution = 'super_resolution';
}
