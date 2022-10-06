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
    final options = <Widget>[];
    options.add(_makeDefaultOption());
    if (defaultDataFolder.isNotEmpty) {
      options.add(_makeAppFolderOption());
    }
    options.add(_makeCustomOption());

    return Column(
      children: options,
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
    final textController = TextEditingController(text: store.customDataFolder);

    final pathField = Row(
      children: [
        Expanded(
          child: TextField(
            controller: textController,
          ),
        ),
        ElevatedButton(
          child: const Icon(Icons.folder),
          onPressed: () async {
            final dir = await FilePicker.platform.getDirectoryPath(
              lockParentWindow: true,
            );
            if (dir != null) {
              store.customDataFolder = dir;
              setValue(DataFolderType.custom);
            }
          },
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
    final focusHandler = (bool focused) async {
      if (focused) {
        if (selectedOption != DataFolderType.custom) {
          setValue(DataFolderType.custom);
        }
      } else {
        store.customDataFolder = textController.text;
        if (textController.text.isEmpty) {
          setValue(DataFolderType.default_);
        }
      }
    };
    return ListTile(
      title: Text(l10n.settingsTaskDataFolderCustom),
      subtitle: Focus(
        onFocusChange: focusHandler,
        child: Column(
          children: [pathField, dirWarning],
        ),
      ),
      leading: Radio<DataFolderType>(
        value: DataFolderType.custom,
        groupValue: selectedOption,
        onChanged: setValue,
      ),
      onTap: () => setValue(DataFolderType.custom),
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

    final dataFolderTitle = ListTile(
      title: Padding(
        padding: const EdgeInsets.only(bottom: 5),
        child: Text(
          l10n.settingsTaskDataFolderTitle,
        ),
      ),
      subtitle: Text(l10n.settingsTaskDataFolderDesc),
    );
    final optionsList = ListView.builder(
      itemCount: _configs.length,
      itemBuilder: (context, index) {
        final configuration = _configs[index];
        return getOptionPattern(
          context,
          configuration,
          store.chosenConfigurationName,
        );
      },
    );

    return Scaffold(
      appBar: AppBar(
        title: Text(l10n.settingsTaskConfigTitle),
      ),
      body: Column(
        children: [
          _makeCacheFolderNotice(l10n),
          dataFolderTitle,
          _DataFolderSelectorHelper(context).build(),
          const Divider(),
          Expanded(child: optionsList)
        ],
      ),
    );
  }
}
