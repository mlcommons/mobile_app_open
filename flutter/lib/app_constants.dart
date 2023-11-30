import 'package:flutter/material.dart';

const isOfficialBuild =
    bool.fromEnvironment('OFFICIAL_BUILD', defaultValue: false);
const firebaseCrashlyticsEnabled =
    bool.fromEnvironment('FIREBASE_CRASHLYTICS_ENABLED', defaultValue: false);
const isFastMode = bool.fromEnvironment('FAST_MODE', defaultValue: false);
const defaultCacheFolder = String.fromEnvironment('FLUTTER_CACHE_FOLDER');
const defaultDataFolder = String.fromEnvironment('FLUTTER_DATA_FOLDER');

class AppColors {
  static const lightText = Colors.white;
  static const lightRedText = Color.fromARGB(255, 255, 120, 100);
  static const darkText = Colors.black;
  static const darkRedText = Colors.red;

  static const dialogBackground = Colors.white;
  static const snackBarBackground = Color(0xFFEDEDED);
  static const appBarBackground =
      isOfficialBuild ? Color(0xFF31A3E2) : Colors.brown;

  static const appBarIcon = Colors.white;

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

  static const allIds = [
    imageClassification,
    objectDetection,
    imageSegmentationV2,
    naturalLanguageProcessing,
    imageClassificationOffline,
    superResolution,
  ];
}

class BackendId {
  static const tflite = 'libtflitebackend';
  static const pixel = 'libtflitepixelbackend';
  static const mediatek = 'libtfliteneuronbackend';
  static const samsung = 'libsamsungbackend';
  static const qti = 'libqtibackend';
  static const apple = 'libcoremlbackend';

  static const allIds = [
    tflite,
    pixel,
    mediatek,
    samsung,
    qti,
    apple,
  ];
}

class Url {
  static const privacyPolicy = 'https://mlcommons.org/mobile_privacy';
  static const eula = 'https://mlcommons.org/mobile_eula';
}
