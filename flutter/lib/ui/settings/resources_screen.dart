import 'dart:io';

import 'package:flutter/material.dart';

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

    return Scaffold(
        appBar: AppBar(title: Text(l10n.menuResources)),
        body: SafeArea(
            child: ListView(
          padding: const EdgeInsets.only(top: 20),
          children: [
            _mainSection(),
            const Divider(),
            const SizedBox(height: 20)
          ],
        )));
  }

  Widget _mainSection() {
    return const Text('data');
  }
}
