class Interval {
  final double min;
  final double max;

  const Interval({required this.min, required this.max});
}

const Map<String, Interval> _imageClassification = {
  'CPU': Interval(min: 1.0, max: 1.0),
  'NNAPI': Interval(min: 0.8999999761581421, max: 0.8999999761581421),
  'CoreML': Interval(min: 1.0, max: 1.0),
  'Neural Engine': Interval(min: 1.0, max: 1.0),
};

const Map<String, Interval> _objectDetection = {
  'CPU': Interval(min: 0.3162025809288025, max: 0.3162025809288025),
  'NNAPI': Interval(min: 0.28167346119880676, max: 0.3033106029033661),
  'NNAPI+TFLite-pixel': Interval(min: 0.3609350919723511, max: 0.3657975494861603),
  'CoreML': Interval(min: 0.3162025809288025, max: 0.3162025809288025),
  'Neural Engine': Interval(min: 0.4558801054954529, max: 0.4558801054954529),
};

const Map<String, Interval> _imageSegmentation = {
  'CPU': Interval(min: 0.8387096524238586, max: 0.8387096524238586),
  'NNAPI': Interval(min: 0.48363617062568665, max: 0.4871025085449219),
  'CoreML': Interval(min: 0.9887274503707886, max: 0.9887274503707886),
  'Neural Engine': Interval(min: 0.9890317320823669, max: 0.9890317320823669),
};

const Map<String, Interval> _languageUnderstanding = {
  'CPU': Interval(min: 1.0, max: 1.0),
  'GPU (FP16)': Interval(min: 1.0, max: 1.0),
  'Metal': Interval(min: 1.0, max: 1.0),
  'Neural Engine': Interval(min: 0.800000011920929, max: 0.800000011920929),
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
