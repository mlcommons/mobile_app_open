import 'utils.dart';

const _tagTflite = 'libtflitebackend';
const _tagPixelSo = 'libtflitepixelbackend';
const _tagCoreml = 'libcoremlbackend';

// Windows
// Google Cloud Build n2-standard-4 machine
const _tagCloudBuildX23 = 'Intel Xeon 2.30GHz';
const _tagCloudBuildX28 = 'Intel Xeon 2.80GHz';
const _tagRyzen5600 = 'AMD Ryzen 5 5600X 6-Core';

// Android
const _tagPixel5 = 'Pixel 5';
const _tagPixel6 = 'Pixel 6';

// iOS
const _tagIphoneOnGitHubAction = 'iPhone15,3';
const _tagIosSimMBP2019 = 'x86_64';

const Map<String, Map<String, Interval>> _imageClassification = {
  _tagCloudBuildX23: {
    _tagTflite: Interval(min: 8, max: 11),
  },
  _tagCloudBuildX28: {
    _tagTflite: Interval(min: 8, max: 12),
  },
  _tagRyzen5600: {
    _tagTflite: Interval(min: 31, max: 37),
  },
  _tagPixel5: {
    _tagTflite: Interval(min: 80, max: 120),
  },
  _tagPixel6: {
    _tagPixelSo: Interval(min: 900, max: 1100),
  },
  _tagIphoneOnGitHubAction: {
    _tagTflite: Interval(min: 550, max: 750),
    _tagCoreml: Interval(min: 450, max: 650),
  },
  _tagIosSimMBP2019: {
    _tagTflite: Interval(min: 19, max: 27),
  },
};

const Map<String, Map<String, Interval>> _objectDetection = {
  _tagCloudBuildX23: {
    _tagTflite: Interval(min: 4, max: 7),
  },
  _tagCloudBuildX28: {
    _tagTflite: Interval(min: 3.5, max: 7),
  },
  _tagRyzen5600: {
    _tagTflite: Interval(min: 14, max: 22),
  },
  _tagPixel5: {
    _tagTflite: Interval(min: 40, max: 55),
  },
  _tagPixel6: {
    _tagPixelSo: Interval(min: 380, max: 490),
  },
  _tagIphoneOnGitHubAction: {
    _tagTflite: Interval(min: 200, max: 300),
    _tagCoreml: Interval(min: 250, max: 350),
  },
  _tagIosSimMBP2019: {_tagTflite: Interval(min: 9, max: 16)},
};

const Map<String, Map<String, Interval>> _imageSegmentation = {
  _tagCloudBuildX23: {
    _tagTflite: Interval(min: 1, max: 3),
  },
  _tagCloudBuildX28: {
    _tagTflite: Interval(min: 0.5, max: 3),
  },
  _tagRyzen5600: {
    _tagTflite: Interval(min: 5, max: 7),
  },
  _tagPixel5: {
    _tagTflite: Interval(min: 25, max: 35),
  },
  _tagPixel6: {
    _tagPixelSo: Interval(min: 140, max: 180),
  },
  _tagIphoneOnGitHubAction: {
    _tagTflite: Interval(min: 18, max: 26),
    _tagCoreml: Interval(min: 60, max: 90),
  },
  _tagIosSimMBP2019: {
    _tagTflite: Interval(min: 3, max: 5),
  },
};

const Map<String, Map<String, Interval>> _naturalLanguageProcessing = {
  _tagCloudBuildX23: {
    _tagTflite: Interval(min: 0.7, max: 1.1),
  },
  _tagCloudBuildX28: {
    _tagTflite: Interval(min: 0.5, max: 1.1),
  },
  _tagRyzen5600: {
    _tagTflite: Interval(min: 2.8, max: 3.2),
  },
  _tagPixel5: {
    _tagTflite: Interval(min: 2.3, max: 2.7),
  },
  _tagPixel6: {
    _tagPixelSo: Interval(min: 60, max: 75),
  },
  _tagIphoneOnGitHubAction: {
    _tagTflite: Interval(min: 9, max: 15),
    _tagCoreml: Interval(min: 65, max: 95),
  },
  _tagIosSimMBP2019: {
    _tagTflite: Interval(min: 1.8, max: 3),
  },
};

const Map<String, Map<String, Interval>> _imageClassificationOffline = {
  _tagCloudBuildX23: {
    _tagTflite: Interval(min: 10, max: 14),
  },
  _tagCloudBuildX28: {
    _tagTflite: Interval(min: 9, max: 14),
  },
  _tagRyzen5600: {
    _tagTflite: Interval(min: 45, max: 60),
  },
  _tagPixel5: {
    _tagTflite: Interval(min: 120, max: 180),
  },
  _tagPixel6: {
    _tagPixelSo: Interval(min: 1400, max: 1700),
  },
  _tagIphoneOnGitHubAction: {
    _tagTflite: Interval(min: 1300, max: 1900),
    _tagCoreml: Interval(min: 1000, max: 1500),
  },
  _tagIosSimMBP2019: {
    _tagTflite: Interval(min: 30, max: 40),
  },
};

const benchmarkExpectedThroughput = {
  'image_classification': _imageClassification,
  'object_detection': _objectDetection,
  'image_segmentation_v2': _imageSegmentation,
  'natural_language_processing': _naturalLanguageProcessing,
  'image_classification_offline': _imageClassificationOffline,
};
