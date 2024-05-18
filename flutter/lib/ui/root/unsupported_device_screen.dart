import 'package:flutter/material.dart';

import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/icons.dart' show AppIcons;
import 'package:mlperfbench/ui/page_constraints.dart';

class UnsupportedDeviceScreen extends StatelessWidget {
  final String backendError;

  const UnsupportedDeviceScreen({super.key, required this.backendError});

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context);

    final iconEdgeSize = MediaQuery.of(context).size.width * 0.66;
    return Scaffold(
      body: getSinglePageView(
        Padding(
          padding: const EdgeInsets.only(top: 50),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              Align(
                alignment: Alignment.center,
                child: SizedBox(
                  height: iconEdgeSize,
                  width: iconEdgeSize,
                  child: AppIcons.error,
                ),
              ),
              const SizedBox(height: 16),
              Padding(
                padding: const EdgeInsets.symmetric(horizontal: 24),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.center,
                  children: [
                    Text(
                      l10n.unsupportedMainMessage,
                      textAlign: TextAlign.center,
                      style: Theme.of(context).textTheme.bodyLarge,
                    ),
                    const SizedBox(height: 12),
                    Text(
                      l10n.unsupportedTryAnotherDevice,
                      textAlign: TextAlign.center,
                      style: Theme.of(context).textTheme.bodyLarge,
                    ),
                    const Divider(height: 32),
                    Text(
                      l10n.unsupportedBackendError,
                      textAlign: TextAlign.center,
                      style: Theme.of(context).textTheme.titleMedium,
                    ),
                    const SizedBox(height: 16),
                    Text(
                      backendError,
                      textAlign: TextAlign.center,
                      style: Theme.of(context).textTheme.bodyLarge,
                    ),
                  ],
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }
}
