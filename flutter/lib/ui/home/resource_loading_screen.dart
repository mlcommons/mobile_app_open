import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/app_styles.dart';

class ResourceLoadingScreen extends StatefulWidget {
  const ResourceLoadingScreen({super.key});

  @override
  State<ResourceLoadingScreen> createState() => _ResourceLoadingScreenState();
}

class _ResourceLoadingScreenState extends State<ResourceLoadingScreen> {
  static const double progressCircleEdgeSize = 150;

  @override
  Widget build(BuildContext context) {
    final state = context.watch<BenchmarkState>();
    final l10n = AppLocalizations.of(context);
    final loadingProgressText = '${(state.loadingProgress * 100).round()}%';

    final progressCircle = Center(
      child: Container(
        width: progressCircleEdgeSize,
        height: progressCircleEdgeSize,
        decoration: const BoxDecoration(
          shape: BoxShape.circle,
          color: AppColors.progressCircle,
          boxShadow: [
            BoxShadow(
              color: Colors.black12,
              offset: Offset(15, 15),
              blurRadius: 10,
            )
          ],
        ),
        child: Center(
          child: Text(
            loadingProgressText,
            style: const TextStyle(
              fontWeight: FontWeight.bold,
              color: AppColors.lightText,
            ),
            textScaleFactor: 2,
          ),
        ),
      ),
    );

    final statusText = Padding(
      padding: const EdgeInsets.fromLTRB(20, 40, 20, 20),
      child: Column(
        mainAxisAlignment: MainAxisAlignment.center,
        crossAxisAlignment: CrossAxisAlignment.center,
        children: [
          Text(
            l10n.mainScreenLoading,
            style: const TextStyle(
              color: AppColors.lightText,
              fontSize: 16,
            ),
          ),
          const SizedBox(height: 20),
          Text(
            state.loadingPath,
            style: const TextStyle(
              fontWeight: FontWeight.w300,
              color: AppColors.lightText,
              fontSize: 12,
            ),
          ),
        ],
      ),
    );

    final backgroundGradient = BoxDecoration(
      gradient: LinearGradient(
        colors: AppGradients.fullScreen,
        begin: Alignment.topCenter,
        end: Alignment.bottomCenter,
      ),
    );

    return Scaffold(
      body: Container(
        decoration: backgroundGradient,
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.center,
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            progressCircle,
            statusText,
          ],
        ),
      ),
    );
  }
}
