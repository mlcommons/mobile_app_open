import 'package:flutter/material.dart';

import 'package:flutter_svg/avd.dart';
import 'package:flutter_svg/svg.dart';

class AppIcons {
  static AvdPicture _pAvd(String name) {
    return AvdPicture.asset('assets/icons/' + name);
  }

  static AvdPicture _iconWhiteAvd(String name) {
    return AvdPicture.asset('assets/icons/' + name, color: Colors.white);
  }

  static SvgPicture _pSvg(String name) {
    return SvgPicture.asset('assets/icons/' + name);
  }

  static SvgPicture _iconColorSvg(String name, Color color) {
    return SvgPicture.asset('assets/icons/' + name, color: color);
  }

  static final AvdPicture image_classification =
      _pAvd('ic_image_classification.xml');
  static final SvgPicture image_segmentation =
      _pSvg('ic_image_segmentation.svg');
  static final AvdPicture object_detection = _pAvd('ic_object_detection.xml');
  static final AvdPicture language_processing =
      _pAvd('ic_language_processing.xml');
  static final SvgPicture image_classification_offline =
      _pSvg('ic_image_classification_offline.svg');

  static final AvdPicture image_classification_white =
      _iconWhiteAvd('ic_image_classification.xml');
  static final SvgPicture image_segmentation_white =
      _iconColorSvg('ic_image_segmentation.svg', Colors.white);
  static final AvdPicture object_detection_white =
      _iconWhiteAvd('ic_object_detection.xml');
  static final AvdPicture language_processing_white =
      _iconWhiteAvd('ic_language_processing.xml');
  static final SvgPicture image_classification_offline_white =
      _pSvg('ic_image_classification_offline_white.svg');

  static final AvdPicture arrow = _pAvd('ic_arrow.xml');
  static final AvdPicture parameters = _pAvd('ic_parameters.xml');
  static final AvdPicture settings = _pAvd('ic_settings.xml');

  static final AvdPicture logo = _pAvd('ic_logo.xml');

  static final SvgPicture error = _iconColorSvg('ic_error.svg', Colors.black);

  static final SvgPicture perfomance_hand = _pSvg('ic_perfomance_hand.svg');

  static final SvgPicture waiting = _pSvg('waiting_picture.svg');

  static DecorationImage splash_background2(String name) {
    return const DecorationImage(
        image: AssetImage('assets/splash.png'), fit: BoxFit.fill);
  }

  static DecorationImage splash_background() {
    return const DecorationImage(
        image: AssetImage('assets/splash.png'), fit: BoxFit.fill);
  }
}
