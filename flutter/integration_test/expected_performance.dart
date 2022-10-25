const _tagTflite = 'libtflitebackend';
const _tagPixelSo = 'libtflitepixelbackend';
const _tagCoreml = 'libcoremlbackend';

/// Windows
// cloudbuild n2-standard-4 machine
const _tagCloudBuildX28 = 'Intel Xeon 2.80GHz';
const _tagCloudBuildX23 = 'Intel Xeon 2.30GHz';
const _tagRyzen5600 = 'AMD Ryzen 5 5600X 6-Core';

const _tagPixel5 = 'Pixel 5';
const _tagPixel6 = 'Pixel 6';

const _tagIphone12mini = 'iPhone13,1';
const _tagIosSimMBP2019 = 'x86_64';

const Map<String, Map<String, double>> _imageClassification = {
  _tagCloudBuildX23: {_tagTflite: 9.0},
  _tagCloudBuildX28: {_tagTflite: 10},
  _tagRyzen5600: {_tagTflite: 34},
  _tagPixel5: {_tagTflite: 104},
  _tagPixel6: {_tagPixelSo: 1015},
  _tagIphone12mini: {
    _tagTflite: 654,
    _tagCoreml: 545,
  },
  _tagIosSimMBP2019: {_tagTflite: 23},
};

const Map<String, Map<String, double>> _objectDetection = {
  _tagCloudBuildX23: {_tagTflite: 5.2},
  _tagCloudBuildX28: {_tagTflite: 5.1},
  _tagRyzen5600: {_tagTflite: 18},
  _tagPixel5: {_tagTflite: 48},
  _tagPixel6: {_tagPixelSo: 440},
  _tagIphone12mini: {
    _tagTflite: 259,
    _tagCoreml: 305,
  },
  _tagIosSimMBP2019: {_tagTflite: 12.5},
};

const Map<String, Map<String, double>> _imageSegmentation = {
  _tagCloudBuildX23: {_tagTflite: 2.0},
  _tagCloudBuildX28: {_tagTflite: 1.9},
  _tagRyzen5600: {_tagTflite: 6},
  _tagPixel5: {_tagTflite: 30},
  _tagPixel6: {_tagPixelSo: 160},
  _tagIphone12mini: {
    _tagTflite: 22,
    _tagCoreml: 75,
  },
  _tagIosSimMBP2019: {_tagTflite: 4},
};

const Map<String, Map<String, double>> _naturalLanguageProcessing = {
  _tagCloudBuildX23: {_tagTflite: 0.85},
  _tagCloudBuildX28: {_tagTflite: 1},
  _tagRyzen5600: {_tagTflite: 3},
  _tagPixel5: {_tagTflite: 2.4},
  _tagPixel6: {_tagPixelSo: 67},
  _tagIphone12mini: {
    _tagTflite: 12,
    _tagCoreml: 81,
  },
  _tagIosSimMBP2019: {_tagTflite: 2.3},
};

const Map<String, Map<String, double>> _imageClassificationOffline = {
  _tagCloudBuildX23: {_tagTflite: 11.4},
  _tagCloudBuildX28: {_tagTflite: 12},
  _tagRyzen5600: {_tagTflite: 52},
  _tagPixel5: {_tagTflite: 152},
  _tagPixel6: {_tagPixelSo: 1550},
  _tagIphone12mini: {
    _tagTflite: 1600,
    _tagCoreml: 1250,
  },
  _tagIosSimMBP2019: {_tagTflite: 35},
};

const taskExpectedPerformance = {
  'image_classification': _imageClassification,
  'object_detection': _objectDetection,
  'image_segmentation_v2': _imageSegmentation,
  'natural_language_processing': _naturalLanguageProcessing,
  'image_classification_offline': _imageClassificationOffline,
};

const expectedInstabilityMap = {
  _tagCloudBuildX23: {_tagTflite: 1.1},
  _tagCloudBuildX28: {_tagTflite: 1.1},
  _tagRyzen5600: {_tagTflite: 1.05},
  _tagPixel5: {_tagTflite: 1.15},
  _tagPixel6: {_tagPixelSo: 1.1},
};
