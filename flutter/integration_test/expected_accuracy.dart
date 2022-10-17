class Interval {
  final double min;
  final double max;

  const Interval({required this.min, required this.max});
}

// key: <accelerator> OR <accelerator>|<backendName>
const Map<String, Interval> _imageClassification = {
  'cpu': Interval(min: 1.0, max: 1.0),
  'gpu+dsp+npu': Interval(min: 0.8999, max: 0.9),
  'ane': Interval(min: 1.0, max: 1.0),
};

const Map<String, Interval> _objectDetection = {
  'cpu': Interval(min: 0.3162, max: 0.3163),
  'gpu+dsp+npu|TFLite': Interval(min: 0.2816, max: 0.3034),
  'gpu+dsp+npu|TFLite-pixel': Interval(min: 0.3609, max: 0.3658),
  'ane|TFLite': Interval(min: 0.3162, max: 0.3163),
  'ane|Core ML': Interval(min: 0.4558, max: 0.4559),
};

const Map<String, Interval> _imageSegmentation = {
  'cpu': Interval(min: 0.8387, max: 0.8388),
  'gpu+dsp+npu': Interval(min: 0.4836, max: 0.4872),
  'ane|TFLite': Interval(min: 0.80, max: 0.8275),
  'ane|Core ML': Interval(min: 0.8277, max: 0.8388),
};

const Map<String, Interval> _naturalLanguageProcessing = {
  'cpu': Interval(min: 1.0, max: 1.0),
  'gpu+dsp+npu': Interval(min: 1.0, max: 1.0),
  'gpu|TFLite': Interval(min: 1.0, max: 1.0),
  // 1.0 in simulator, 0.8 on iphone 12 mini
  'gpu|Core ML': Interval(min: 0.8, max: 1.0),
};

const benchmarkExpectedAccuracy = {
  'image_classification': _imageClassification,
  'object_detection': _objectDetection,
  'image_segmentation_v2': _imageSegmentation,
  'natural_language_processing': _naturalLanguageProcessing,
  'image_classification_offline': _imageClassification,
};
