import 'package:flutter/material.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';

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
        shape: RoundedRectangleBorder(
          borderRadius: BorderRadius.circular(12),
        ),
        content: Column(
          mainAxisSize: MainAxisSize.min,
          crossAxisAlignment: CrossAxisAlignment.center,
          children: [
            const SizedBox(height: 4),
            const CircularProgressIndicator(),
            const SizedBox(height: 16),
            Text(
              l10n.dialogTitleChecksumValidation,
              style: const TextStyle(
                fontSize: 16,
                fontWeight: FontWeight.w600,
              ),
              textAlign: TextAlign.center,
            ),
            const SizedBox(height: 8),
            Text(
              l10n.dialogContentChecksumValidation,
              style: TextStyle(
                fontSize: 13,
                color: Colors.black.withOpacity(0.65),
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
