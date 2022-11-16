import 'package:flutter/material.dart';

import 'package:flutter_svg/avd.dart';
import 'package:flutter_svg/svg.dart';

class AppIcons {
  static AvdPicture _pAvd(String name) {
    return AvdPicture.asset('assets/icons/$name');
  }

  static AvdPicture _iconWhiteAvd(String name) {
    return AvdPicture.asset('assets/icons/$name', color: Colors.white);
  }

  static SvgPicture _pSvg(String name) {
    return SvgPicture.asset('assets/icons/$name');
  }

  static SvgPicture _iconColorSvg(String name, Color color) {
    return SvgPicture.asset('assets/icons/$name', color: color);
  }

  static final AvdPicture imageClassification =
      _pAvd('ic_image_classification.xml');
  static final SvgPicture imageSegmentation =
      _pSvg('ic_image_segmentation.svg');
  static final AvdPicture objectDetection = _pAvd('ic_object_detection.xml');
  static final AvdPicture languageProcessing =
      _pAvd('ic_language_processing.xml');
  static final SvgPicture imageClassificationOffline =
      _pSvg('ic_image_classification_offline.svg');
  static final SvgPicture superResolution = _pSvg('ic_super_resolution.svg');

  static final AvdPicture imageClassificationWhite =
      _iconWhiteAvd('ic_image_classification.xml');
  static final SvgPicture imageSegmentationWhite =
      _iconColorSvg('ic_image_segmentation.svg', Colors.white);
  static final AvdPicture objectDetectionWhite =
      _iconWhiteAvd('ic_object_detection.xml');
  static final AvdPicture languageProcessingWhite =
      _iconWhiteAvd('ic_language_processing.xml');
  static final SvgPicture imageClassificationOfflineWhite =
      _pSvg('ic_image_classification_offline_white.svg');
  static final SvgPicture superResolutionWhite =
      _iconColorSvg('ic_super_resolution.svg', Colors.white);

  static final AvdPicture arrow = _pAvd('ic_arrow.xml');
  static final AvdPicture parameters = _pAvd('ic_parameters.xml');
  static final AvdPicture settings = _pAvd('ic_settings.xml');

  static final AvdPicture logo = _pAvd('ic_logo.xml');

  static final SvgPicture error = _iconColorSvg('ic_error.svg', Colors.black);

  static final SvgPicture performanceHand = _pSvg('ic_performance_hand.svg');

  static final SvgPicture waiting = _pSvg('waiting_picture.svg');

  static DecorationImage splashBackground() {
    return const DecorationImage(
        image: AssetImage('assets/splash.png'), fit: BoxFit.fill);
  }
}
