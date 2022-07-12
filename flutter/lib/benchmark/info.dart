import 'package:flutter/material.dart';

import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/protos/mlperf_task.pb.dart' as pb;
import 'package:mlperfbench/ui/icons.dart';

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
            detailsContent: stringResources.isMosaicInfoDescription);
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

  double get maxThroughput => modelConfig.maxThroughput;

  /// 'IC', 'OD', and so on.
  String get code => modelConfig.id.split('_').first;

  /// 'SingleStream' or 'Offline'.
  String get scenario => modelConfig.scenario;

  BenchmarkTypeEnum get type => _typeFromCode();

  Widget get icon => _benchmarkIcons[scenario]?[code] ?? AppIcons.logo;

  Widget get iconWhite =>
      _benchmarkIconsWhite[scenario]?[code] ?? AppIcons.logo;

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
}

final _benchmarkIcons = {
  'SingleStream': {
    'IC': AppIcons.imageClassification,
    'OD': AppIcons.objectDetection,
    'IS': AppIcons.imageSegmentation,
    'LU': AppIcons.languageProcessing,
  },
  'Offline': {
    'IC': AppIcons.imageClassificationOffline,
  },
};

final _benchmarkIconsWhite = {
  'SingleStream': {
    'IC': AppIcons.imageClassificationWhite,
    'OD': AppIcons.objectDetectionWhite,
    'IS': AppIcons.imageSegmentationWhite,
    'LU': AppIcons.languageProcessingWhite,
  },
  'Offline': {
    'IC': AppIcons.imageClassificationOfflineWhite,
  },
};
