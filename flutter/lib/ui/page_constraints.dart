import 'package:flutter/material.dart';

Widget getPageWidget(BoxConstraints constraint, Widget childWidget) {
  return Container(
    constraints: BoxConstraints(
        minHeight: constraint.maxHeight, minWidth: constraint.maxWidth),
    child: IntrinsicHeight(child: childWidget),
  );
}

Widget getSinglePageView(Widget child) {
  return LayoutBuilder(builder: (context, constraint) {
    return getPageWidget(constraint, child);
  });
}
