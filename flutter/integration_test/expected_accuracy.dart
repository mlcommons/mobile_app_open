const Map<String, double> _windowsTflite = {
  'IC_tpu_float32': 1.0,
  'OD_float32': 0.3162025809288025,
  'IS_float32_mosaic': 1.0,
  'LU_float32': 1.0,
  'IC_tpu_float32_offline': 1.0,
};

const Map<String, double> _androidTflite = {
  'IC_tpu_uint8': 0.8999999761581421,
  'OD_uint8': 0.28167346119880676,
  'IS_uint8_mosaic': 0.5827004909515381,
  'LU_float32': 1.0,
  'IC_tpu_uint8_offline': 0.8999999761581421,
};

const Map<String, double> _iosTflite = {
  'IC_tpu_float32': 1.0,
  'OD_float32': 0.4558801054954529,
  'IS_float32_mosaic': 0.9890317320823669,
  'LU_float32': 0.800000011920929,
  'IC_tpu_float32_offline': 1.0,
};

const expectedAccuracy = {
  'libtflitebackend.dll': _windowsTflite,
  'libtflitebackend.so': _androidTflite,
  'libtflitepixelbackend.so': _androidTflite,
  'libcoremlbackend.framework/libcoremlbackend': _iosTflite,
};
