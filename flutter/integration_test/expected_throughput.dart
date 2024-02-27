import 'package:mlperfbench/app_constants.dart';

import 'utils.dart';

/*
value: Interval of expected throughput
key: model_code (on Android or iOS) or cpuFullName (on Windows)
*/

const _kTFLiteBackend = 'libtflitebackend';
const _kPixelBackend = 'libtflitepixelbackend';
const _kCoreMLBackend = 'libcoremlbackend';
const _kQtiBackend = 'libqtibackend';
const _kMediatekBackend = 'libtfliteneuronbackend';

// Windows
// Google Cloud Build n2-standard-4 machine
const _kCloudBuildX23 = 'Intel Xeon 2.30GHz';
const _kCloudBuildX28 = 'Intel Xeon 2.80GHz';
const _kRyzen5600 = 'AMD Ryzen 5 5600X 6-Core';

// Android
const _kPixel5 = 'Pixel 5'; // Google Pixel 5
const _kPixel6 = 'Pixel 6'; // Google Pixel 6
const _kS22Ultra = 'SM-S908U1'; // Galaxy S22 Ultra
const _kDN2103 = 'DN2103'; // OnePlus DN2103

// iOS
const _kIphoneOnGitHubAction = 'iPhone15,3';
const _kIphoneOnMacbookM1 = 'iPhone14,7';

const Map<String, Map<String, Interval>> _imageClassification = {
  _kTFLiteBackend: {
    _kCloudBuildX23: Interval(min: 4, max: 12),
    _kCloudBuildX28: Interval(min: 4, max: 13),
    _kRyzen5600: Interval(min: 31, max: 37),
    _kPixel5: Interval(min: 80, max: 130),
    _kIphoneOnGitHubAction: Interval(min: 1, max: 8),
    _kIphoneOnMacbookM1: Interval(min: 19, max: 27),
  },
  _kCoreMLBackend: {
    _kIphoneOnGitHubAction: Interval(min: 1, max: 8),
  },
  _kPixelBackend: {
    _kPixel6: Interval(min: 800, max: 1100),
  },
  _kQtiBackend: {
    _kS22Ultra: Interval(min: 1900, max: 2400),
  },
  _kMediatekBackend: {
    _kDN2103: Interval(min: 30, max: 50),
  },
};

// TODO (anhappdev): update the expected value for _imageClassificationV2 after gathering some statistics
const Map<String, Map<String, Interval>> _imageClassificationV2 = {
  _kTFLiteBackend: {
    _kCloudBuildX23: Interval(min: 1, max: 9),
    _kCloudBuildX28: Interval(min: 1, max: 9),
    _kRyzen5600: Interval(min: 1, max: 37),
    _kPixel5: Interval(min: 20, max: 75),
    _kIphoneOnGitHubAction: Interval(min: 0.4, max: 7),
    _kIphoneOnMacbookM1: Interval(min: 10, max: 27),
  },
  _kCoreMLBackend: {
    _kIphoneOnGitHubAction: Interval(min: 0.4, max: 7),
  },
  _kPixelBackend: {
    _kPixel6: Interval(min: 100, max: 600),
  },
  _kQtiBackend: {
    _kS22Ultra: Interval(min: 1900, max: 2200),
  },
  _kMediatekBackend: {
    _kDN2103: Interval(min: 5, max: 50),
  },
};

const Map<String, Map<String, Interval>> _objectDetection = {
  _kTFLiteBackend: {
    _kCloudBuildX23: Interval(min: 4, max: 7),
    _kCloudBuildX28: Interval(min: 3.5, max: 8),
    _kRyzen5600: Interval(min: 14, max: 22),
    _kPixel5: Interval(min: 40, max: 60),
    _kIphoneOnGitHubAction: Interval(min: 0.5, max: 4),
    _kIphoneOnMacbookM1: Interval(min: 9, max: 16),
  },
  _kCoreMLBackend: {
    _kIphoneOnGitHubAction: Interval(min: 0.5, max: 4),
  },
  _kPixelBackend: {
    _kPixel6: Interval(min: 200, max: 450),
  },
  _kQtiBackend: {
    _kS22Ultra: Interval(min: 800, max: 1400),
  },
  _kMediatekBackend: {
    _kDN2103: Interval(min: 120, max: 180),
  },
};

const Map<String, Map<String, Interval>> _imageSegmentationV2 = {
  _kTFLiteBackend: {
    _kCloudBuildX23: Interval(min: 0.5, max: 3),
    _kCloudBuildX28: Interval(min: 0.5, max: 4),
    _kRyzen5600: Interval(min: 5, max: 7),
    _kPixel5: Interval(min: 25, max: 40),
    _kIphoneOnGitHubAction: Interval(min: 0.1, max: 2.5),
    _kIphoneOnMacbookM1: Interval(min: 3, max: 6),
  },
  _kCoreMLBackend: {
    _kIphoneOnGitHubAction: Interval(min: 0.1, max: 2.5),
  },
  _kPixelBackend: {
    _kPixel6: Interval(min: 100, max: 180),
  },
  _kQtiBackend: {
    _kS22Ultra: Interval(min: 450, max: 700),
  },
  _kMediatekBackend: {
    _kDN2103: Interval(min: 45, max: 65),
  },
};

const Map<String, Map<String, Interval>> _naturalLanguageProcessing = {
  _kTFLiteBackend: {
    _kCloudBuildX23: Interval(min: 0.7, max: 1.1),
    _kCloudBuildX28: Interval(min: 0.5, max: 1.3),
    _kRyzen5600: Interval(min: 2.8, max: 3.2),
    _kPixel5: Interval(min: 2.3, max: 3.0),
    _kIphoneOnGitHubAction: Interval(min: 0.1, max: 1),
    _kIphoneOnMacbookM1: Interval(min: 1.8, max: 3),
  },
  _kCoreMLBackend: {
    _kIphoneOnGitHubAction: Interval(min: 0.1, max: 1),
  },
  _kPixelBackend: {
    // pixel some time finish this task in 4 seconds, not sure why.
    _kPixel6: Interval(min: 2, max: 75),
  },
  _kQtiBackend: {
    _kS22Ultra: Interval(min: 120, max: 180),
  },
  _kMediatekBackend: {
    _kDN2103: Interval(min: 1, max: 5),
  },
};

const Map<String, Map<String, Interval>> _superResolution = {
  _kTFLiteBackend: {
    _kCloudBuildX23: Interval(min: 0.1, max: 3),
    _kCloudBuildX28: Interval(min: 0.1, max: 4),
    _kRyzen5600: Interval(min: 0.1, max: 3),
    _kPixel5: Interval(min: 4, max: 8),
    _kIphoneOnGitHubAction: Interval(min: 0.02, max: 1.0),
    _kIphoneOnMacbookM1: Interval(min: 0.1, max: 10),
  },
  _kCoreMLBackend: {
    _kIphoneOnGitHubAction: Interval(min: 0.02, max: 1.0),
  },
  _kPixelBackend: {
    _kPixel6: Interval(min: 10, max: 14),
  },
  _kQtiBackend: {
    _kS22Ultra: Interval(min: 35, max: 55),
  },
  _kMediatekBackend: {
    _kDN2103: Interval(min: 5, max: 12),
  },
};

const Map<String, Map<String, Interval>> _imageClassificationOffline = {
  _kTFLiteBackend: {
    _kCloudBuildX23: Interval(min: 8, max: 14),
    _kCloudBuildX28: Interval(min: 7, max: 16),
    _kRyzen5600: Interval(min: 45, max: 60),
    _kPixel5: Interval(min: 120, max: 190),
    _kIphoneOnGitHubAction: Interval(min: 2, max: 15),
    _kIphoneOnMacbookM1: Interval(min: 30, max: 45),
  },
  _kCoreMLBackend: {
    _kIphoneOnGitHubAction: Interval(min: 2, max: 20),
  },
  _kPixelBackend: {
    _kPixel6: Interval(min: 1000, max: 1700),
  },
  _kQtiBackend: {
    _kS22Ultra: Interval(min: 2600, max: 3500),
  },
  _kMediatekBackend: {
    _kDN2103: Interval(min: 75, max: 110),
  },
};

// TODO (anhappdev): update the expected value for _imageClassificationOfflineV2 after gathering some statistics
const Map<String, Map<String, Interval>> _imageClassificationOfflineV2 = {
  _kTFLiteBackend: {
    _kCloudBuildX23: Interval(min: 1, max: 9),
    _kCloudBuildX28: Interval(min: 1, max: 9),
    _kRyzen5600: Interval(min: 20, max: 60),
    _kPixel5: Interval(min: 20, max: 180),
    _kIphoneOnGitHubAction: Interval(min: 0.4, max: 15),
    _kIphoneOnMacbookM1: Interval(min: 10, max: 45),
  },
  _kCoreMLBackend: {
    _kIphoneOnGitHubAction: Interval(min: 0.4, max: 15),
  },
  _kPixelBackend: {
    _kPixel6: Interval(min: 100, max: 1700),
  },
  _kQtiBackend: {
    _kS22Ultra: Interval(min: 2600, max: 3000),
  },
  _kMediatekBackend: {
    _kDN2103: Interval(min: 5, max: 90),
  },
};

const benchmarkExpectedThroughput = {
  BenchmarkId.imageClassification: _imageClassification,
  BenchmarkId.imageClassificationV2: _imageClassificationV2,
  BenchmarkId.objectDetection: _objectDetection,
  BenchmarkId.imageSegmentationV2: _imageSegmentationV2,
  BenchmarkId.naturalLanguageProcessing: _naturalLanguageProcessing,
  BenchmarkId.superResolution: _superResolution,
  BenchmarkId.imageClassificationOffline: _imageClassificationOffline,
  BenchmarkId.imageClassificationOfflineV2: _imageClassificationOfflineV2,
};
