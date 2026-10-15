class DartDefine {
  DartDefine._();

  static const isOfficialBuild = bool.fromEnvironment(
    'OFFICIAL_BUILD',
    defaultValue: false,
  );
  static const firebaseCrashlyticsEnabled = bool.fromEnvironment(
    'FIREBASE_CRASHLYTICS_ENABLED',
    defaultValue: false,
  );
  static const perfTestEnabled = bool.fromEnvironment(
    'PERF_TEST',
    defaultValue: false,
  );
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
  static const llm1b = 'llm-1b';
  static const llm1bInstruct = 'llm-1b-instruct';
  static const llm3b = 'llm-3b';
  static const llm3bInstruct = 'llm-3b-instruct';
  static const llm8b = 'llm-8b';
  static const llm8bInstruct = 'llm-8b-instruct';

  static const llmIds = [
    llm1b,
    llm1bInstruct,
    llm3b,
    llm3bInstruct,
    llm8b,
    llm8bInstruct,
  ];

  // The sort order of this list will be used in the UI.
  // Every task in tasks.pbtxt must appear here: BenchmarkStore sorts with
  // indexOf, which returns -1 for anything missing and floats it to the top.
  static const allIds = [
    imageClassificationV2,
    objectDetection,
    imageSegmentationV2,
    naturalLanguageProcessing,
    superResolution,
    stableDiffusion,
    llm1b,
    llm1bInstruct,
    llm3b,
    llm3bInstruct,
    llm8b,
    llm8bInstruct,
    imageClassificationOfflineV2,
  ];
}

/// Ids of the `task_set` blocks in tasks.pbtxt.
class BenchmarkSetId {
  static const imageClassification = 'image_classification';
  static const llm = 'llm';
}

class BackendId {
  static const tflite = 'libtflitebackend';
  static const pixel = 'libtflitepixelbackend';
  static const mediatek = 'libtfliteneuronbackend';
  static const samsung = 'libsamsungbackend';
  static const qti = 'libqtibackend';
  static const apple = 'libcoremlbackend';

  static const allIds = [tflite, pixel, mediatek, samsung, qti, apple];
}

class Url {
  static const privacyPolicy = 'https://mlcommons.org/privacy';
  static const eula = 'https://mlcommons.org/mobile_eula';
}
