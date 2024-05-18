import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/store.dart';
import 'package:mlperfbench/ui/app_styles.dart';
import 'package:mlperfbench/ui/error_dialog.dart';
import 'package:mlperfbench/ui/icons.dart' show AppIcons;
import 'package:mlperfbench/ui/page_constraints.dart';
import 'package:mlperfbench/ui/settings/task_config_section.dart';

class ResourceErrorScreen extends StatelessWidget {
  const ResourceErrorScreen({super.key});

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context)!;
    final state = context.watch<BenchmarkState>();
    final store = context.watch<Store>();

    final iconEdgeSize = MediaQuery.of(context).size.width * 0.66;

    return Scaffold(
      body: getSinglePageView(
        Column(
          crossAxisAlignment: CrossAxisAlignment.center,
          mainAxisAlignment: MainAxisAlignment.spaceEvenly,
          children: [
            Expanded(
              child: Align(
                alignment: Alignment.bottomCenter,
                child: SizedBox(
                  height: iconEdgeSize,
                  width: iconEdgeSize,
                  child: AppIcons.error,
                ),
              ),
            ),
            Expanded(
              child: Padding(
                padding: const EdgeInsets.fromLTRB(35, 0, 35, 0),
                child: Align(
                  alignment: Alignment.topCenter,
                  child: Column(
                    children: [
                      Text(
                        'Error: ${state.error}\n',
                        textAlign: TextAlign.center,
                        style: const TextStyle(
                          fontSize: 15,
                          color: AppColors.errorText,
                        ),
                      ),
                      Text(
                        l10n.resourceErrorMessage,
                        textAlign: TextAlign.center,
                        style: const TextStyle(
                          fontSize: 15,
                          color: AppColors.darkText,
                        ),
                      ),
                      Text(
                        '${l10n.resourceErrorCurrentConfig} ${state.configManager.configLocation}',
                        textAlign: TextAlign.center,
                        style: const TextStyle(
                          fontSize: 15,
                          color: AppColors.darkText,
                        ),
                      ),
                      TextButton(
                        onPressed: () {
                          state.clearCache();
                        },
                        child: Text(l10n.settingsClearCache),
                      ),
                      TextButton(
                        onPressed: () async {
                          final taskConfigs =
                              await state.configManager.getConfigs();
                          // Workaround for Dart linter bug. See https://github.com/dart-lang/linter/issues/4007
                          // ignore: use_build_context_synchronously
                          if (!context.mounted) return;
                          await Navigator.of(context).push(
                            MaterialPageRoute(
                              builder: (context) =>
                                  TaskConfigErrorScreen(configs: taskConfigs),
                            ),
                          );
                        },
                        child: Text(l10n.resourceErrorSelectTaskFile),
                      ),
                      TextButton(
                        onPressed: () async {
                          try {
                            await state.setTaskConfig(
                                name: store.chosenConfigurationName);
                            state.deferredLoadResources();
                          } catch (e, trace) {
                            print("can't change task config: $e");
                            print(trace);
                            await showErrorDialog(context,
                                ['${l10n.resourceErrorFail}: ${e.toString()}']);
                          }
                        },
                        child: Text(l10n.resourceErrorRereadConfig),
                      ),
                    ],
                  ),
                ),
              ),
            ),
          ],
        ),
      ),
    );
  }
}
