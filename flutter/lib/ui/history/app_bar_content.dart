import 'package:flutter/widgets.dart';

class AppBarContent {
  final String title;
  final Widget? leading;
  final List<Widget>? trailing;
  final void Function() reset;

  AppBarContent({
    required this.title,
    required this.leading,
    required this.trailing,
    required this.reset,
  });
}
