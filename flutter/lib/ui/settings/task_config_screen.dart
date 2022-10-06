import 'dart:io';

import 'package:flutter/material.dart';

import 'package:file_picker/file_picker.dart';
import 'package:provider/provider.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/resources/config_manager.dart';
import 'package:mlperfbench/store.dart';
import 'package:mlperfbench/ui/error_dialog.dart';
import 'package:mlperfbench/ui/settings/data_folder_type.dart';

class _DataFolderSelectorHelper {
  final AppLocalizations l10n;
  final Store store;
  final DataFolderType selectedOption;

  _DataFolderSelectorHelper(BuildContext context)
      : l10n = AppLocalizations.of(context),
        store = context.watch<Store>(),
        selectedOption =
            parseDataFolderType(context.read<Store>().dataFolderType);

  Widget build() {
    final items = <Widget>[];

    final dataFolderTitle = ListTile(
      title: Padding(
        padding: const EdgeInsets.only(bottom: 5),
        child: Text(
          l10n.settingsTaskDataFolderTitle,
        ),
      ),
      subtitle: Text(l10n.settingsTaskDataFolderDesc),
    );
    items.add(dataFolderTitle);

    items.add(_makeDefaultOption());
    if (defaultDataFolder.isNotEmpty) {
      items.add(_makeAppFolderOption());
    }
    items.add(_makeCustomOption());

    return Column(
      children: items,
    );
  }

  void setValue(DataFolderType? value) {
    if (value == null) return;
    if (value == selectedOption) return;
    store.dataFolderType = value.name;
  }

  Widget _makeDefaultOption() {
    late final String defaultOptionSubtitle;
    if (defaultDataFolder.isEmpty) {
      defaultOptionSubtitle = l10n.settingsTaskDataFolderApp;
    } else {
      defaultOptionSubtitle = defaultDataFolder;
    }
    return ListTile(
      title: Text(l10n.settingsTaskDataFolderDefault),
      subtitle: Text(defaultOptionSubtitle),
      leading: Radio<DataFolderType>(
        value: DataFolderType.default_,
        groupValue: selectedOption,
        onChanged: setValue,
      ),
      onTap: () => setValue(DataFolderType.default_),
    );
  }

  Widget _makeAppFolderOption() {
    return ListTile(
      title: Text(l10n.settingsTaskDataFolderApp),
      leading: Radio<DataFolderType>(
        value: DataFolderType.appFolder,
        groupValue: selectedOption,
        onChanged: setValue,
      ),
      onTap: () => setValue(DataFolderType.appFolder),
    );
  }

  Widget _makeCustomOption() {
    final pickFolder = () async {
      final dir = await FilePicker.platform.getDirectoryPath(
        lockParentWindow: true,
      );
      if (dir != null) {
        store.customDataFolder = dir;
        setValue(DataFolderType.custom);
      }
    };
    final pathField = Row(
      children: [
        Expanded(
          child: Text(store.customDataFolder),
        ),
        ElevatedButton(
          child: const Icon(Icons.folder),
          onPressed: pickFolder,
        )
      ],
    );
    Widget dirWarning = const SizedBox.shrink();
    if (store.customDataFolder.isNotEmpty) {
      dirWarning = FutureBuilder(
        future: Directory(store.customDataFolder).exists(),
        builder: (context, AsyncSnapshot<bool> snapshot) {
          if (!snapshot.hasData) {
            return const SizedBox.shrink();
          }
          final dirExists = snapshot.data!;
          if (!dirExists) {
            return Text(
              'Folder ${store.customDataFolder} does not exist or is not accessible',
              style: const TextStyle(color: AppColors.darkRedText),
            );
          }
          return const SizedBox.shrink();
        },
      );
    }
    return ListTile(
      title: Text(l10n.settingsTaskDataFolderCustom),
      subtitle: WillPopScope(
        onWillPop: () async {
          if (store.customDataFolder.isEmpty ||
              !await Directory(store.customDataFolder).exists()) {
            setValue(DataFolderType.default_);
          }
          return true;
        },
        child: Column(
          children: [pathField, dirWarning],
        ),
      ),
      leading: Radio<DataFolderType>(
        value: DataFolderType.custom,
        groupValue: selectedOption,
        onChanged: setValue,
      ),
      onTap: () async {
        if (store.customDataFolder.isEmpty) {
          await pickFolder();
        } else {
          setValue(DataFolderType.custom);
        }
      },
    );
  }
}

class TaskConfigScreen extends StatelessWidget {
  final List<TaskConfigDescription> _configs;

  const TaskConfigScreen(this._configs, {Key? key}) : super(key: key);

  Card getOptionPattern(
    BuildContext context,
    TaskConfigDescription configuration,
    String chosenConfigName,
  ) {
    final stringResources = AppLocalizations.of(context);
    final state = context.watch<BenchmarkState>();
    final wasChosen = chosenConfigName == configuration.name;

    return Card(
      child: ListTile(
        selected: wasChosen,
        title: Padding(
          padding: const EdgeInsets.only(bottom: 5),
          child: Text(configuration.name),
        ),
        subtitle: Text(configuration.path),
        trailing: Text(configuration.getType(stringResources)),
        onTap: () async {
          try {
            await state.setTaskConfig(name: configuration.name);
            Navigator.of(context).popUntil((route) => route.isFirst);
            await state.loadResources();
          } catch (e) {
            await showErrorDialog(context, <String>[
              stringResources.settingsTaskConfigError,
              e.toString()
            ]);
          }
        },
      ),
    );
  }

  Widget _makeCacheFolderNotice(AppLocalizations l10n) {
    if (defaultCacheFolder.isEmpty) {
      return const SizedBox.shrink();
    }
    return Column(
      children: [
        ListTile(
          title: Padding(
            padding: const EdgeInsets.only(bottom: 5),
            child: Text(
              l10n.settingsTaskCacheFolderTitle,
            ),
          ),
          subtitle: Text(l10n.settingsTaskCacheFolderDesc),
        ),
        ListTile(
          enabled: false,
          title: Text(l10n.settingsTaskCacheFolderDefault),
          subtitle: const Text(defaultCacheFolder),
          leading: const Radio<bool>(
            value: true,
            groupValue: true,
            onChanged: null,
          ),
        )
      ],
    );
  }

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context);
    final store = context.watch<Store>();

    return Scaffold(
      appBar: AppBar(
        title: Text(l10n.settingsTaskConfigTitle),
      ),
      body: ListView(
        children: [
          _makeCacheFolderNotice(l10n),
          _DataFolderSelectorHelper(context).build(),
          const Divider(),
          ..._configs.map((c) => getOptionPattern(
                context,
                c,
                store.chosenConfigurationName,
              )),
        ],
      ),
    );
  }
}
