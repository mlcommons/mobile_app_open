import 'dart:io';

import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/benchmark/run_mode.dart';
import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/build_info.dart';
import 'package:mlperfbench/firebase/firebase_manager.dart';
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
  late AppLocalizations l10n;
  late BenchmarkState state;
  late Store store;

  @override
  Widget build(BuildContext context) {
    store = context.watch<Store>();
    state = context.watch<BenchmarkState>();
    l10n = AppLocalizations.of(context);

    Widget artificialLoadSwitch = _artificialLoadSwitch();
    Widget crashlyticsSwitch = _crashlyticsSwitch();
    Widget runModeDropdown = _runModeDropdown();
    Widget offlineModeSwitch = _offlineModeSwitch();
    Widget keepLogSwitch = _keepLogSwitch();
    Widget cooldownSwitch = _cooldownSwitch();
    Widget cooldownSlider = _cooldownSlider();
    Widget clearCacheButton = _clearCacheButton(context);
    Widget versionText = _versionText();
    Widget taskConfig = _taskConfig(context);

    return Scaffold(
        appBar: AppBar(title: Text(l10n.menuSettings)),
        body: SafeArea(
            child: ListView(
          padding: const EdgeInsets.only(top: 20),
          children: [
            runModeDropdown,
            offlineModeSwitch,
            keepLogSwitch,
            artificialLoadSwitch,
            cooldownSwitch,
            cooldownSlider,
            const Divider(),
            crashlyticsSwitch,
            const Divider(),
            taskConfig,
            const Divider(),
            clearCacheButton,
            const Divider(),
            versionText
          ],
        )));
  }

  Widget _taskConfig(BuildContext context) {
    return ListTile(
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
          _showUnableSpecifyConfigurationMessage(context);
        }
      },
    );
  }

  Widget _versionText() {
    final buildInfo = BuildInfoHelper.info;
    return Text(
      'Version: ${buildInfo.version} | Build: ${buildInfo.buildNumber}',
      textAlign: TextAlign.center,
    );
  }

  Widget _clearCacheButton(BuildContext context) {
    return TextButton(
      style: TextButton.styleFrom(
        textStyle: const TextStyle(fontSize: 20),
      ),
      onPressed: () async {
        switch (
            await showConfirmDialog(context, l10n.settingsClearCacheConfirm)) {
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
    );
  }

  Widget _cooldownSlider() {
    return Slider(
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
    );
  }

  Widget _cooldownSwitch() {
    return ListTile(
      title: Padding(
        padding: const EdgeInsets.only(bottom: 5),
        child: Text(l10n.settingsCooldown),
      ),
      subtitle: Text(l10n.settingsCooldownSubtitle
          .replaceAll('<cooldownPause>', store.cooldownDuration.toString())),
      trailing: Switch(
          value: store.cooldown,
          onChanged: (flag) {
            store.cooldown = flag;
          }),
    );
  }

  Widget _keepLogSwitch() {
    return ListTile(
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
    );
  }

  Widget _offlineModeSwitch() {
    return ListTile(
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
    );
  }

  Widget _runModeDropdown() {
    return ListTile(
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
    );
  }

  Widget _artificialLoadSwitch() {
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
    return artificialLoadSwitch;
  }

  Widget _crashlyticsSwitch() {
    Widget crashlyticsSwitch;
    if (FirebaseManager.enabled && DartDefine.firebaseCrashlyticsEnabled) {
      crashlyticsSwitch = ListTile(
        title: Padding(
          padding: const EdgeInsets.only(bottom: 5),
          child: Text(l10n.settingsCrashlyticsTitle),
        ),
        subtitle: Text(l10n.settingsCrashlyticsSubtitle),
        trailing: Switch(
          value: store.crashlyticsEnabled,
          onChanged: (enabled) {
            store.crashlyticsEnabled = enabled;
            FirebaseManager.instance.configureCrashlytics(enabled);
          },
        ),
      );
    } else {
      crashlyticsSwitch = const SizedBox(width: 0, height: 0);
    }
    return crashlyticsSwitch;
  }

  void _showUnableSpecifyConfigurationMessage(BuildContext context) {
    if (!_isSnackBarShowing) {
      final snackBar = getSnackBar(l10n.settingsUnableSpecifyConfiguration);

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
}
