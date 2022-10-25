class Interval {
  final double min;
  final double max;

  const Interval({required this.min, required this.max});
}

/*
value: Interval of expected accuracy
key: <accelerator> OR <accelerator>|<backendName>
- cpu -> Windows
- npu -> Android TFLite
- tpu -> Android Pixel
- ane -> iOS TFLite or Core ML
*/

const Map<String, Interval> _imageClassification = {
  'cpu': Interval(min: 1.00, max: 1.00),
  'npu': Interval(min: 0.89, max: 0.91),
  'tpu': Interval(min: 0.89, max: 0.91),
  'ane': Interval(min: 1.00, max: 1.00),
};

const Map<String, Interval> _objectDetection = {
  'cpu': Interval(min: 0.31, max: 0.32),
  'npu': Interval(min: 0.28, max: 0.31),
  'tpu': Interval(min: 0.36, max: 0.38),
  'ane|TFLite': Interval(min: 0.31, max: 0.32),
  'ane|Core ML': Interval(min: 0.45, max: 0.46),
};

const Map<String, Interval> _imageSegmentation = {
  'cpu': Interval(min: 0.83, max: 0.84),
  'npu': Interval(min: 0.48, max: 0.49),
  'tpu': Interval(min: 0.48, max: 0.49),
  'ane|TFLite': Interval(min: 0.80, max: 0.83),
  'ane|Core ML': Interval(min: 0.82, max: 0.84),
};

const Map<String, Interval> _naturalLanguageProcessing = {
  'cpu': Interval(min: 1.00, max: 1.00),
  'tpu': Interval(min: 1.00, max: 1.00),
  'gpu|TFLite': Interval(min: 1.00, max: 1.00),
  // 1.00 in simulator, 0.80 on iphone 12 mini
  'gpu|Core ML': Interval(min: 0.80, max: 1.00),
};

const benchmarkExpectedAccuracy = {
  'image_classification': _imageClassification,
  'object_detection': _objectDetection,
  'image_segmentation_v2': _imageSegmentation,
  'natural_language_processing': _naturalLanguageProcessing,
  'image_classification_offline': _imageClassification,
};
