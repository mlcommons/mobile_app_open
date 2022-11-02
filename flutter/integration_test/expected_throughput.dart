import 'utils.dart';

/*
- value: Interval of expected throughput.
- key: model_code (on Android or iOS) or cpuFullName (on Windows).
*/

const _kTFLiteBackend = 'libtflitebackend';
const _kPixelBackend = 'libtflitepixelbackend';
const _kCoreMLBackend = 'libcoremlbackend';

// Windows
// Google Cloud Build n2-standard-4 machine
const _kCloudBuildX23 = 'Intel Xeon 2.30GHz';
const _kCloudBuildX28 = 'Intel Xeon 2.80GHz';
const _kRyzen5600 = 'AMD Ryzen 5 5600X 6-Core';

// Android
const _kPixel5 = 'Pixel 5';
const _kPixel6 = 'Pixel 6';

// iOS
const _kIphoneOnGitHubAction = 'iPhone15,3';
const _kIphoneOnMacbookM1 = 'iPhone14,7';

const Map<String, Map<String, Interval>> _imageClassification = {
  _kTFLiteBackend: {
    _kCloudBuildX23: Interval(min: 4, max: 11),
    _kCloudBuildX28: Interval(min: 4, max: 12),
    _kRyzen5600: Interval(min: 31, max: 37),
    _kPixel5: Interval(min: 80, max: 120),
    _kIphoneOnGitHubAction: Interval(min: 550, max: 750),
    _kIphoneOnMacbookM1: Interval(min: 19, max: 27),
  },
  _kCoreMLBackend: {
    _kIphoneOnGitHubAction: Interval(min: 2, max: 7),
  },
  _kPixelBackend: {
    _kPixel6: Interval(min: 900, max: 1100),
  },
};

const Map<String, Map<String, Interval>> _objectDetection = {
  _kTFLiteBackend: {
    _kCloudBuildX23: Interval(min: 4, max: 7),
    _kCloudBuildX28: Interval(min: 3.5, max: 7),
    _kRyzen5600: Interval(min: 14, max: 22),
    _kPixel5: Interval(min: 40, max: 55),
    _kIphoneOnGitHubAction: Interval(min: 200, max: 300),
    _kIphoneOnMacbookM1: Interval(min: 9, max: 16),
  },
  _kCoreMLBackend: {
    _kIphoneOnGitHubAction: Interval(min: 2, max: 7),
  },
  _kPixelBackend: {
    _kPixel6: Interval(min: 380, max: 490),
  },
};

const Map<String, Map<String, Interval>> _imageSegmentation = {
  _kTFLiteBackend: {
    _kCloudBuildX23: Interval(min: 0.5, max: 3),
    _kCloudBuildX28: Interval(min: 0.5, max: 3),
    _kRyzen5600: Interval(min: 5, max: 7),
    _kPixel5: Interval(min: 25, max: 35),
    _kIphoneOnGitHubAction: Interval(min: 18, max: 26),
    _kIphoneOnMacbookM1: Interval(min: 3, max: 6),
  },
  _kCoreMLBackend: {
    _kIphoneOnGitHubAction: Interval(min: 0.5, max: 2),
  },
  _kPixelBackend: {
    _kPixel6: Interval(min: 140, max: 180),
  },
};

const Map<String, Map<String, Interval>> _naturalLanguageProcessing = {
  _kTFLiteBackend: {
    _kCloudBuildX23: Interval(min: 0.7, max: 1.1),
    _kCloudBuildX28: Interval(min: 0.5, max: 1.1),
    _kRyzen5600: Interval(min: 2.8, max: 3.2),
    _kPixel5: Interval(min: 2.3, max: 2.7),
    _kIphoneOnGitHubAction: Interval(min: 9, max: 15),
    _kIphoneOnMacbookM1: Interval(min: 1.8, max: 3),
  },
  _kCoreMLBackend: {
    _kIphoneOnGitHubAction: Interval(min: 0.4, max: 1),
  },
  _kPixelBackend: {
    // pixel some time finish this task in 4 seconds, not sure why.
    _kPixel6: Interval(min: 2, max: 75),
  },
};

const Map<String, Map<String, Interval>> _imageClassificationOffline = {
  _kTFLiteBackend: {
    _kCloudBuildX23: Interval(min: 8, max: 14),
    _kCloudBuildX28: Interval(min: 7, max: 14),
    _kRyzen5600: Interval(min: 45, max: 60),
    _kPixel5: Interval(min: 120, max: 180),
    _kIphoneOnGitHubAction: Interval(min: 1300, max: 1900),
    _kIphoneOnMacbookM1: Interval(min: 30, max: 45),
  },
  _kCoreMLBackend: {
    _kIphoneOnGitHubAction: Interval(min: 3, max: 15),
  },
  _kPixelBackend: {
    _kPixel6: Interval(min: 1400, max: 1700),
  },
};

const benchmarkExpectedThroughput = {
  'image_classification': _imageClassification,
  'object_detection': _objectDetection,
  'image_segmentation_v2': _imageSegmentation,
  'natural_language_processing': _naturalLanguageProcessing,
  'image_classification_offline': _imageClassificationOffline,
};
