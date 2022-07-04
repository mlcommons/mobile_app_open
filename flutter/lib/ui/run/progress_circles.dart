import 'dart:async';
import 'dart:math';

import 'package:flutter/material.dart';

class ProgressCircles extends StatefulWidget {
  final Size _size;

  const ProgressCircles(this._size, {Key? key}) : super(key: key);

  @override
  _ProgressCirclesState createState() => _ProgressCirclesState();
}

class _ProgressCirclesState extends State<ProgressCircles> {
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
    _timer =
        Timer.periodic(const Duration(seconds: 1), (_) => setState(_updateState));
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
        size: widget._size,
        painter: _ProgressCirclesPaint(_startCircleNumber, _endCircleNumber));
  }
}

class _ProgressCirclesPaint extends CustomPainter {
  final int _startCircleNumber;
  final int _endCircleNumber;

  _ProgressCirclesPaint(this._startCircleNumber, this._endCircleNumber);

  final double circleRadius = 10;
  final Paint _paintLine = Paint()
    ..color = Colors.white
    ..style = PaintingStyle.fill;
  final double pi = 3.1415926535897932;

  @override
  void paint(Canvas canvas, Size size) {
    for (var i = _startCircleNumber; i < _endCircleNumber; i++) {
      final currentDegree =
          2 * pi * ((i) / _ProgressCirclesState.circlesCount) + pi / 2;
      final x = (size.width + size.height * cos(currentDegree)) / 2;
      final y = (size.height + size.width * sin(currentDegree)) / 2;
      canvas.drawCircle(Offset(x, y), circleRadius, _paintLine);
    }
  } // paint

  @override
  bool shouldRepaint(_ProgressCirclesPaint old) =>
      old._endCircleNumber != _endCircleNumber ||
      old._startCircleNumber != _startCircleNumber;
}
