const _tagTfliteDll = 'libtflitebackend.dll';
const _tagTfliteSo = 'libtflitebackend.so';
const _tagPixelSo = 'libtflitepixelbackend.so';
const _tagTfliteFramework = 'libtflitebackend.framework/libtflitebackend';
const _tagCoremlFramework = 'libcoremlbackend.framework/libcoremlbackend';

/// Windows
// cloudbuild n2-standard-4 machine
const _tagCloudBuildN2S4 = 'Unknown PC';
// not a valid device name, it's only here as a reference
const _tagRyzen5600 = 'Unknown PC+ryzen5600x';

const _tagPixel5 = 'Pixel 5';
const _tagPixel6 = 'Pixel 6';

const _tagIphone12mini = 'iPhone13,1';
const _tagIosSimMBP2019 = 'x86_64';

const Map<String, Map<String, double>> _imageClassification = {
  _tagCloudBuildN2S4: {_tagTfliteDll: 10},
  _tagRyzen5600: {_tagTfliteDll: 34},
  _tagPixel5: {_tagTfliteSo: 104},
  _tagPixel6: {_tagPixelSo: 1015},
  _tagIphone12mini: {
    _tagTfliteFramework: 654,
    _tagCoremlFramework: 545,
  },
  _tagIosSimMBP2019: {_tagTfliteSo: 23},
};

const Map<String, Map<String, double>> _objectDetection = {
  _tagCloudBuildN2S4: {_tagTfliteDll: 4.5},
  _tagRyzen5600: {_tagTfliteDll: 18},
  _tagPixel5: {_tagTfliteSo: 48},
  _tagPixel6: {_tagPixelSo: 440},
  _tagIphone12mini: {
    _tagTfliteFramework: 259,
    _tagCoremlFramework: 305,
  },
  _tagIosSimMBP2019: {_tagTfliteSo: 12.5},
};

const Map<String, Map<String, double>> _imageSegmentation = {
  _tagCloudBuildN2S4: {_tagTfliteDll: 1.9},
  _tagRyzen5600: {_tagTfliteDll: 6},
  _tagPixel5: {_tagTfliteSo: 30},
  _tagPixel6: {_tagPixelSo: 160},
  _tagIphone12mini: {
    _tagTfliteFramework: 22,
    _tagCoremlFramework: 75,
  },
  _tagIosSimMBP2019: {_tagTfliteSo: 4},
};

const Map<String, Map<String, double>> _naturalLanguageProcessing = {
  _tagCloudBuildN2S4: {_tagTfliteDll: 1},
  _tagRyzen5600: {_tagTfliteDll: 3},
  _tagPixel5: {_tagTfliteSo: 2.4},
  _tagPixel6: {_tagPixelSo: 67},
  _tagIphone12mini: {
    _tagTfliteFramework: 12,
    _tagCoremlFramework: 81,
  },
  _tagIosSimMBP2019: {_tagTfliteSo: 2.3},
};

const Map<String, Map<String, double>> _imageClassificationOffline = {
  _tagCloudBuildN2S4: {_tagTfliteDll: 12},
  _tagRyzen5600: {_tagTfliteDll: 52},
  _tagPixel5: {_tagTfliteSo: 152},
  _tagPixel6: {_tagPixelSo: 1550},
  _tagIphone12mini: {
    _tagTfliteFramework: 1600,
    _tagCoremlFramework: 1250,
  },
  _tagIosSimMBP2019: {_tagTfliteSo: 35},
};

const taskExpectedPerformance = {
  'image_classification': _imageClassification,
  'object_detection': _objectDetection,
  'image_segmentation_v2': _imageSegmentation,
  'natural_language_processing': _naturalLanguageProcessing,
  'image_classification_offline': _imageClassificationOffline,
};

const expectedInstabilityMap = {
  _tagCloudBuildN2S4: {
    _tagTfliteDll: 1.15,
  },
  _tagPixel5: {
    _tagTfliteSo: 1.15,
  },
  _tagPixel6: {
    _tagPixelSo: 1.1,
  },
};
