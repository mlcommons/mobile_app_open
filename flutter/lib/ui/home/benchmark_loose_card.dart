import 'package:flutter/material.dart';

import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/app_styles.dart';
import 'package:mlperfbench/ui/home/backend_choice.dart';

/// A benchmark that belongs to no set, switched on or off on its own.
///
/// Free of [BenchmarkState] for the same reason as [BenchmarkSetCard]: every
/// mutation goes out through a callback, so it can be rendered in a test.
class BenchmarkLooseCard extends StatelessWidget {
  final Benchmark benchmark;
  final VoidCallback onInfoTap;
  final VoidCallback onDownloadTap;

  /// Resolves once resource validation for this benchmark completes.
  final Future<bool> resourcesExist;

  final void Function(bool isActive) onActiveChanged;
  final void Function(String libName) onBackendChanged;
  final void Function(String delegate) onDelegateChanged;

  const BenchmarkLooseCard({
    super.key,
    required this.benchmark,
    required this.onInfoTap,
    required this.onDownloadTap,
    required this.resourcesExist,
    required this.onActiveChanged,
    required this.onBackendChanged,
    required this.onDelegateChanged,
  });

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context)!;

    return Container(
      margin: const EdgeInsets.only(bottom: 12),
      decoration: BoxDecoration(
        color: Colors.white,
        border: Border.all(color: AppColors.cardBorder),
        borderRadius: BorderRadius.circular(14),
      ),
      clipBehavior: Clip.antiAlias,
      child: FutureBuilder<bool>(
        future: resourcesExist,
        initialData: true,
        builder: (context, snapshot) {
          return Padding(
            padding: const EdgeInsets.fromLTRB(14, 10, 6, 10),
            child: Row(
              children: [
                SizedBox(
                  width: 40,
                  height: 40,
                  child: TextButton(
                    onPressed: onInfoTap,
                    style: ElevatedButton.styleFrom(
                      backgroundColor: AppColors.setIconBackground,
                      padding: EdgeInsets.zero,
                      elevation: 0,
                      shape: RoundedRectangleBorder(
                        borderRadius: BorderRadius.circular(
                          WidgetSizes.borderRadius,
                        ),
                      ),
                    ),
                    child: SizedBox(
                      width: 28,
                      height: 28,
                      child: benchmark.info.icon,
                    ),
                  ),
                ),
                const SizedBox(width: 12),
                Expanded(
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      Text(
                        benchmark.info.taskName,
                        style: const TextStyle(
                          fontWeight: FontWeight.bold,
                          fontSize: 15,
                        ),
                        overflow: TextOverflow.ellipsis,
                      ),
                      const SizedBox(height: 4),
                      Row(
                        children: [
                          Flexible(
                            child: BackendChoice(
                              benchmark: benchmark,
                              onChanged: onBackendChanged,
                            ),
                          ),
                          if (hasDelegateChoice(benchmark)) ...[
                            const SizedBox(
                              height: 16,
                              child: VerticalDivider(color: Colors.black26),
                            ),
                            DelegateChoice(
                              benchmark: benchmark,
                              onChanged: onDelegateChanged,
                            ),
                          ],
                        ],
                      ),
                    ],
                  ),
                ),
                if (benchmark.isActive && !snapshot.data!)
                  InkWell(
                    onTap: onDownloadTap,
                    child: Tooltip(
                      message: l10n.mainScreenSetFilesMissing,
                      child: const Icon(
                        Icons.downloading_rounded,
                        size: 26,
                        color: AppColors.warningIcon,
                      ),
                    ),
                  ),
                Switch(
                  activeThumbColor: AppColors.primary,
                  value: benchmark.isActive,
                  onChanged: onActiveChanged,
                ),
              ],
            ),
          );
        },
      ),
    );
  }
}
