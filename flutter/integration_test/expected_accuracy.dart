import 'package:mlperfbench/app_constants.dart';

import 'utils.dart';

/*
value: Interval of expected accuracy
key: <accelerator> OR <accelerator>|<backendName>
- cpu -> Windows
- npu -> Android TFLite
- tpu -> Android Pixel
- ane -> iOS TFLite or Core ML
- cpu|gpu|ane -> IOS Core ML
*/

const Map<String, Interval> _imageClassification = {
  'cpu': Interval(min: 1.00, max: 1.00),
  'npu': Interval(min: 0.89, max: 0.91),
  'tpu': Interval(min: 0.89, max: 0.91),
  'ane': Interval(min: 1.00, max: 1.00),
  'cpu|gpu|ane': Interval(min: 1.00, max: 1.00)
};

const Map<String, Interval> _objectDetection = {
  'cpu': Interval(min: 0.31, max: 0.32),
  'npu': Interval(min: 0.28, max: 0.31),
  'tpu': Interval(min: 0.36, max: 0.38),
  'ane|TFLite': Interval(min: 0.31, max: 0.34),
  'ane|Core ML': Interval(min: 0.45, max: 0.46),
  'cpu|gpu|ane': Interval(min: 0.45, max: 0.46)
};

const Map<String, Interval> _imageSegmentation = {
  'cpu': Interval(min: 0.38, max: 0.40),
  'npu': Interval(min: 0.33, max: 0.34),
  'tpu': Interval(min: 0.33, max: 0.34),
  'ane|TFLite': Interval(min: 0.38, max: 0.40),
  'ane|Core ML': Interval(min: 0.38, max: 0.40),
  'cpu|gpu|ane': Interval(min: 0.38, max: 0.40)
};

const Map<String, Interval> _naturalLanguageProcessing = {
  'cpu': Interval(min: 1.00, max: 1.00),
  'tpu': Interval(min: 1.00, max: 1.00),
  'gpu|TFLite': Interval(min: 1.00, max: 1.00),
  // 1.00 in simulator, 0.80 on iphone 12 mini
  'gpu|Core ML': Interval(min: 0.80, max: 1.00),
  'cpu|gpu|ane': Interval(min: 0.80, max: 1.00)
};

const Map<String, Interval> _superResolution = {
  'cpu': Interval(min: 0.32, max: 0.35),
  'npu': Interval(min: 0.32, max: 0.35),
  'tpu': Interval(min: 0.32, max: 0.35),
  'ane|TFLite': Interval(min: 0.32, max: 0.35),
  'ane|Core ML': Interval(min: 0.32, max: 0.35),
  'cpu|gpu|ane': Interval(min: 0.32, max: 0.35)
};

const benchmarkExpectedAccuracy = {
  BenchmarkId.imageClassification: _imageClassification,
  BenchmarkId.objectDetection: _objectDetection,
  BenchmarkId.imageSegmentationV2: _imageSegmentation,
  BenchmarkId.naturalLanguageProcessing: _naturalLanguageProcessing,
  BenchmarkId.superResolution: _superResolution,
  BenchmarkId.imageClassificationOffline: _imageClassification,
};
