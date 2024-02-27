import 'package:flutter/material.dart';

class ProgressCircle extends StatelessWidget {
  final double strokeWidth;
  final double size;

  const ProgressCircle({
    super.key,
    required this.size,
    required this.strokeWidth,
  });

  @override
  Widget build(BuildContext context) {
    return _BorderedCircle(
      strokeWidth: strokeWidth,
      radius: size / 2,
      color: Colors.white.withOpacity(0.88),
    );
  }
}

class _BorderedCircle extends StatelessWidget {
  final double strokeWidth;
  final double radius;
  final Color color;

  const _BorderedCircle({
    super.key,
    required this.strokeWidth,
    required this.radius,
    required this.color,
  });

  @override
  Widget build(BuildContext context) {
    return CustomPaint(
      painter: DrawCircle(strokeWidth, radius, color),
    );
  }
}

class DrawCircle extends CustomPainter {
  final double strokeWidth;
  final double radius;
  final Color color;

  DrawCircle(this.strokeWidth, this.radius, this.color);

  @override
  void paint(Canvas canvas, Size size) {
    final paint = Paint()
      ..color = color
      ..strokeWidth = strokeWidth
      ..style = PaintingStyle.stroke;
    canvas.drawCircle(const Offset(0.0, 0.0), radius, paint);
  }

  @override
  bool shouldRepaint(CustomPainter oldDelegate) {
    return false;
  }
}
