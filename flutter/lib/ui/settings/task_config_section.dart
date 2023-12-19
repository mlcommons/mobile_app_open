import 'dart:io';

import 'package:flutter/material.dart';

import 'package:file_picker/file_picker.dart';
import 'package:permission_handler/permission_handler.dart';
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
    if (DartDefine.defaultDataFolder.isNotEmpty) {
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
    if (DartDefine.defaultDataFolder.isEmpty) {
      defaultOptionSubtitle = l10n.settingsTaskDataFolderApp;
    } else {
      defaultOptionSubtitle = DartDefine.defaultDataFolder;
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
          onPressed: pickFolder,
          child: const Icon(Icons.folder),
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
              l10n.settingsTaskDataFolderWarning
                  .replaceFirst('<path>', store.customDataFolder),
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

class TaskConfigSection extends StatelessWidget {
  final List<TaskConfigDescription> _configs;

  const TaskConfigSection(this._configs, {Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context);
    final store = context.watch<Store>();
    final state = context.watch<BenchmarkState>();
    List<Widget> items = [];
    items.add(ListTile(
      leading: Text(
        l10n.settingsTaskConfigTitle,
        style: Theme.of(context).textTheme.headlineSmall,
      ),
    ));
    items.addAll(
      _configs.map(
        (c) => _getConfigChoice(context, c, store.chosenConfigurationName),
      ),
    );
    // On ios you need to properly request access to a folder outside of the app folder
    // but file_picker plugin doesn't do it.
    // see the following link for details:
    // https://github.com/mlcommons/mobile_app_open/pull/562#discussion_r992167655
    if (!Platform.isIOS) {
      items.addAll([
        _makeCacheFolderNotice(l10n),
        _DataFolderSelectorHelper(context).build(),
        const Divider(),
        ListTile(
            title: Text(l10n.settingsTaskDataFolderSelected),
            subtitle: Text(state.resourceManager.getDataFolder())),
      ]);
    }

    // On Android we need MANAGE_EXTERNAL_STORAGE permission
    // see https://github.com/mlcommons/mobile_app_open/issues/702
    if (Platform.isAndroid) {
      items.add(const ManageFilePermissionWidget());
    }

    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: items,
    );
  }

  Widget _getConfigChoice(
    BuildContext context,
    TaskConfigDescription configuration,
    String chosenConfigName,
  ) {
    final l10n = AppLocalizations.of(context);
    final state = context.watch<BenchmarkState>();
    final isSelected = chosenConfigName == configuration.name;

    return ListTile(
      selected: isSelected,
      title: Padding(
        padding: const EdgeInsets.only(bottom: 5),
        child: Text(configuration.name),
      ),
      subtitle: Text(configuration.path),
      trailing: Text(configuration.getType(l10n)),
      onTap: () async {
        try {
          await state.setTaskConfig(name: configuration.name);
          // Workaround for Dart linter bug. See https://github.com/dart-lang/linter/issues/4007
          // ignore: use_build_context_synchronously
          if (!context.mounted) return;
          Navigator.of(context).popUntil((route) => route.isFirst);
          await state.loadResources();
        } catch (e) {
          await showErrorDialog(
            context,
            <String>[l10n.settingsTaskConfigError, e.toString()],
          );
        }
      },
    );
  }

  Widget _makeCacheFolderNotice(AppLocalizations l10n) {
    if (DartDefine.defaultCacheFolder.isEmpty) {
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
          subtitle: const Text(DartDefine.defaultCacheFolder),
          leading: const Radio<bool>(
            value: true,
            groupValue: true,
            onChanged: null,
          ),
        )
      ],
    );
  }
}

class ManageFilePermissionWidget extends StatefulWidget {
  const ManageFilePermissionWidget({super.key});

  @override
  State<ManageFilePermissionWidget> createState() =>
      _ManageFilePermissionWidgetState();
}

class _ManageFilePermissionWidgetState
    extends State<ManageFilePermissionWidget> {
  Future<bool> _permissionGranted = Permission.manageExternalStorage.isGranted;

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context);
    return FutureBuilder<bool>(
      future: _permissionGranted,
      builder: (BuildContext context, AsyncSnapshot<bool> snapshot) {
        final title = l10n.settingsTaskDataFolderPermissionRequired;
        var subtitle = l10n.settingsTaskDataFolderPermissionNotGranted;
        var iconColor = Colors.grey;
        if (snapshot.data == true) {
          subtitle = l10n.settingsTaskDataFolderPermissionGranted;
          iconColor = Colors.green;
        }
        return ListTile(
          title: Text(title),
          subtitle: Text(subtitle),
          trailing: Icon(color: iconColor, Icons.check_circle),
          onTap: () {
            setState(() {
              final status = Permission.manageExternalStorage.request();
              _permissionGranted = status.isGranted;
            });
          },
        );
      },
    );
  }
}

class TaskConfigErrorScreen extends StatelessWidget {
  final List<TaskConfigDescription> configs;

  const TaskConfigErrorScreen({required this.configs, super.key});

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context);
    return Scaffold(
      appBar: AppBar(
        title: Text(l10n.resourceErrorSelectTaskFile),
      ),
      body: TaskConfigSection(configs),
    );
  }
}
