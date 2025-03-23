import 'package:mlperfbench/app_constants.dart';

import 'utils.dart';

/*
value: Interval of expected throughput
key: model_code (on Android or iOS) or cpuFullName (on Windows)
*/

const _kTFLiteBackend = BackendId.tflite;
const _kPixelBackend = BackendId.pixel;
const _kCoreMLBackend = BackendId.apple;
const _kQtiBackend = BackendId.qti;
const _kMediatekBackend = BackendId.mediatek;
const _kSamsungBackend = BackendId.samsung;

// Windows
// Google Cloud Build n2-standard-4 machine
const _kCloudBuildX23 = 'Intel Xeon 2.30GHz';
const _kCloudBuildX28 = 'Intel Xeon 2.80GHz';
const _kRyzen5600 = 'AMD Ryzen 5 5600X 6-Core';

// Android devices on Firebase TestLab
const _kPixel5 = 'Pixel 5'; // Google Pixel 5
const _kPixel6 = 'Pixel 6'; // Google Pixel 6
const _kS22Ultra = 'SM-S908U1'; // Samsung Galaxy S22 Ultra
const _kDN2103 = 'DN2103'; // OnePlus DN2103

// Android devices on BrowserStack App Automate
const _kPixel9Pro = 'Pixel 9 Pro'; // Google Pixel 9 Pro
const _kS24 = 'SM-S921B'; // Samsung Galaxy S24
const _kS24Ultra = 'SM-S928B'; // Samsung Galaxy S24 Ultra
const _kS25Ultra = 'SM-S938B'; // Samsung Galaxy S25 Ultra
const _kS10Plus = 'SM-X826B'; // Samsung Galaxy Tab S10 Plus
const _kM32 = 'SM-M326B'; // Samsung Galaxy M32

// iOS
const _kIphoneOnGitHubAction = 'iPhone16,2';
const _kIphoneOnMacbookM1 = 'iPhone14,7';

const Map<String, Map<String, Interval>> _imageClassificationV2 = {
  _kTFLiteBackend: {
    _kCloudBuildX23: Interval(min: 0.5, max: 9),
    _kCloudBuildX28: Interval(min: 0.5, max: 9),
    _kRyzen5600: Interval(min: 1, max: 37),
    _kPixel5: Interval(min: 20, max: 75),
    _kPixel6: Interval(min: 100, max: 600),
    _kM32: Interval(min: 5, max: 15),
    _kIphoneOnGitHubAction: Interval(min: 0.4, max: 9),
    _kIphoneOnMacbookM1: Interval(min: 10, max: 27),
  },
  _kCoreMLBackend: {
    _kIphoneOnGitHubAction: Interval(min: 0.4, max: 9),
  },
  _kPixelBackend: {
    _kPixel6: Interval(min: 100, max: 600),
    _kPixel9Pro: Interval(min: 100, max: 600),
  },
  _kQtiBackend: {
    _kS22Ultra: Interval(min: 250, max: 400),
    _kS24Ultra: Interval(min: 800, max: 1500),
    _kS25Ultra: Interval(min: 900, max: 2200),
  },
  _kMediatekBackend: {
    _kDN2103: Interval(min: 4.5, max: 90),
    _kS10Plus: Interval(min: 400, max: 800)
  },
  _kSamsungBackend: {
    _kS24: Interval(min: 600, max: 1000),
  },
};

const Map<String, Map<String, Interval>> _objectDetection = {
  _kTFLiteBackend: {
    _kCloudBuildX23: Interval(min: 2, max: 7),
    _kCloudBuildX28: Interval(min: 2, max: 8),
    _kRyzen5600: Interval(min: 14, max: 22),
    _kPixel5: Interval(min: 40, max: 60),
    _kPixel6: Interval(min: 100, max: 450),
    _kM32: Interval(min: 20, max: 40),
    _kIphoneOnGitHubAction: Interval(min: 0.5, max: 4),
    _kIphoneOnMacbookM1: Interval(min: 9, max: 16),
  },
  _kCoreMLBackend: {
    _kIphoneOnGitHubAction: Interval(min: 0.5, max: 7),
  },
  _kPixelBackend: {
    _kPixel6: Interval(min: 200, max: 500),
    _kPixel9Pro: Interval(min: 200, max: 500),
  },
  _kQtiBackend: {
    _kS22Ultra: Interval(min: 700, max: 1400),
    _kS24Ultra: Interval(min: 1800, max: 2700),
    _kS25Ultra: Interval(min: 2000, max: 3500),
  },
  _kMediatekBackend: {
    _kDN2103: Interval(min: 120, max: 210),
    _kS10Plus: Interval(min: 1200, max: 2000)
  },
  _kSamsungBackend: {
    _kS24: Interval(min: 1400, max: 2400),
  },
};

const Map<String, Map<String, Interval>> _imageSegmentationV2 = {
  _kTFLiteBackend: {
    _kCloudBuildX23: Interval(min: 0.5, max: 3),
    _kCloudBuildX28: Interval(min: 0.5, max: 4),
    _kRyzen5600: Interval(min: 5, max: 7),
    _kPixel5: Interval(min: 25, max: 40),
    _kPixel6: Interval(min: 80, max: 180),
    _kM32: Interval(min: 2, max: 10),
    _kIphoneOnGitHubAction: Interval(min: 0.1, max: 2.5),
    _kIphoneOnMacbookM1: Interval(min: 3, max: 6),
  },
  _kCoreMLBackend: {
    _kIphoneOnGitHubAction: Interval(min: 0.1, max: 3.5),
  },
  _kPixelBackend: {
    _kPixel6: Interval(min: 100, max: 190),
    _kPixel9Pro: Interval(min: 200, max: 500),
  },
  _kQtiBackend: {
    _kS22Ultra: Interval(min: 400, max: 750),
    _kS24Ultra: Interval(min: 1200, max: 1600),
    _kS25Ultra: Interval(min: 1500, max: 2200),
  },
  _kMediatekBackend: {
    _kDN2103: Interval(min: 45, max: 70),
    _kS10Plus: Interval(min: 800, max: 1500)
  },
  _kSamsungBackend: {
    _kS24: Interval(min: 800, max: 1500),
  },
};

const Map<String, Map<String, Interval>> _naturalLanguageProcessing = {
  _kTFLiteBackend: {
    _kCloudBuildX23: Interval(min: 0.7, max: 1.2),
    _kCloudBuildX28: Interval(min: 0.5, max: 1.4),
    _kRyzen5600: Interval(min: 2.8, max: 3.2),
    _kPixel5: Interval(min: 2.3, max: 3.0),
    _kPixel6: Interval(min: 2, max: 75),
    _kM32: Interval(min: 2, max: 5),
    _kIphoneOnGitHubAction: Interval(min: 0.1, max: 1),
    _kIphoneOnMacbookM1: Interval(min: 1.8, max: 3),
  },
  _kCoreMLBackend: {
    _kIphoneOnGitHubAction: Interval(min: 0.1, max: 1.1),
  },
  _kPixelBackend: {
    _kPixel6: Interval(min: 2, max: 85),
    _kPixel9Pro: Interval(min: 60, max: 150),
  },
  _kQtiBackend: {
    _kS22Ultra: Interval(min: 100, max: 200),
    _kS24Ultra: Interval(min: 250, max: 460),
    _kS25Ultra: Interval(min: 350, max: 800),
  },
  _kMediatekBackend: {
    _kDN2103: Interval(min: 1, max: 6),
    _kS10Plus: Interval(min: 100, max: 300)
  },
  _kSamsungBackend: {
    _kS24: Interval(min: 220, max: 350),
  },
};

const Map<String, Map<String, Interval>> _superResolution = {
  _kTFLiteBackend: {
    _kCloudBuildX23: Interval(min: 0.1, max: 3),
    _kCloudBuildX28: Interval(min: 0.1, max: 4),
    _kRyzen5600: Interval(min: 0.1, max: 3),
    _kPixel5: Interval(min: 4, max: 8),
    _kPixel6: Interval(min: 7, max: 14),
    _kM32: Interval(min: 0.1, max: 3),
    _kIphoneOnGitHubAction: Interval(min: 0.02, max: 1.0),
    _kIphoneOnMacbookM1: Interval(min: 0.1, max: 10),
  },
  _kCoreMLBackend: {
    _kIphoneOnGitHubAction: Interval(min: 0.02, max: 1.0),
  },
  _kPixelBackend: {
    _kPixel6: Interval(min: 7, max: 17),
    _kPixel9Pro: Interval(min: 30, max: 70),
  },
  _kQtiBackend: {
    _kS22Ultra: Interval(min: 25, max: 75),
    _kS24Ultra: Interval(min: 120, max: 180),
    _kS25Ultra: Interval(min: 200, max: 340),
  },
  _kMediatekBackend: {
    _kDN2103: Interval(min: 5, max: 15),
    _kS10Plus: Interval(min: 150, max: 300)
  },
  _kSamsungBackend: {
    _kS24: Interval(min: 90, max: 180),
  },
};

// TODO (anhappdev): update expected throughput for stable diffusion
const Map<String, Map<String, Interval>> _stableDiffusion = {
  _kTFLiteBackend: {
    _kCloudBuildX23: Interval(min: 0, max: 100),
    _kCloudBuildX28: Interval(min: 0, max: 100),
    _kRyzen5600: Interval(min: 0, max: 100),
    _kPixel5: Interval(min: 0, max: 100),
    _kPixel6: Interval(min: 0, max: 100),
    _kM32: Interval(min: 0, max: 100),
    _kIphoneOnGitHubAction: Interval(min: 0, max: 100),
    _kIphoneOnMacbookM1: Interval(min: 0, max: 100),
  },
  _kCoreMLBackend: {
    _kIphoneOnGitHubAction: Interval(min: 0, max: 100),
  },
  _kPixelBackend: {
    _kPixel6: Interval(min: 0, max: 100),
    _kPixel9Pro: Interval(min: 0, max: 100),
  },
  _kQtiBackend: {
    _kS22Ultra: Interval(min: 0, max: 100),
    _kS24Ultra: Interval(min: 0, max: 100),
    _kS25Ultra: Interval(min: 0, max: 100),
  },
  _kMediatekBackend: {
    _kDN2103: Interval(min: 0, max: 100),
    _kS10Plus: Interval(min: 0, max: 100)
  },
  _kSamsungBackend: {
    _kS24: Interval(min: 0, max: 100),
  },
};

const Map<String, Map<String, Interval>> _imageClassificationOfflineV2 = {
  _kTFLiteBackend: {
    _kCloudBuildX23: Interval(min: 0.8, max: 9),
    _kCloudBuildX28: Interval(min: 0.8, max: 9),
    _kRyzen5600: Interval(min: 20, max: 60),
    _kPixel5: Interval(min: 20, max: 180),
    _kPixel6: Interval(min: 100, max: 700),
    _kM32: Interval(min: 8, max: 18),
    _kIphoneOnGitHubAction: Interval(min: 0.4, max: 15),
    _kIphoneOnMacbookM1: Interval(min: 10, max: 45),
  },
  _kCoreMLBackend: {
    _kIphoneOnGitHubAction: Interval(min: 0.4, max: 15),
  },
  _kPixelBackend: {
    _kPixel6: Interval(min: 100, max: 700),
    _kPixel9Pro: Interval(min: 300, max: 700),
  },
  _kQtiBackend: {
    _kS22Ultra: Interval(min: 250, max: 450),
    _kS24Ultra: Interval(min: 900, max: 1700),
    _kS25Ultra: Interval(min: 1200, max: 2200),
  },
  _kMediatekBackend: {
    _kDN2103: Interval(min: 4.5, max: 90),
    _kS10Plus: Interval(min: 700, max: 1200)
  },
  _kSamsungBackend: {
    _kS24: Interval(min: 800, max: 1200),
  },
};

const benchmarkExpectedThroughput = {
  BenchmarkId.imageClassificationV2: _imageClassificationV2,
  BenchmarkId.objectDetection: _objectDetection,
  BenchmarkId.imageSegmentationV2: _imageSegmentationV2,
  BenchmarkId.naturalLanguageProcessing: _naturalLanguageProcessing,
  BenchmarkId.superResolution: _superResolution,
  BenchmarkId.stableDiffusion: _stableDiffusion,
  BenchmarkId.imageClassificationOfflineV2: _imageClassificationOfflineV2,
};
