import 'package:flutter/material.dart';

import 'package:flutter_svg/svg.dart';

class AppIcons {
  static SvgPicture _pSvgWhite(String name) {
    const colorFilter = ColorFilter.mode(Colors.white, BlendMode.srcIn);
    return SvgPicture.asset('assets/icons/$name', colorFilter: colorFilter);
  }

  static SvgPicture _pSvgBlack(String name) {
    const colorFilter = ColorFilter.mode(Colors.black, BlendMode.srcIn);
    return SvgPicture.asset('assets/icons/$name', colorFilter: colorFilter);
  }

  static SvgPicture _pSvg(String name) {
    return SvgPicture.asset('assets/icons/$name');
  }

  static final SvgPicture imageClassification =
      _pSvg('ic_image_classification.svg');
  static final SvgPicture imageSegmentation =
      _pSvg('ic_image_segmentation.svg');
  static final SvgPicture objectDetection = _pSvg('ic_object_detection.svg');
  static final SvgPicture languageProcessing =
      _pSvg('ic_language_processing.svg');
  static final SvgPicture imageClassificationOffline =
      _pSvg('ic_image_classification_offline.svg');
  static final SvgPicture superResolution = _pSvg('ic_super_resolution.svg');

  static final SvgPicture imageClassificationWhite =
      _pSvgWhite('ic_image_classification.svg');
  static final SvgPicture imageSegmentationWhite =
      _pSvgWhite('ic_image_segmentation.svg');
  static final SvgPicture objectDetectionWhite =
      _pSvgWhite('ic_object_detection.svg');
  static final SvgPicture languageProcessingWhite =
      _pSvgWhite('ic_language_processing.svg');
  static final SvgPicture imageClassificationOfflineWhite =
      _pSvg('ic_image_classification_offline_white.svg');
  static final SvgPicture superResolutionWhite =
      _pSvgWhite('ic_super_resolution.svg');

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
