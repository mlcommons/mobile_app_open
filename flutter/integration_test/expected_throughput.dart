import 'package:mlperfbench/app_constants.dart';

import 'utils.dart';

/*
value: Interval of expected throughput
key: model_code (on Android or iOS) or cpuFullName (on Windows)
*/

const _kTFLiteBackend = 'libtflitebackend';
const _kPixelBackend = 'libtflitepixelbackend';
const _kCoreMLBackend = 'libcoremlbackend';

// Windows
// Google Cloud Build n2-standard-4 machine
const _kCloudBuildX23 = 'Intel Xeon 2.30GHz';
const _kCloudBuildX28 = 'Intel Xeon 2.80GHz';
const _kRyzen5600 = 'AMD Ryzen 5 5600X 6-Core';

// Android
const _kPixel5 = 'Pixel 5';
const _kPixel6 = 'Pixel 6';

// iOS
const _kIphoneOnGitHubAction = 'iPhone15,3';
const _kIphoneOnMacbookM1 = 'iPhone14,7';

const Map<String, Map<String, Interval>> _imageClassification = {
  _kTFLiteBackend: {
    _kCloudBuildX23: Interval(min: 4, max: 12),
    _kCloudBuildX28: Interval(min: 4, max: 13),
    _kRyzen5600: Interval(min: 31, max: 37),
    _kPixel5: Interval(min: 80, max: 130),
    _kIphoneOnGitHubAction: Interval(min: 2, max: 8),
    _kIphoneOnMacbookM1: Interval(min: 19, max: 27),
  },
  _kCoreMLBackend: {
    _kIphoneOnGitHubAction: Interval(min: 2, max: 8),
  },
  _kPixelBackend: {
    _kPixel6: Interval(min: 800, max: 1100),
  },
};

const Map<String, Map<String, Interval>> _objectDetection = {
  _kTFLiteBackend: {
    _kCloudBuildX23: Interval(min: 4, max: 7),
    _kCloudBuildX28: Interval(min: 3.5, max: 8),
    _kRyzen5600: Interval(min: 14, max: 22),
    _kPixel5: Interval(min: 40, max: 60),
    _kIphoneOnGitHubAction: Interval(min: 1.5, max: 4),
    _kIphoneOnMacbookM1: Interval(min: 9, max: 16),
  },
  _kCoreMLBackend: {
    _kIphoneOnGitHubAction: Interval(min: 1.5, max: 4),
  },
  _kPixelBackend: {
    _kPixel6: Interval(min: 300, max: 490),
  },
};

const Map<String, Map<String, Interval>> _imageSegmentation = {
  _kTFLiteBackend: {
    _kCloudBuildX23: Interval(min: 0.5, max: 3),
    _kCloudBuildX28: Interval(min: 0.5, max: 4),
    _kRyzen5600: Interval(min: 5, max: 7),
    _kPixel5: Interval(min: 25, max: 40),
    _kIphoneOnGitHubAction: Interval(min: 0.5, max: 2.5),
    _kIphoneOnMacbookM1: Interval(min: 3, max: 6),
  },
  _kCoreMLBackend: {
    _kIphoneOnGitHubAction: Interval(min: 0.5, max: 2.5),
  },
  _kPixelBackend: {
    _kPixel6: Interval(min: 100, max: 180),
  },
};

const Map<String, Map<String, Interval>> _naturalLanguageProcessing = {
  _kTFLiteBackend: {
    _kCloudBuildX23: Interval(min: 0.7, max: 1.1),
    _kCloudBuildX28: Interval(min: 0.5, max: 1.3),
    _kRyzen5600: Interval(min: 2.8, max: 3.2),
    _kPixel5: Interval(min: 2.3, max: 3.0),
    _kIphoneOnGitHubAction: Interval(min: 0.3, max: 1),
    _kIphoneOnMacbookM1: Interval(min: 1.8, max: 3),
  },
  _kCoreMLBackend: {
    _kIphoneOnGitHubAction: Interval(min: 0.3, max: 1),
  },
  _kPixelBackend: {
    // pixel some time finish this task in 4 seconds, not sure why.
    _kPixel6: Interval(min: 2, max: 75),
  },
};

const Map<String, Map<String, Interval>> _imageClassificationOffline = {
  _kTFLiteBackend: {
    _kCloudBuildX23: Interval(min: 8, max: 14),
    _kCloudBuildX28: Interval(min: 7, max: 16),
    _kRyzen5600: Interval(min: 45, max: 60),
    _kPixel5: Interval(min: 120, max: 190),
    _kIphoneOnGitHubAction: Interval(min: 3, max: 15),
    _kIphoneOnMacbookM1: Interval(min: 30, max: 45),
  },
  _kCoreMLBackend: {
    _kIphoneOnGitHubAction: Interval(min: 3, max: 15),
  },
  _kPixelBackend: {
    _kPixel6: Interval(min: 1000, max: 1700),
  },
};

// TODO (anhappdev): update min throughput for _superResolution after we gather some statistic
const Map<String, Map<String, Interval>> _superResolution = {
  _kTFLiteBackend: {
    _kCloudBuildX23: Interval(min: 0.1, max: 3),
    _kCloudBuildX28: Interval(min: 0.1, max: 4),
    _kRyzen5600: Interval(min: 0.1, max: 3),
    _kPixel5: Interval(min: 4, max: 8),
    _kIphoneOnGitHubAction: Interval(min: 0.1, max: 3),
    _kIphoneOnMacbookM1: Interval(min: 0.1, max: 10),
  },
  _kCoreMLBackend: {
    _kIphoneOnGitHubAction: Interval(min: 0.1, max: 3),
  },
  _kPixelBackend: {
    _kPixel6: Interval(min: 10, max: 14),
  },
};

const benchmarkExpectedThroughput = {
  BenchmarkId.imageClassification: _imageClassification,
  BenchmarkId.objectDetection: _objectDetection,
  BenchmarkId.imageSegmentationV2: _imageSegmentation,
  BenchmarkId.naturalLanguageProcessing: _naturalLanguageProcessing,
  BenchmarkId.superResolution: _superResolution,
  BenchmarkId.imageClassificationOffline: _imageClassificationOffline,
};
