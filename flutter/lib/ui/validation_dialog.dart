import 'package:flutter/material.dart';

import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/app_styles.dart';

/// Non-dismissible validation progress dialog used while computing checksums.
///
/// Displays a spinner, a title and a short hint, and prevents dismissing via
/// back button or tapping outside.
class ValidationDialog extends StatelessWidget {
  const ValidationDialog({super.key});

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context)!;
    return PopScope(
      canPop: false,
      child: AlertDialog(
        backgroundColor: AppColors.dialogBackground,
        shape: RoundedRectangleBorder(
          borderRadius: BorderRadius.circular(12),
        ),
        content: Column(
          mainAxisSize: MainAxisSize.min,
          crossAxisAlignment: CrossAxisAlignment.center,
          children: [
            const SizedBox(height: 4),
            const CircularProgressIndicator(
              color: AppColors.primary,
              strokeWidth: 3,
            ),
            const SizedBox(height: 16),
            Text(
              l10n.dialogTitleChecksumValidation,
              style: const TextStyle(
                fontSize: 18,
                fontWeight: FontWeight.w600,
                color: AppColors.darkText,
              ),
              textAlign: TextAlign.center,
            ),
            const SizedBox(height: 8),
            Text(
              l10n.dialogContentChecksumValidation,
              style: TextStyle(
                fontSize: 14,
                color: AppColors.darkText.withOpacity(0.65),
              ),
              textAlign: TextAlign.center,
            ),
            const SizedBox(height: 4),
          ],
        ),
      ),
    );
  }
}
