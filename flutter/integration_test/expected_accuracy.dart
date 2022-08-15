class Interval {
  final double min;
  final double max;

  const Interval({required this.min, required this.max});
}

const Map<String, Interval> _windowsTflite = {
  'IC_tpu_float32': Interval(min: 1.0, max: 1.0),
  'OD_float32': Interval(min: 0.3162025809288025, max: 0.3162025809288025),
  'IS_float32_mosaic': Interval(min: 0.8387096524238586, max: 0.8387096524238586),
  'LU_float32': Interval(min: 1.0, max: 1.0),
  'IC_tpu_float32_offline': Interval(min: 0.8387096524238586, max: 1.0),
};

const Map<String, Interval> _androidTflite = {
  'IC_tpu_uint8': Interval(min: 0.8999999761581421, max: 0.8999999761581421),
  'OD_uint8': Interval(min: 0.28167346119880676, max: 0.3033106029033661),
  'IS_uint8_mosaic': Interval(min: 0.48363617062568665, max: 0.4871025085449219),
  'LU_float32': Interval(min: 1.0, max: 1.0),
  'IC_tpu_uint8_offline': Interval(min: 0.8999999761581421, max: 0.8999999761581421),
};

const Map<String, Interval> _androidPixel = {
  'IC_tpu_uint8': Interval(min: 0.8999999761581421, max: 0.8999999761581421),
  'OD_uint8': Interval(min: 0.3609350919723511, max: 0.3657975494861603),
  'IS_uint8_mosaic': Interval(min: 0.4859262704849243, max: 0.4859262704849243),
  'LU_float32': Interval(min: 1.0, max: 1.0),
  'IC_tpu_uint8_offline': Interval(min: 0.8999999761581421, max: 0.8999999761581421),
};

const Map<String, Interval> _iosTflite = {
  'IC_tpu_float32': Interval(min: 1.0, max: 1.0),
  'OD_float32': Interval(min: 0.4558801054954529, max: 0.4558801054954529),
  'IS_float32_mosaic': Interval(min: 0.9890317320823669, max: 0.9890317320823669),
  'LU_float32': Interval(min: 0.800000011920929, max: 0.800000011920929),
  'IC_tpu_float32_offline': Interval(min: 1.0, max: 1.0),
};

const backendExpectedAccuracy = {
  'libtflitebackend.dll': _windowsTflite,
  'libtflitebackend.so': _androidTflite,
  'libtflitepixelbackend.so': _androidPixel,
  'libcoremlbackend.framework/libcoremlbackend': _iosTflite,
};
