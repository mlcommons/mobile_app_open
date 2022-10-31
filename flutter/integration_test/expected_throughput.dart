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

class ExpectedValue {
  final double mean;
  final double deviation;

  const ExpectedValue({
    required this.mean,
    required this.deviation,
  });
}

const Map<String, Map<String, ExpectedValue>> _imageClassification = {
  _tagCloudBuildX23: {
    _tagTflite: ExpectedValue(mean: 9.5, deviation: 1.15),
  },
  _tagCloudBuildX28: {
    _tagTflite: ExpectedValue(mean: 10, deviation: 1.15),
  },
  _tagRyzen5600: {
    _tagTflite: ExpectedValue(mean: 34, deviation: 1.05),
  },
  _tagPixel5: {
    _tagTflite: ExpectedValue(mean: 104, deviation: 1.15),
  },
  _tagPixel6: {
    _tagPixelSo: ExpectedValue(mean: 1015, deviation: 1.1),
  },
  _tagIphone12mini: {
    _tagTflite: ExpectedValue(mean: 654, deviation: 1.15),
    _tagCoreml: ExpectedValue(mean: 545, deviation: 1.15),
  },
  _tagIosSimMBP2019: {
    _tagTflite: ExpectedValue(mean: 23, deviation: 1.15),
  },
};

const Map<String, Map<String, ExpectedValue>> _objectDetection = {
  _tagCloudBuildX23: {
    _tagTflite: ExpectedValue(mean: 5.2, deviation: 1.15),
  },
  _tagCloudBuildX28: {
    _tagTflite: ExpectedValue(mean: 5.1, deviation: 1.15),
  },
  _tagRyzen5600: {
    _tagTflite: ExpectedValue(mean: 18, deviation: 1.05),
  },
  _tagPixel5: {
    _tagTflite: ExpectedValue(mean: 48, deviation: 1.15),
  },
  _tagPixel6: {
    _tagPixelSo: ExpectedValue(mean: 440, deviation: 1.1),
  },
  _tagIphone12mini: {
    _tagTflite: ExpectedValue(mean: 259, deviation: 1.15),
    _tagCoreml: ExpectedValue(mean: 305, deviation: 1.15),
  },
  _tagIosSimMBP2019: {_tagTflite: ExpectedValue(mean: 12.5, deviation: 1.15)},
};

const Map<String, Map<String, ExpectedValue>> _imageSegmentation = {
  _tagCloudBuildX23: {
    _tagTflite: ExpectedValue(mean: 2.0, deviation: 1.15),
  },
  _tagCloudBuildX28: {
    _tagTflite: ExpectedValue(mean: 1.9, deviation: 1.15),
  },
  _tagRyzen5600: {
    _tagTflite: ExpectedValue(mean: 6, deviation: 1.05),
  },
  _tagPixel5: {
    _tagTflite: ExpectedValue(mean: 30, deviation: 1.15),
  },
  _tagPixel6: {
    _tagPixelSo: ExpectedValue(mean: 160, deviation: 1.1),
  },
  _tagIphone12mini: {
    _tagTflite: ExpectedValue(mean: 22, deviation: 1.15),
    _tagCoreml: ExpectedValue(mean: 75, deviation: 1.15),
  },
  _tagIosSimMBP2019: {
    _tagTflite: ExpectedValue(mean: 4, deviation: 1.15),
  },
};

const Map<String, Map<String, ExpectedValue>> _naturalLanguageProcessing = {
  _tagCloudBuildX23: {
    _tagTflite: ExpectedValue(mean: 0.9, deviation: 1.15),
  },
  _tagCloudBuildX28: {
    _tagTflite: ExpectedValue(mean: 1, deviation: 1.15),
  },
  _tagRyzen5600: {
    _tagTflite: ExpectedValue(mean: 3, deviation: 1.05),
  },
  _tagPixel5: {
    _tagTflite: ExpectedValue(mean: 2.5, deviation: 1.15),
  },
  _tagPixel6: {
    _tagPixelSo: ExpectedValue(mean: 67, deviation: 1.1),
  },
  _tagIphone12mini: {
    _tagTflite: ExpectedValue(mean: 12, deviation: 1.15),
    _tagCoreml: ExpectedValue(mean: 81, deviation: 1.15),
  },
  _tagIosSimMBP2019: {
    _tagTflite: ExpectedValue(mean: 2.3, deviation: 1.15),
  },
};

const Map<String, Map<String, ExpectedValue>> _imageClassificationOffline = {
  _tagCloudBuildX23: {
    _tagTflite: ExpectedValue(mean: 12, deviation: 1.15),
  },
  _tagCloudBuildX28: {
    _tagTflite: ExpectedValue(mean: 12, deviation: 1.15),
  },
  _tagRyzen5600: {
    _tagTflite: ExpectedValue(mean: 52, deviation: 1.05),
  },
  _tagPixel5: {
    _tagTflite: ExpectedValue(mean: 152, deviation: 1.15),
  },
  _tagPixel6: {
    _tagPixelSo: ExpectedValue(mean: 1550, deviation: 1.1),
  },
  _tagIphone12mini: {
    _tagTflite: ExpectedValue(mean: 1600, deviation: 1.15),
    _tagCoreml: ExpectedValue(mean: 1250, deviation: 1.15),
  },
  _tagIosSimMBP2019: {
    _tagTflite: ExpectedValue(mean: 35, deviation: 1.15),
  },
};

const taskExpectedPerformance = {
  'image_classification': _imageClassification,
  'object_detection': _objectDetection,
  'image_segmentation_v2': _imageSegmentation,
  'natural_language_processing': _naturalLanguageProcessing,
  'image_classification_offline': _imageClassificationOffline,
};
