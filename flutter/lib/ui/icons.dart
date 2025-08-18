import 'package:flutter/material.dart';

import 'package:flutter_svg/svg.dart';

import 'package:mlperfbench/app_constants.dart';

class AppIcons {
  static SvgPicture _pSvgBlack(String name) {
    const colorFilter = ColorFilter.mode(Colors.black, BlendMode.srcIn);
    return SvgPicture.asset('assets/icons/$name', colorFilter: colorFilter);
  }

  static SvgPicture _pSvg(String name) {
    return SvgPicture.asset('assets/icons/$name');
  }

  static final SvgPicture imageClassification =
      _pSvg('ic_task_image_classification.svg');
  static final SvgPicture imageSegmentation =
      _pSvg('ic_task_image_segmentation.svg');
  static final SvgPicture objectDetection =
      _pSvg('ic_task_object_detection.svg');
  static final SvgPicture languageProcessing =
      _pSvg('ic_task_language_processing.svg');
  static final SvgPicture imageClassificationOffline =
      _pSvg('ic_task_image_classification_offline.svg');
  static final SvgPicture superResolution =
      _pSvg('ic_task_super_resolution.svg');
  static final SvgPicture stableDiffusion =
      _pSvg('ic_task_stable_diffusion.svg');
  static final SvgPicture llm =
      _pSvg('ic_task_llm.svg');

  static final SvgPicture imageClassificationWhite =
      _pSvg('ic_task_image_classification_white.svg');
  static final SvgPicture imageSegmentationWhite =
      _pSvg('ic_task_image_segmentation_white.svg');
  static final SvgPicture objectDetectionWhite =
      _pSvg('ic_task_object_detection_white.svg');
  static final SvgPicture languageProcessingWhite =
      _pSvg('ic_task_language_processing_white.svg');
  static final SvgPicture imageClassificationOfflineWhite =
      _pSvg('ic_task_image_classification_offline_white.svg');
  static final SvgPicture superResolutionWhite =
      _pSvg('ic_task_super_resolution_white.svg');
  static final SvgPicture stableDiffusionWhite =
      _pSvg('ic_task_stable_diffusion_white.svg');

  static final SvgPicture arrow = _pSvg('ic_arrow.svg');

  static final SvgPicture logo = _pSvg('ic_logo.svg');

  static final SvgPicture error = _pSvgBlack('ic_error.svg');

  static final SvgPicture performanceHand = _pSvg('ic_performance_hand.svg');

  static final SvgPicture waiting = _pSvg('waiting_picture.svg');

  static DecorationImage splashBackground() {
    return const DecorationImage(
        image: AssetImage('assets/splash.png'), fit: BoxFit.fill);
  }
}

class BenchmarkIcons {
  static final darkSet = {
    BenchmarkId.imageClassificationV2: AppIcons.imageClassification,
    BenchmarkId.objectDetection: AppIcons.objectDetection,
    BenchmarkId.imageSegmentationV2: AppIcons.imageSegmentation,
    BenchmarkId.naturalLanguageProcessing: AppIcons.languageProcessing,
    BenchmarkId.superResolution: AppIcons.superResolution,
    BenchmarkId.stableDiffusion: AppIcons.stableDiffusion,
    BenchmarkId.imageClassificationOfflineV2:
        AppIcons.imageClassificationOffline,
    BenchmarkId.llm: AppIcons.llm,
  };

  static final lightSet = {
    BenchmarkId.imageClassificationV2: AppIcons.imageClassificationWhite,
    BenchmarkId.objectDetection: AppIcons.objectDetectionWhite,
    BenchmarkId.imageSegmentationV2: AppIcons.imageSegmentationWhite,
    BenchmarkId.naturalLanguageProcessing: AppIcons.languageProcessingWhite,
    BenchmarkId.superResolution: AppIcons.superResolutionWhite,
    BenchmarkId.stableDiffusion: AppIcons.stableDiffusionWhite,
    BenchmarkId.imageClassificationOfflineV2:
        AppIcons.imageClassificationOfflineWhite,
    BenchmarkId.llm: AppIcons.llm,
  };

  static Widget getDarkIcon(String benchmarkId) =>
      darkSet[benchmarkId] ?? AppIcons.logo;

  static Widget getLightIcon(String benchmarkId) =>
      lightSet[benchmarkId] ?? AppIcons.logo;
}
