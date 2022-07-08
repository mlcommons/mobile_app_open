import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/store.dart';
import 'package:mlperfbench/ui/error_dialog.dart';
import 'package:mlperfbench/ui/icons.dart' show AppIcons;
import 'package:mlperfbench/ui/page_constraints.dart';

class ResourceErrorScreen extends StatelessWidget {
  const ResourceErrorScreen({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    final stringResources = AppLocalizations.of(context);
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
                          color: AppColors.darkRedText,
                        ),
                      ),
                      Text(
                        stringResources.resourceErrorMessage,
                        textAlign: TextAlign.center,
                        style: const TextStyle(
                          fontSize: 15,
                          color: AppColors.darkText,
                        ),
                      ),
                      Text(
                        '${stringResources.resourceErrorCurrentConfig} ${state.configManager.configLocation}',
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
                        child: Text(stringResources.clearCache),
                      ),
                      TextButton(
                        onPressed: () async {
                          try {
                            await state.setTaskConfig(name: '');
                            state.deferredLoadResources();
                          } catch (e, trace) {
                            print("can't change task config: $e");
                            print(trace);
                          }
                        },
                        child:
                            Text(stringResources.resourceErrorSwitchToDefault),
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
                            await showErrorDialog(context, [
                              '${stringResources.resourceErrorFail}: ${e.toString()}'
                            ]);
                          }
                        },
                        child: Text(stringResources.resourceErrorRereadConfig),
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
