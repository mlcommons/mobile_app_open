import 'dart:io';

import 'package:flutter/material.dart';

import 'package:provider/provider.dart';
import 'package:url_launcher/url_launcher.dart';

import 'package:mlperfbench/benchmark/run_mode.dart';
import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/build_info.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/store.dart';
import 'package:mlperfbench/ui/confirm_dialog.dart';
import 'package:mlperfbench/ui/settings/snack_bar.dart';
import 'package:mlperfbench/ui/settings/task_config_screen.dart';

class SettingsScreen extends StatefulWidget {
  const SettingsScreen({Key? key}) : super(key: key);

  @override
  State<SettingsScreen> createState() => _SettingsScreen();
}

class _SettingsScreen extends State<SettingsScreen> {
  bool _isSnackBarShowing = false;

  void _showUnableSpecifyConfigurationMessage(
      BuildContext context, AppLocalizations stringResources) {
    if (!_isSnackBarShowing) {
      final snackBar =
          getSnackBar(stringResources.settingsUnableSpecifyConfiguration);

      ScaffoldMessenger.of(context)
          .showSnackBar(snackBar)
          .closed
          .whenComplete(() => setState(() {
                _isSnackBarShowing = false;
              }));

      setState(() {
        _isSnackBarShowing = true;
      });
    }
  }

  @override
  Widget build(BuildContext context) {
    final store = context.watch<Store>();
    final state = context.watch<BenchmarkState>();
    final l10n = AppLocalizations.of(context);
    final buildInfo = BuildInfoHelper.info;

    Widget artificialLoadSwitch;
    if (Platform.isIOS) {
      artificialLoadSwitch = ListTile(
        title: Padding(
          padding: const EdgeInsets.only(bottom: 5),
          child: Text(l10n.settingsArtificialCPULoadTitle),
        ),
        subtitle: Text(l10n.settingsArtificialCPULoadSubtitle),
        trailing: Switch(
          value: store.artificialCPULoadEnabled,
          onChanged: (flag) {
            store.artificialCPULoadEnabled = flag;
          },
        ),
      );
    } else {
      artificialLoadSwitch = const SizedBox(width: 0, height: 0);
    }

    return Scaffold(
        appBar: AppBar(title: Text(l10n.menuSettings)),
        body: SafeArea(
            child: ListView(
          padding: const EdgeInsets.only(top: 20),
          children: [
            ListTile(
              title: Padding(
                padding: const EdgeInsets.only(bottom: 5),
                child: Text(l10n.settingsRunMode),
              ),
              subtitle: Text(l10n.settingsRunModeSubtitle),
              trailing: DropdownButton<BenchmarkRunModeEnum>(
                  value: store.selectedBenchmarkRunMode,
                  items: BenchmarkRunModeEnum.values
                      .map((runMode) => DropdownMenuItem<BenchmarkRunModeEnum>(
                            value: runMode,
                            child: Text(runMode.name),
                          ))
                      .toList(),
                  onChanged: (value) => setState(() {
                        store.selectedBenchmarkRunMode = value!;
                      })),
            ),
            ListTile(
              title: Padding(
                padding: const EdgeInsets.only(bottom: 5),
                child: Text(l10n.settingsOffline),
              ),
              subtitle: Text(l10n.settingsOfflineSubtitle),
              trailing: Switch(
                value: store.offlineMode,
                onChanged: (flag) {
                  store.offlineMode = flag;
                },
              ),
            ),
            ListTile(
              title: Padding(
                padding: const EdgeInsets.only(bottom: 5),
                child: Text(l10n.settingsKeepLogs),
              ),
              subtitle: Text(l10n.settingsKeepLogsSubtitle),
              trailing: Switch(
                value: store.keepLogs,
                onChanged: (flag) {
                  store.keepLogs = flag;
                },
              ),
            ),
            artificialLoadSwitch,
            ListTile(
              title: Padding(
                padding: const EdgeInsets.only(bottom: 5),
                child: Text(l10n.settingsCooldown),
              ),
              subtitle: Text(l10n.settingsCooldownSubtitle.replaceAll(
                  '<cooldownPause>', store.cooldownDuration.toString())),
              trailing: Switch(
                  value: store.cooldown,
                  onChanged: (flag) {
                    store.cooldown = flag;
                  }),
            ),
            Slider(
              value: store.cooldownDuration.toDouble(),
              min: 1,
              max: 10,
              divisions: 9,
              label: store.cooldownDuration.toString(),
              onChanged: store.cooldown
                  ? (double value) {
                      setState(() {
                        store.cooldownDuration = value.toInt();
                      });
                    }
                  : null,
            ),
            const Divider(),
            ListTile(
              title: Text(l10n.settingsPrivacyPolicy),
              trailing: const Icon(Icons.chevron_right),
              onTap: () =>
                  launchUrl(Uri.parse('https://mlcommons.org/mobile_privacy')),
            ),
            ListTile(
              title: Text(l10n.settingsEula),
              trailing: const Icon(Icons.chevron_right),
              onTap: () =>
                  launchUrl(Uri.parse('https://mlcommons.org/mobile_eula')),
            ),
            const Divider(),
            ListTile(
              title: Padding(
                padding: const EdgeInsets.only(bottom: 5),
                child: Text(l10n.settingsTaskConfigTitle),
              ),
              trailing: const Icon(Icons.chevron_right),
              onTap: () async {
                if (state.state == BenchmarkStateEnum.done ||
                    state.state == BenchmarkStateEnum.waiting) {
                  final taskConfigs = await state.configManager.getConfigs();

                  if (!mounted) return;
                  await Navigator.of(context).push(MaterialPageRoute(
                      builder: (context) => TaskConfigScreen(taskConfigs)));
                } else {
                  _showUnableSpecifyConfigurationMessage(context, l10n);
                }
              },
            ),
            const Divider(),
            TextButton(
              style: TextButton.styleFrom(
                textStyle: const TextStyle(fontSize: 20),
              ),
              onPressed: () async {
                switch (await showConfirmDialog(
                    context, l10n.settingsClearCacheConfirm)) {
                  case ConfirmDialogAction.ok:
                    await state.clearCache();
                    if (!mounted) return;
                    Navigator.pop(context);
                    break;
                  case ConfirmDialogAction.cancel:
                    break;
                  default:
                    break;
                }
              },
              child: Text(l10n.settingsClearCache),
            ),
            const Divider(),
            Text(
              'Version: ${buildInfo.version} | Build: ${buildInfo.buildNumber}',
              textAlign: TextAlign.center,
            )
          ],
        )));
  }
}
