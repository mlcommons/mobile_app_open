import 'utils.dart';

const _tagTfliteBackend = 'libtflitebackend';
const _tagPixelBackend = 'libtflitepixelbackend';
const _tagCoremlBackend = 'libcoremlbackend';

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
  _tagTfliteBackend: {
    _tagCloudBuildX23: Interval(min: 8, max: 11),
    _tagCloudBuildX28: Interval(min: 8, max: 12),
    _tagRyzen5600: Interval(min: 31, max: 37),
    _tagPixel5: Interval(min: 80, max: 120),
    _tagIphoneOnGitHubAction: Interval(min: 550, max: 750),
    _tagIosSimMBP2019: Interval(min: 19, max: 27),
  },
  _tagCoremlBackend: {
    _tagIphoneOnGitHubAction: Interval(min: 450, max: 650),
  },
  _tagPixelBackend: {
    _tagPixel6: Interval(min: 900, max: 1100),
  },
};

const Map<String, Map<String, Interval>> _objectDetection = {
  _tagTfliteBackend: {
    _tagCloudBuildX23: Interval(min: 4, max: 7),
    _tagCloudBuildX28: Interval(min: 3.5, max: 7),
    _tagRyzen5600: Interval(min: 14, max: 22),
    _tagPixel5: Interval(min: 40, max: 55),
    _tagIphoneOnGitHubAction: Interval(min: 200, max: 300),
    _tagIosSimMBP2019: Interval(min: 9, max: 16),
  },
  _tagCoremlBackend: {
    _tagIphoneOnGitHubAction: Interval(min: 250, max: 350),
  },
  _tagPixelBackend: {
    _tagPixel6: Interval(min: 380, max: 490),
  },
};

const Map<String, Map<String, Interval>> _imageSegmentation = {
  _tagTfliteBackend: {
    _tagCloudBuildX23: Interval(min: 1, max: 3),
    _tagCloudBuildX28: Interval(min: 0.5, max: 3),
    _tagRyzen5600: Interval(min: 5, max: 7),
    _tagPixel5: Interval(min: 25, max: 35),
    _tagIphoneOnGitHubAction: Interval(min: 18, max: 26),
    _tagIosSimMBP2019: Interval(min: 3, max: 5),
  },
  _tagCoremlBackend: {
    _tagIphoneOnGitHubAction: Interval(min: 60, max: 90),
  },
  _tagPixelBackend: {
    _tagPixel6: Interval(min: 140, max: 180),
  },
};

const Map<String, Map<String, Interval>> _naturalLanguageProcessing = {
  _tagTfliteBackend: {
    _tagCloudBuildX23: Interval(min: 0.7, max: 1.1),
    _tagCloudBuildX28: Interval(min: 0.5, max: 1.1),
    _tagRyzen5600: Interval(min: 2.8, max: 3.2),
    _tagPixel5: Interval(min: 2.3, max: 2.7),
    _tagIphoneOnGitHubAction: Interval(min: 9, max: 15),
    _tagIosSimMBP2019: Interval(min: 1.8, max: 3),
  },
  _tagCoremlBackend: {
    _tagIphoneOnGitHubAction: Interval(min: 65, max: 95),
  },
  _tagPixelBackend: {
    _tagPixel6: Interval(min: 60, max: 75),
  },
};

const Map<String, Map<String, Interval>> _imageClassificationOffline = {
  _tagTfliteBackend: {
    _tagCloudBuildX23: Interval(min: 10, max: 14),
    _tagCloudBuildX28: Interval(min: 9, max: 14),
    _tagRyzen5600: Interval(min: 45, max: 60),
    _tagPixel5: Interval(min: 120, max: 180),
    _tagIphoneOnGitHubAction: Interval(min: 1300, max: 1900),
    _tagIosSimMBP2019: Interval(min: 30, max: 40),
  },
  _tagCoremlBackend: {
    _tagIphoneOnGitHubAction: Interval(min: 1000, max: 1500),
  },
  _tagPixelBackend: {
    _tagPixel6: Interval(min: 1400, max: 1700),
  },
};

const benchmarkExpectedThroughput = {
  'image_classification': _imageClassification,
  'object_detection': _objectDetection,
  'image_segmentation_v2': _imageSegmentation,
  'natural_language_processing': _naturalLanguageProcessing,
  'image_classification_offline': _imageClassificationOffline,
};
