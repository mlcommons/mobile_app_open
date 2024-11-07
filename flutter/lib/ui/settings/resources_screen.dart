import 'dart:io';

import 'package:flutter/material.dart';
import 'package:mlperfbench/benchmark/benchmark.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/benchmark/run_mode.dart';
import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/build_info.dart';
import 'package:mlperfbench/firebase/firebase_manager.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/resources/config_manager.dart';
import 'package:mlperfbench/store.dart';
import 'package:mlperfbench/ui/app_styles.dart';
import 'package:mlperfbench/ui/confirm_dialog.dart';
import 'package:mlperfbench/ui/settings/task_config_section.dart';

class ResourcesScreen extends StatefulWidget {
  const ResourcesScreen({super.key});

  @override
  State<ResourcesScreen> createState() => _ResourcesScreen();
}

class _ResourcesScreen extends State<ResourcesScreen> {
  late AppLocalizations l10n;
  late BenchmarkState state;
  late Store store;

  @override
  Widget build(BuildContext context) {
    store = context.watch<Store>();
    state = context.watch<BenchmarkState>();
    l10n = AppLocalizations.of(context)!;

    final loadingProgressText =
        'Loading progress: ${(state.loadingProgress * 100).round()}%';

    final children = <Widget>[];

    for (var benchmark in state.benchmarks) {
      children.add(_listTile(benchmark));
      children.add(const Divider(height: 20));
    }
    children.add(Text(loadingProgressText));
    children.add(const Divider(height: 20));
    children.add(_clearCacheButton());

    return Scaffold(
      appBar: AppBar(title: Text(l10n.menuResources)),
      body: SafeArea(
        child: Container(
          color: Colors.white,
          child: ListView(
            padding: const EdgeInsets.fromLTRB(0, 20, 0, 20),
            children: children,
          ),
        ),
      ),
    );
  }

  Widget _listTile(Benchmark benchmark) {
    final leadingWidth = 0.10 * MediaQuery.of(context).size.width;
    final subtitleWidth = 0.70 * MediaQuery.of(context).size.width;
    final trailingWidth = 0.20 * MediaQuery.of(context).size.width;
    return ListTile(
      leading: SizedBox(
          width: leadingWidth,
          height: leadingWidth,
          child: Padding(
            padding: const EdgeInsets.all(4),
            child: benchmark.info.icon,
          )),
      title: Text(benchmark.info.taskName),
      subtitle: SizedBox(
        width: subtitleWidth,
        child: const Text('Download progress...'),
      ),
      trailing: SizedBox(
        width: trailingWidth,
        child: _downloadButton(benchmark),
      ),
    );
  }

  Widget _downloadButton(Benchmark benchmark) {
    return ElevatedButton(
      onPressed: () {
        print('download files for ${benchmark.info.taskName}');
        print(benchmark.taskConfig.datasets.lite.inputPath);
        print(benchmark.taskConfig.datasets.lite.groundtruthPath);
      },
      child: FittedBox(
        fit: BoxFit.scaleDown,
        child: const Text('Download'),
      ),
    );
  }

  Widget _clearCacheButton() {
    return TextButton(
      style: TextButton.styleFrom(
        textStyle: const TextStyle(fontSize: 20),
      ),
      onPressed: () async {
        final dialogAction =
            await showConfirmDialog(context, l10n.settingsClearCacheConfirm);
        switch (dialogAction) {
          case ConfirmDialogAction.ok:
            await state.clearCache();
            BotToast.showSimpleNotification(
              title: l10n.settingsClearCacheFinished,
              hideCloseButton: true,
            );
            break;
          case ConfirmDialogAction.cancel:
            break;
          default:
            break;
        }
      },
      child: Text(l10n.settingsClearCache),
    );
  }
}
