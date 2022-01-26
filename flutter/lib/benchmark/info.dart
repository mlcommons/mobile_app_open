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

class BenchmarkInfoItem {
  final String title;
  final String details;

  BenchmarkInfoItem({required this.title, required String details})
      : details = _trim(details);

  BenchmarkInfoItem.stub(String name)
      : title = name,
        details = name;

  static BenchmarkInfoItem? getBenchmarkInfoItem(
      String benchmarkCode, AppLocalizations stringResources) {
    switch (benchmarkCode) {
      case ('IC'):
        return BenchmarkInfoItem(
          title: stringResources.icInfo,
          details: stringResources.icInfoDescription,
        );
      case ('OD'):
        return BenchmarkInfoItem(
          title: stringResources.odInfo,
          details: stringResources.odInfoDescription,
        );
      case ('IS'):
        return BenchmarkInfoItem(
            title: stringResources.isInfo,
            details: stringResources.isInfoDescription);
      case ('LU'):
        return BenchmarkInfoItem(
            title: stringResources.luInfo,
            details: stringResources.luInfoDescription);
    }
  }
}

class BenchmarkInfo {
  final pb.ModelConfig modelConfig;

  /// 'Object Detection', 'Image Classification (offline)', and so on.
  final String taskName;

  BenchmarkInfo(this.modelConfig, this.taskName);

  String get name => modelConfig.name;

  String getBenchmarkName(AppLocalizations stringResources) {
    switch (code) {
      case ('IC'):
        if (isOffline) {
          return stringResources.imageClassificationOffline;
        }
        return stringResources.imageClassification;
      case ('OD'):
        return stringResources.objectDetection;
      case ('IS'):
        return stringResources.imageSegmentation;
      case ('LU'):
        return stringResources.languageProcessing;
    }

    return modelConfig.id;
  }

  bool get isOffline => modelConfig.scenario == 'Offline';

  double get maxScore => _MAX_SCORE[modelConfig.id]!;

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

  static double getSummaryMaxScore() => _MAX_SCORE['SUMMARY_MAX_SCORE']!;
}

String _trim(String s) {
  return s
      .trim()
      .splitMapJoin(
        RegExp('\\n *'),
        onMatch: (_) => '\n',
      )
      .splitMapJoin(
        RegExp('\n\n?'),
        onMatch: (nl) => nl.end - nl.start == 1 ? ' ' : '\n',
      )
      .replaceAll('\n', '\n\n');
}

final _MAX_SCORE = {
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
  'SUMMARY_MAX_SCORE': 155.0,
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
