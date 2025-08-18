class DartDefine {
  DartDefine._();

  static const isOfficialBuild =
      bool.fromEnvironment('OFFICIAL_BUILD', defaultValue: false);
  static const firebaseCrashlyticsEnabled =
      bool.fromEnvironment('FIREBASE_CRASHLYTICS_ENABLED', defaultValue: false);
  static const perfTestEnabled =
      bool.fromEnvironment('PERF_TEST', defaultValue: false);
}

class WidgetKeys {
  // list of widget keys that need to be accessed in the test code
  static const String goButton = 'goButton';
  static const String testAgainButton = 'testAgainButton';
  static const String totalScoreCircle = 'totalScoreCircle';
  static const String progressCircle = 'progressCircle';
}

class BenchmarkId {
  static const objectDetection = 'object_detection';
  static const imageSegmentationV2 = 'image_segmentation_v2';
  static const naturalLanguageProcessing = 'natural_language_processing';
  static const superResolution = 'super_resolution';
  static const imageClassificationV2 = 'image_classification_v2';
  static const imageClassificationOfflineV2 = 'image_classification_offline_v2';
  static const stableDiffusion = 'stable_diffusion';
  static const llm = 'llm';

  // The sort order of this list will be used in the UI
  static const allIds = [
    imageClassificationV2,
    objectDetection,
    imageSegmentationV2,
    naturalLanguageProcessing,
    superResolution,
    stableDiffusion,
    imageClassificationOfflineV2,
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
