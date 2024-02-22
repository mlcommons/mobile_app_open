import 'package:flutter/material.dart';

import 'package:mlperfbench/ui/home/gradient_circular_progress_indicator.dart';

class GradientProgressCircle extends StatefulWidget {
  final double size;
  final double strokeWidth;

  const GradientProgressCircle(
      {Key? key, required this.size, required this.strokeWidth})
      : super(key: key);

  @override
  State<GradientProgressCircle> createState() => _GradientProgressCircleState();
}

class _GradientProgressCircleState extends State<GradientProgressCircle>
    with SingleTickerProviderStateMixin {
  late AnimationController _controller;

  @override
  void initState() {
    _controller =
        AnimationController(vsync: this, duration: const Duration(seconds: 2));
    _controller.addListener(() => setState(() {}));
    _controller.repeat();
    super.initState();
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return RotationTransition(
      turns: Tween(begin: 0.0, end: 1.0).animate(_controller),
      child: GradientCircularProgressIndicator(
        progress: 1.0,
        // Specify the progress value between 0 and 1
        gradient: const LinearGradient(
          colors: [Colors.white, Colors.white, Colors.white10, Colors.white10],
          begin: Alignment.topLeft,
          end: Alignment.bottomRight,
        ),
        backgroundColor: Colors.transparent,
        stroke: widget.strokeWidth,
        size: widget.size,
      ),
    );
  }
}
