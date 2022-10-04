import 'package:flutter/material.dart';

import 'package:provider/provider.dart';
import 'package:url_launcher/url_launcher.dart';

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
  _SettingsScreen createState() => _SettingsScreen();
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
    final stringResources = AppLocalizations.of(context);
    final buildInfo = BuildInfoHelper.info;

    return Scaffold(
        appBar: AppBar(
          title: Text(
            stringResources.settingsTitle,
          ),
        ),
        body: SafeArea(
            child: ListView(
          padding: const EdgeInsets.only(top: 20),
          children: [
            ListTile(
              title: Padding(
                padding: const EdgeInsets.only(bottom: 5),
                child: Text(
                  stringResources.settingsShare,
                ),
              ),
              subtitle: Text(stringResources.settingsShareSubtitle),
              trailing: Switch(
                  value: store.share,
                  onChanged: (flag) {
                    store.share = flag;
                  }),
            ),
            ListTile(
              title: Padding(
                padding: const EdgeInsets.only(bottom: 5),
                child: Text(
                  stringResources.settingsOffline,
                ),
              ),
              subtitle: Text(stringResources.settingsOfflineSubtitle),
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
                child: Text(
                  stringResources.settingsSubmission,
                ),
              ),
              subtitle: Text(stringResources.settingsSubmissionSubtitle),
              trailing: Switch(
                value: store.submissionMode,
                onChanged: (flag) {
                  store.submissionMode = flag;
                },
              ),
            ),
            ListTile(
              title: Padding(
                padding: const EdgeInsets.only(bottom: 5),
                child: Text(
                  stringResources.settingsKeepLogs,
                ),
              ),
              subtitle: Text(stringResources.settingsKeepLogsSubtitle),
              trailing: Switch(
                value: store.keepLogs,
                onChanged: (flag) {
                  store.keepLogs = flag;
                },
              ),
            ),
            ListTile(
              title: Padding(
                padding: const EdgeInsets.only(bottom: 5),
                child: Text(
                  stringResources.settingsCooldown,
                ),
              ),
              subtitle: Text(stringResources.settingsCooldownSubtitle
                  .replaceAll(
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
              title: Text(
                stringResources.settingsPrivacyPolicy,
              ),
              trailing: const Icon(Icons.chevron_right),
              onTap: () =>
                  launchUrl(Uri.parse('https://mlcommons.org/mobile_privacy')),
            ),
            ListTile(
              title: Text(stringResources.settingsEula),
              trailing: const Icon(Icons.chevron_right),
              onTap: () =>
                  launchUrl(Uri.parse('https://mlcommons.org/mobile_eula')),
            ),
            const Divider(),
            ListTile(
              title: Padding(
                padding: const EdgeInsets.only(bottom: 5),
                child: Text(
                  stringResources.settingsTaskConfigTitle,
                ),
              ),
              trailing: const Icon(Icons.chevron_right),
              onTap: () async {
                if (state.state == BenchmarkStateEnum.done ||
                    state.state == BenchmarkStateEnum.waiting) {
                  final taskConfigs = await state.configManager.getConfigs();

                  await Navigator.of(context).push(MaterialPageRoute(
                      builder: (context) => TaskConfigScreen(taskConfigs)));
                } else {
                  _showUnableSpecifyConfigurationMessage(
                      context, stringResources);
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
                    context, stringResources.settingsClearCacheConfirm)) {
                  case ConfirmDialogAction.ok:
                    await state.clearCache();
                    Navigator.pop(context);
                    break;
                  case ConfirmDialogAction.cancel:
                    break;
                  default:
                    break;
                }
              },
              child: Text(stringResources.settingsClearCache),
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
