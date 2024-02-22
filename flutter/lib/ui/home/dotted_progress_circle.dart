import 'dart:async';
import 'dart:math';

import 'package:flutter/material.dart';

class DottedProgressCircle extends StatefulWidget {
  final double circleSize;
  final double dotSize;

  const DottedProgressCircle(
      {required this.circleSize, required this.dotSize, Key? key})
      : super(key: key);

  @override
  State<DottedProgressCircle> createState() => _DottedProgressCircleState();
}

class _DottedProgressCircleState extends State<DottedProgressCircle> {
  static const circlesCount = 10;

  late final Timer _timer;

  bool _increase = true;
  int _startCircleNumber = 0;
  int _endCircleNumber = 1;

  void _updateState() {
    if (_increase) {
      if (_startCircleNumber == circlesCount) {
        _startCircleNumber = 0;
        _endCircleNumber = 1;
      } else {
        _endCircleNumber++;
      }

      if (_endCircleNumber == circlesCount) {
        _increase = false;
      }
    } else {
      _startCircleNumber++;

      if (_startCircleNumber == circlesCount) {
        _increase = true;
      }
    }
  }

  @override
  void initState() {
    _timer = Timer.periodic(
      const Duration(seconds: 1),
      (_) => setState(_updateState),
    );
    super.initState();
  }

  @override
  void dispose() {
    _timer.cancel();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return CustomPaint(
        size: Size(widget.circleSize, widget.circleSize),
        painter: _ProgressCirclesPaint(
          _startCircleNumber,
          _endCircleNumber,
          widget.dotSize,
        ));
  }
}

class _ProgressCirclesPaint extends CustomPainter {
  final int _startCircleNumber;
  final int _endCircleNumber;
  final double _circleRadius;

  _ProgressCirclesPaint(
      this._startCircleNumber, this._endCircleNumber, this._circleRadius);

  final Paint _paintLine = Paint()
    ..color = Colors.white
    ..style = PaintingStyle.fill;
  final double pi = 3.1415926535897932;

  @override
  void paint(Canvas canvas, Size size) {
    for (var i = _startCircleNumber; i < _endCircleNumber; i++) {
      final currentDegree =
          2 * pi * ((i) / _DottedProgressCircleState.circlesCount) + pi / 2;
      final x = (size.width + size.height * cos(currentDegree)) / 2;
      final y = (size.height + size.width * sin(currentDegree)) / 2;
      canvas.drawCircle(Offset(x, y), _circleRadius, _paintLine);
    }
  } // paint

  @override
  bool shouldRepaint(_ProgressCirclesPaint old) =>
      old._endCircleNumber != _endCircleNumber ||
      old._startCircleNumber != _startCircleNumber;
}
