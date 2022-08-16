class Interval {
  final double min;
  final double max;

  const Interval({required this.min, required this.max});
}

const Map<String, Interval> _imageClassification = {
  'CPU': Interval(min: 1.0, max: 1.0),
  'NNAPI': Interval(min: 0.8999, max: 0.9),
  'CoreML': Interval(min: 1.0, max: 1.0),
  'Neural Engine': Interval(min: 1.0, max: 1.0),
};

const Map<String, Interval> _objectDetection = {
  'CPU': Interval(min: 0.3162, max: 0.3163),
  'NNAPI': Interval(min: 0.2816, max: 0.3034),
  'NNAPI+TFLite-pixel': Interval(min: 0.3609, max: 0.3658),
  'CoreML': Interval(min: 0.3162, max: 0.3163),
  'Neural Engine': Interval(min: 0.4558, max: 0.4559),
};

const Map<String, Interval> _imageSegmentation = {
  'CPU': Interval(min: 0.8387, max: 0.8388),
  'NNAPI': Interval(min: 0.4836, max: 0.4872),
  'CoreML': Interval(min: 0.9887, max: 0.9888),
  // accuracy in emulator is 0.83, on real device 0.98
  'Neural Engine': Interval(min: 0.8387, max: 0.9891),
};

const Map<String, Interval> _languageUnderstanding = {
  'CPU': Interval(min: 1.0, max: 1.0),
  'GPU (FP16)': Interval(min: 1.0, max: 1.0),
  'NNAPI': Interval(min: 1.0, max: 1.0),
  'Metal': Interval(min: 1.0, max: 1.0),
  'Neural Engine': Interval(min: 1.0, max: 1.0),
};

const benchmarkExpectedAccuracy = {
  'IC_tpu_float32': _imageClassification,
  'IC_tpu_uint8': _imageClassification,
  'OD_float32': _objectDetection,
  'OD_uint8': _objectDetection,
  'IS_float32_mosaic': _imageSegmentation,
  'IS_uint8_mosaic': _imageSegmentation,
  'LU_float32': _languageUnderstanding,
  'IC_tpu_float32_offline': _imageClassification,
  'IC_tpu_uint8_offline': _imageClassification,
};
