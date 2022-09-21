import 'package:flutter/widgets.dart';

class AppBarContent {
  final List<Widget>? trailing;
  final void Function() reset;

  AppBarContent({
    required this.trailing,
    required this.reset,
  });
}
