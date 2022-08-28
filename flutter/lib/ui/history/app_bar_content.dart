import 'package:flutter/widgets.dart';

class AppBarContent {
  final Widget? leading;
  final List<Widget>? trailing;
  final void Function() reset;

  AppBarContent({
    required this.leading,
    required this.trailing,
    required this.reset,
  });
}
