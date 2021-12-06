import 'package:flutter/material.dart' hide Icons;
import 'package:flutter/material.dart';
import 'package:mlcommons_ios_app/ui/confirm_dialog.dart';

import 'package:provider/provider.dart';
import 'package:url_launcher/url_launcher.dart';

import 'package:mlcommons_ios_app/benchmark/benchmark.dart';
import 'package:mlcommons_ios_app/localizations/app_localizations.dart';
import 'package:mlcommons_ios_app/store.dart';
import 'package:mlcommons_ios_app/ui/benchmarks_configuration_screen.dart';
import 'package:mlcommons_ios_app/ui/snack_bar.dart';

class SettingsScreen extends StatefulWidget {
  @override
  _SettingsScreen createState() => _SettingsScreen();
}

class _SettingsScreen extends State<SettingsScreen> {
  bool _isSnackBarShowing = false;

  void _showUnableSpecifyConfigurationMessage(
      BuildContext context, AppLocalizations stringResources) {
    if (!_isSnackBarShowing) {
      final snackBar = getSnackBar(stringResources.unableSpecifyConfiguration);

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

    return Scaffold(
      appBar: AppBar(
        title: Text(
          stringResources.settingsTitle,
        ),
      ),
      body: ListView(
        padding: const EdgeInsets.only(top: 20),
        children: [
          ListTile(
            title: Padding(
              padding: const EdgeInsets.only(bottom: 5),
              child: Text(
                stringResources.sharing,
              ),
            ),
            subtitle: Text(stringResources.sharingSubtitle),
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
                stringResources.submissionMode,
              ),
            ),
            subtitle: Text(stringResources.submissionModeSubtitle),
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
                stringResources.cooldown,
              ),
            ),
            subtitle: Text(stringResources.cooldownSubtitle
                .replaceAll('<cooldownPause>', store.cooldownPause.toString())),
            trailing: Switch(
                value: store.cooldown,
                onChanged: (flag) {
                  store.cooldown = flag;
                }),
          ),
          Slider(
            value: store.cooldownPause.toDouble(),
            min: 0,
            max: 10,
            divisions: 10,
            label: store.cooldownPause.toString(),
            onChanged: store.cooldown
                ? (double value) {
                    setState(() {
                      store.cooldownPause = value.toInt();
                    });
                  }
                : null,
          ),
          Divider(),
          ListTile(
            title: Text(
              stringResources.privacyPolicy,
            ),
            trailing: Icon(Icons.chevron_right),
            onTap: () => _launchURL('https://mlcommons.org/mobile_privacy'),
          ),
          ListTile(
            title: Text(stringResources.eula),
            trailing: Icon(Icons.chevron_right),
            onTap: () => _launchURL('https://mlcommons.org/mobile_eula'),
          ),
          Divider(),
          ListTile(
            title: Padding(
              padding: const EdgeInsets.only(bottom: 5),
              child: Text(
                stringResources.benchmarksConfiguration,
              ),
            ),
            trailing: Icon(Icons.chevron_right),
            onTap: () async {
              if (state.state == BenchmarkStateEnum.done ||
                  state.state == BenchmarkStateEnum.waiting) {
                final benchmarksConfigurations = await state.resourceManager
                    .getAvailableBenchmarksConfigurations();

                await Navigator.of(context).push(MaterialPageRoute(
                    builder: (context) => BenchmarksConfigurationScreen(
                        benchmarksConfigurations)));
              } else {
                _showUnableSpecifyConfigurationMessage(
                    context, stringResources);
              }
            },
          ),
          Divider(),
          TextButton(
            style: TextButton.styleFrom(
              textStyle: const TextStyle(fontSize: 20),
            ),
            onPressed: () async {
              switch (await showConfirmDialog(
                  context, stringResources.confirmClearCache)) {
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
            child: Text(stringResources.clearCache),
          ),
        ],
      ),
    );
  }
}

void _launchURL(String url) async {
  if (await canLaunch(url)) {
    await launch(url);
  } else {
    throw 'Could not launch $url';
  }
}
