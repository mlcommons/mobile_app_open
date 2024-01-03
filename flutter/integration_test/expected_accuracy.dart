import 'package:mlperfbench/app_constants.dart';

import 'utils.dart';

/*
value: Interval of expected accuracy
key: <accelerator> OR <accelerator>|<backendName>
- cpu -> Windows
- npu -> Android TFLite
- tpu -> Android Pixel
- ane -> iOS TFLite or Core ML
- cpu&gpu&ane -> IOS Core ML
*/
const _kCpu = 'cpu';
const _kNpu = 'npu';
const _kTpu = 'tpu';
const _kAne = 'ane';
const _kCpuGpuAne = 'cpu&gpu&ane';
const _kAneTflite = 'ane|TFLite';
const _kAneCoreml = 'ane|Core ML';
const _kGpuTflite = 'gpu|TFLite';
const _kGpuCoreml = 'gpu|Core ML';

const Map<String, Interval> _imageClassification = {
  _kCpu: Interval(min: 1.00, max: 1.00),
  _kNpu: Interval(min: 0.89, max: 0.91),
  _kTpu: Interval(min: 0.89, max: 0.91),
  _kAne: Interval(min: 1.00, max: 1.00),
  _kCpuGpuAne: Interval(min: 1.00, max: 1.00)
};

const Map<String, Interval> _imageClassificationV2 = {
  _kCpu: Interval(min: 0.69, max: 0.71),
  _kNpu: Interval(min: 0.69, max: 0.71),
  _kTpu: Interval(min: 0.69, max: 0.71),
  _kAne: Interval(min: 0.69, max: 0.71),
  _kCpuGpuAne: Interval(min: 0.69, max: 0.71),
};

const Map<String, Interval> _objectDetection = {
  _kCpu: Interval(min: 0.31, max: 0.32),
  _kNpu: Interval(min: 0.28, max: 0.31),
  _kTpu: Interval(min: 0.36, max: 0.38),
  _kAneTflite: Interval(min: 0.31, max: 0.34),
  _kAneCoreml: Interval(min: 0.45, max: 0.46),
  _kCpuGpuAne: Interval(min: 0.45, max: 0.46)
};

const Map<String, Interval> _imageSegmentation = {
  _kCpu: Interval(min: 0.38, max: 0.40),
  _kNpu: Interval(min: 0.33, max: 0.34),
  _kTpu: Interval(min: 0.33, max: 0.34),
  _kAneTflite: Interval(min: 0.38, max: 0.40),
  _kAneCoreml: Interval(min: 0.38, max: 0.40),
  _kCpuGpuAne: Interval(min: 0.38, max: 0.40)
};

const Map<String, Interval> _naturalLanguageProcessing = {
  _kCpu: Interval(min: 1.00, max: 1.00),
  _kTpu: Interval(min: 1.00, max: 1.00),
  _kGpuTflite: Interval(min: 1.00, max: 1.00),
  // 1.00 in simulator, 0.80 on iphone 12 mini
  _kGpuCoreml: Interval(min: 0.80, max: 1.00),
  _kCpuGpuAne: Interval(min: 0.80, max: 1.00)
};

const Map<String, Interval> _superResolution = {
  _kCpu: Interval(min: 0.32, max: 0.35),
  _kNpu: Interval(min: 0.32, max: 0.35),
  _kTpu: Interval(min: 0.32, max: 0.35),
  _kAneTflite: Interval(min: 0.32, max: 0.35),
  _kAneCoreml: Interval(min: 0.32, max: 0.35),
  _kCpuGpuAne: Interval(min: 0.32, max: 0.35)
};

const benchmarkExpectedAccuracy = {
  BenchmarkId.imageClassification: _imageClassification,
  BenchmarkId.imageClassificationV2: _imageClassificationV2,
  BenchmarkId.objectDetection: _objectDetection,
  BenchmarkId.imageSegmentationV2: _imageSegmentation,
  BenchmarkId.naturalLanguageProcessing: _naturalLanguageProcessing,
  BenchmarkId.superResolution: _superResolution,
  BenchmarkId.imageClassificationOffline: _imageClassification,
  BenchmarkId.imageClassificationOfflineV2: _imageClassificationV2,
};
