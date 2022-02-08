import 'package:flutter_svg/svg.dart';

import 'package:mlperfbench/icons.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/protos/mlperf_task.pb.dart' as pb;

enum BenchmarkTypeEnum {
  unknown,
  imageClassification,
  objectDetection,
  imageSegmentation,
  languageUnderstanding,
}

class BenchmarkLocalizationInfo {
  final String name;
  final String detailsTitle;
  final String detailsContent;

  BenchmarkLocalizationInfo(
      {required this.name,
      required this.detailsTitle,
      required this.detailsContent});
}

class BenchmarkInfo {
  final pb.ModelConfig modelConfig;

  /// 'Object Detection', 'Image Classification (offline)', and so on.
  final String taskName;

  BenchmarkInfo(this.modelConfig, this.taskName);

  String get name => modelConfig.name;

  BenchmarkLocalizationInfo getLocalizedInfo(AppLocalizations stringResources) {
    switch (code) {
      case ('IC'):
        if (isOffline) {
          return BenchmarkLocalizationInfo(
              name: stringResources.imageClassificationOffline,
              detailsTitle: stringResources.icInfo,
              detailsContent: stringResources.icInfoDescription);
        } else {
          return BenchmarkLocalizationInfo(
              name: stringResources.imageClassification,
              detailsTitle: stringResources.icInfo,
              detailsContent: stringResources.icInfoDescription);
        }
      case ('OD'):
        return BenchmarkLocalizationInfo(
            name: stringResources.objectDetection,
            detailsTitle: stringResources.odInfo,
            detailsContent: stringResources.odInfoDescription);
      case ('IS'):
        return BenchmarkLocalizationInfo(
            name: stringResources.imageSegmentation,
            detailsTitle: stringResources.isInfo,
            detailsContent: stringResources.isInfoDescription);
      case ('LU'):
        return BenchmarkLocalizationInfo(
            name: stringResources.languageProcessing,
            detailsTitle: stringResources.luInfo,
            detailsContent: stringResources.luInfoDescription);
      default:
        throw 'unhandled benchmark code: $code';
    }
  }

  bool get isOffline => modelConfig.scenario == 'Offline';

  double get maxThroughput => _MAX_THROUGHPUT[modelConfig.id]!;

  /// 'IC', 'OD', and so on.
  String get code => modelConfig.id.split('_').first;

  /// 'SingleStream' or 'Offline'.
  String get scenario => modelConfig.scenario;

  BenchmarkTypeEnum get type => _typeFromCode();

  SvgPicture get icon => _BENCHMARK_ICONS[scenario]?[code] ?? Icons.logo;

  SvgPicture get iconWhite =>
      _BENCHMARK_ICONS_WHITE[scenario]?[code] ?? Icons.logo;

  @override
  String toString() => 'Benchmark:${modelConfig.id}';

  BenchmarkTypeEnum _typeFromCode() {
    switch (code) {
      case 'IC':
        return BenchmarkTypeEnum.imageClassification;
      case 'OD':
        return BenchmarkTypeEnum.objectDetection;
      case 'IS':
        return BenchmarkTypeEnum.imageSegmentation;
      case 'LU':
        return BenchmarkTypeEnum.languageUnderstanding;
      default:
        return BenchmarkTypeEnum.unknown;
    }
  }

  static double getSummaryMaxThroughput() =>
      _MAX_THROUGHPUT['SUMMARY_MAX_THROUGHPUT']!;
}

final _MAX_THROUGHPUT = {
  'IC_tpu_uint8': 508.0,
  'IC_tpu_float32': 508.0,
  'IC_tpu_uint8_offline': 508.0,
  'IC_tpu_float32_offline': 508.0,
  'OD_uint8': 288.0,
  'OD_float32': 288.0,
  'LU_int8': 12.0,
  'LU_float32': 12.0,
  'LU_gpu_float32': 12.0,
  'LU_nnapi_int8': 12.0,
  'IS_int8': 50.0,
  'IS_uint8': 50.0,
  'IS_float32': 50.0,
  'SUMMARY_MAX_THROUGHPUT': 155.0,
};

final _BENCHMARK_ICONS = {
  'SingleStream': {
    'IC': Icons.image_classification,
    'OD': Icons.object_detection,
    'IS': Icons.image_segmentation,
    'LU': Icons.language_processing,
  },
  'Offline': {
    'IC': Icons.image_classification_offline,
  },
};

final _BENCHMARK_ICONS_WHITE = {
  'SingleStream': {
    'IC': Icons.image_classification_white,
    'OD': Icons.object_detection_white,
    'IS': Icons.image_segmentation_white,
    'LU': Icons.language_processing_white,
  },
  'Offline': {
    'IC': Icons.image_classification_offline_white,
  },
};
