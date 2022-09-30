// All valid tags have the following format: os_type+model_code+backend_file
// In this file there are tags that don't match this format.
// Thet are only here as a reference and will not be used in a test
// unless you manually edit the tag to match the format.

/// Windows
// cloudbuild n2-standard-4 machine
const _tagWindowsCI = 'windows+Unknown PC+libtflitebackend.dll';
const _tagWindowsRyzen5600 =
    'windows+Unknown PC+libtflitebackend.dll+ryzen5600x';

/// Android
const _tagPixel5 = 'android+Pixel 5+libtflitebackend.so';
const _tagPixel6 = 'android+Pixel 6+libtflitepixelbackend.so';
// extremely unstable, actual results may vary from 2x lower to 2x higher
const _tagSamsungS21PlusExynosTflite = 'android+SM-G996B+libtflitebackend.so';
// extremely unstable, actual results may vary from 2x lower to 2x higher
const _tagSamsungS0UltraExynosTflite = 'android+SM-N985F+libtflitebackend.so';

/// iPhone
const _tagIphone12miniTflite =
    'ios+iPhone13,1+libtflitebackend.framework/libtflitebackend';
const _tagIphone12miniCoreml =
    'ios+iPhone13,1+libcoremlbackend.framework/libcoremlbackend';

/// iOS Simulator
const _tagIosSimMacincloudTflite =
    'ios+x86_64+libtflitebackend.framework/libtflitebackend';
const _tagIosSimMBP2019Tflite =
    'ios+x86_64+libtflitebackend.framework/libtflitebackend+mbp2019';

const Map<String, double> _imageClassification = {
  _tagWindowsRyzen5600: 34,
  _tagWindowsCI: 10,
  _tagPixel5: 104,
  _tagPixel6: 1015,
  _tagSamsungS0UltraExynosTflite: 64,
  _tagSamsungS21PlusExynosTflite: 68,
  _tagIosSimMacincloudTflite: 4.5,
  _tagIphone12miniTflite: 654,
  _tagIphone12miniCoreml: 545,
  _tagIosSimMBP2019Tflite: 23,
};

const Map<String, double> _objectDetection = {
  _tagWindowsRyzen5600: 18,
  _tagWindowsCI: 4.5,
  _tagPixel5: 48,
  _tagPixel6: 440,
  _tagSamsungS0UltraExynosTflite: 33,
  _tagSamsungS21PlusExynosTflite: 38,
  _tagIosSimMacincloudTflite: 2.2,
  _tagIphone12miniTflite: 259,
  _tagIphone12miniCoreml: 305,
  _tagIosSimMBP2019Tflite: 12.5,
};

const Map<String, double> _imageSegmentation = {
  _tagWindowsRyzen5600: 6,
  _tagWindowsCI: 1.9,
  _tagPixel5: 30,
  _tagPixel6: 160,
  _tagSamsungS0UltraExynosTflite: 15,
  _tagSamsungS21PlusExynosTflite: 14,
  _tagIosSimMacincloudTflite: 0.85,
  _tagIphone12miniTflite: 22,
  _tagIphone12miniCoreml: 75,
  _tagIosSimMBP2019Tflite: 4,
};

const Map<String, double> _naturalLanguageProcessing = {
  _tagWindowsRyzen5600: 3,
  _tagWindowsCI: 1,
  _tagPixel5: 2.5,
  _tagPixel6: 67,
  _tagSamsungS0UltraExynosTflite: 1.5,
  _tagSamsungS21PlusExynosTflite: 1.5,
  _tagIosSimMacincloudTflite: 0.9,
  _tagIphone12miniTflite: 12,
  _tagIphone12miniCoreml: 81,
  _tagIosSimMBP2019Tflite: 2.3,
};

const Map<String, double> _imageClassificationOffline = {
  _tagWindowsRyzen5600: 52,
  _tagWindowsCI: 12,
  _tagPixel5: 152,
  _tagPixel6: 1550,
  _tagSamsungS0UltraExynosTflite: 89,
  _tagSamsungS21PlusExynosTflite: 110,
  _tagIosSimMacincloudTflite: 8,
  _tagIphone12miniTflite: 1600,
  _tagIphone12miniCoreml: 1250,
  _tagIosSimMBP2019Tflite: 35,
};

const taskExpectedPerformance = {
  'image_classification': _imageClassification,
  'object_detection': _objectDetection,
  'image_segmentation_v2': _imageSegmentation,
  'natural_language_processing': _naturalLanguageProcessing,
  'image_classification_offline': _imageClassificationOffline,
};
