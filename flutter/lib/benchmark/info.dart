import 'package:flutter/material.dart';

import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/protos/mlperf_task.pb.dart' as pb;
import 'package:mlperfbench/ui/icons.dart';

// enum BenchmarkTypeEnum {
//   unknown,
//   imageClassification,
//   objectDetection,
//   imageSegmentation,
//   languageUnderstanding,
// }

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
  final pb.TaskConfig task;

  /// 'Object Detection', 'Image Classification (offline)', and so on.
  String get taskName => task.name;

  BenchmarkInfo(this.task);

  // TODO remove
  String get name => task.model.name;

  BenchmarkLocalizationInfo getLocalizedInfo(AppLocalizations stringResources) {
    switch (task.id) {
      case ('image_classification'):
        return BenchmarkLocalizationInfo(
          name: stringResources.imageClassification,
          detailsTitle: stringResources.icInfo,
          detailsContent: stringResources.icInfoDescription,
        );
      case ('image_classification_offline'):
        return BenchmarkLocalizationInfo(
          name: stringResources.imageClassificationOffline,
          detailsTitle: stringResources.icInfo,
          detailsContent: stringResources.icInfoDescription,
        );
      case ('object_detection'):
        return BenchmarkLocalizationInfo(
          name: stringResources.objectDetection,
          detailsTitle: stringResources.odInfo,
          detailsContent: stringResources.odInfoDescription,
        );
      case ('image_segmentation_v2'):
        return BenchmarkLocalizationInfo(
          name: stringResources.imageSegmentation,
          detailsTitle: stringResources.isInfo,
          detailsContent: stringResources.isMosaicInfoDescription,
        );
      case ('natural_language_processing'):
        return BenchmarkLocalizationInfo(
          name: stringResources.languageProcessing,
          detailsTitle: stringResources.luInfo,
          detailsContent: stringResources.luInfoDescription,
        );
      default:
        throw 'unhandled task id: ${task.id}';
    }
  }

  bool get isOffline => task.scenario == 'Offline';

  double get maxThroughput => task.maxThroughput;

  /// 'SingleStream' or 'Offline'.
  String get scenario => task.scenario;

  Widget get icon => _benchmarkIcons[task.id] ?? AppIcons.logo;

  Widget get iconWhite => _benchmarkIconsWhite[task.id] ?? AppIcons.logo;

  @override
  String toString() => 'Benchmark:${task.id}';
}

final _benchmarkIcons = {
  'image_classification': AppIcons.imageClassification,
  'object_detection': AppIcons.objectDetection,
  'image_segmentation_v2': AppIcons.imageSegmentation,
  'natural_language_processing': AppIcons.languageProcessing,
  'image_classification_offline': AppIcons.imageClassificationOffline,
};

final _benchmarkIconsWhite = {
  'image_classification': AppIcons.imageClassificationWhite,
  'object_detection': AppIcons.objectDetectionWhite,
  'image_segmentation_v2': AppIcons.imageSegmentationWhite,
  'natural_language_processing': AppIcons.languageProcessingWhite,
  'image_classification_offline': AppIcons.imageClassificationOfflineWhite,
};
