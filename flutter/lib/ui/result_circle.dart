import 'dart:math';

import 'package:flutter/material.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/icons.dart' as app_icons;

class ResultCircle extends StatefulWidget {
  final num _value;

  ResultCircle(num value) : _value = value.clamp(0, 1);

  @override
  _ResultCircleState createState() => _ResultCircleState();
}

class _ResultCircleState extends State<ResultCircle>
    with SingleTickerProviderStateMixin {
  late final AnimationController _controller;

  Duration get _durationOfAnimation {
    final durationMS = (widget._value / 3000).round();
    return Duration(milliseconds: durationMS);
  }

  @override
  void initState() {
    _controller =
        AnimationController(duration: _durationOfAnimation, vsync: this);
    _controller.addListener(() => setState(() {}));
    _controller.forward();

    super.initState();
  }

  @override
  void didUpdateWidget(covariant ResultCircle oldWidget) {
    if (oldWidget._value != widget._value) {
      _controller
        ..reset()
        ..duration = _durationOfAnimation
        ..forward();
    }
    super.didUpdateWidget(oldWidget);
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final edgeSize = MediaQuery.of(context).size.width * 0.35;
    final value = widget._value * _controller.value;

    return Padding(
      padding: EdgeInsets.fromLTRB(0, 30, 0, 30),
      child: Container(
        margin: EdgeInsets.only(bottom: 20.0),
        width: edgeSize,
        height: edgeSize,
        decoration: BoxDecoration(
          shape: BoxShape.circle,
          gradient: LinearGradient(
            colors: AppColors.resultCircleGradient,
            begin: Alignment.topCenter,
            end: Alignment.bottomCenter,
          ),
          boxShadow: [
            BoxShadow(
              color: Colors.black12,
              offset: Offset(15, 15),
              blurRadius: 10,
            )
          ],
        ),
        child: Stack(
          alignment: AlignmentDirectional.center,
          children: <Widget>[
            Container(
                width: edgeSize,
                height: edgeSize,
                child: RotationTransition(
                    turns: AlwaysStoppedAnimation(value),
                    child: app_icons.AppIcons.perfomance_hand)),
            CustomPaint(
                size: Size(edgeSize, edgeSize), painter: ArcPaint(value))
          ],
        ),
      ),
    );
  }
}

class ArcPaint extends CustomPainter {
  final double _value;

  ArcPaint(this._value);

  @override
  void paint(Canvas canvas, Size size) {
    final angleEpsilon = pi / 25;
    final strokeWidth = 8.0;
    final startAngle = pi / 2;
    final sweepAngle = 2 * pi * _value;
    final useCenter = false;

    final rect = Rect.fromLTRB(-size.width / 8, -size.width / 8,
        9 / 8 * size.width, 9 / 8 * size.width);
    final paint = Paint()
      ..shader = SweepGradient(
          transform: GradientRotation(pi / 2 - angleEpsilon),
          endAngle: sweepAngle + angleEpsilon,
          colors: [Colors.white54, Colors.white]).createShader(rect)
      ..maskFilter = MaskFilter.blur(BlurStyle.solid, 10)
      ..style = PaintingStyle.stroke
      ..strokeWidth = strokeWidth
      ..strokeCap = StrokeCap.round;
    canvas.drawArc(rect, startAngle, sweepAngle, useCenter, paint);
  } // paint

  @override
  bool shouldRepaint(ArcPaint old) => old._value != _value;
}
