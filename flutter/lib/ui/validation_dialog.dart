import 'package:flutter/material.dart';

/// Non-dismissible validation progress dialog used while computing checksums.
///
/// Displays a spinner, a title and a short hint, and prevents dismissing via
/// back button or tapping outside.
class ValidationDialog extends StatelessWidget {
  const ValidationDialog({super.key});

  @override
  Widget build(BuildContext context) {
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
            const Text(
              'Validating files...',
              style: TextStyle(
                fontSize: 16,
                fontWeight: FontWeight.w600,
              ),
              textAlign: TextAlign.center,
            ),
            const SizedBox(height: 8),
            Text(
              'This may take a minute depending on your device speed. Please keep the app open.',
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
