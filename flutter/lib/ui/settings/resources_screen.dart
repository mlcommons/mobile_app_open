import 'package:flutter/material.dart';

import 'package:bot_toast/bot_toast.dart';
import 'package:provider/provider.dart';

import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/benchmark/run_mode.dart';
import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/store.dart';
import 'package:mlperfbench/ui/app_styles.dart';
import 'package:mlperfbench/ui/confirm_dialog.dart';
import 'package:mlperfbench/ui/error_dialog.dart';
import 'package:mlperfbench/ui/nil.dart';

class ResourcesScreen extends StatefulWidget {
  final bool autoStart;

  const ResourcesScreen({this.autoStart = false, super.key});

  @override
  State<ResourcesScreen> createState() => _ResourcesScreen();
}

class _ResourcesScreen extends State<ResourcesScreen> {
  late AppLocalizations l10n;
  late BenchmarkState state;
  late Store store;

  bool get downloading =>
      (state.loadingProgress > 0.0 && state.loadingProgress < 0.999);

  @override
  void initState() {
    if (widget.autoStart) {
      WidgetsBinding.instance.addPostFrameCallback((_) async {
        await state.loadResources(
          downloadMissing: true,
          benchmarks: state.activeBenchmarks,
        );
        if (state.error != null) {
          if (!mounted) return;
          await showErrorDialog(context, <String>[state.error.toString()]);
          // Reset both the error and stacktrace for further operation
          state.error = null;
          state.stackTrace = null;
        }
      });

      super.initState();
    }
  }

  @override
  Widget build(BuildContext context) {
    store = context.watch<Store>();
    state = context.watch<BenchmarkState>();
    l10n = AppLocalizations.of(context)!;

    final children = <Widget>[
      _downloadButton(state.allBenchmarks, l10n.resourceDownloadAll),
      const SizedBox(height: 20),
      for (var benchmark in state.allBenchmarks) ...[
        _listTileBuilder(benchmark),
        const Divider(height: 20),
      ],
      _downloadProgress(),
      const SizedBox(height: 20),
      _clearCacheButton(),
    ];

    return Scaffold(
      appBar: AppBar(title: Text(l10n.menuResources)),
      body: SafeArea(
        child: Container(
          color: Colors.white,
          child: ListView(
            padding: const EdgeInsets.fromLTRB(10, 20, 10, 20),
            children: children,
          ),
        ),
      ),
    );
  }

  Widget _listTileBuilder(Benchmark benchmark) {
    final leadingWidth = 0.10 * MediaQuery.of(context).size.width;
    final subtitleWidth = 0.90 * MediaQuery.of(context).size.width;
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
        child: Column(
          mainAxisAlignment: MainAxisAlignment.start,
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            _downloadStatus(
                benchmark, store.selectedBenchmarkRunMode.performanceRunMode),
            _downloadStatus(
                benchmark, store.selectedBenchmarkRunMode.accuracyRunMode),
          ],
        ),
      ),
      trailing: _downloadButton([benchmark], l10n.resourceDownload),
    );
  }

  Widget _downloadStatus(Benchmark benchmark, BenchmarkRunMode mode) {
    return FutureBuilder<Map<bool, List<String>>>(
      future: state.validator.validateResourcesExist(benchmark, mode),
      builder: (BuildContext context,
          AsyncSnapshot<Map<bool, List<String>>> snapshot) {
        if (snapshot.hasData && snapshot.data != null) {
          const double size = 18;
          const downloadedIcon =
              Icon(Icons.download_done, size: size, color: Colors.green);
          const notDownloadedIcon =
              Icon(Icons.download_done, size: size, color: Colors.grey);
          final result = snapshot.data!;
          final missing = result[false] ?? [];
          final existed = result[true] ?? [];
          final downloaded = missing.isEmpty;
          return TextButton.icon(
            icon: downloaded ? downloadedIcon : notDownloadedIcon,
            label: Text(
              mode.readable,
              style: const TextStyle(color: AppColors.darkText),
            ),
            //iconAlignment: IconAlignment.start,
            style: TextButton.styleFrom(
                padding: EdgeInsets.zero,
                minimumSize: const Size(50, 30),
                tapTargetSize: MaterialTapTargetSize.shrinkWrap,
                alignment: Alignment.centerLeft),
            onPressed: () {
              showDialog(
                context: context,
                builder: (BuildContext context) {
                  return _ResourcesTable(
                    taskName: benchmark.info.taskName,
                    modeName: mode.readable,
                    missing: missing,
                    existed: existed,
                  );
                },
              );
            },
          );
        } else {
          return Text(l10n.resourceChecking);
        }
      },
    );
  }

  Widget _downloadProgress() {
    if (!downloading) {
      return nil;
    }
    return Column(
      mainAxisAlignment: MainAxisAlignment.start,
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Text(
          l10n.resourceDownloading,
          maxLines: 1,
          style: const TextStyle(fontSize: 12),
        ),
        Text(
          state.loadingPath,
          maxLines: 5,
          overflow: TextOverflow.ellipsis,
          style: const TextStyle(fontSize: 12),
        ),
        const SizedBox(height: 8),
        LinearProgressIndicator(
          value: state.loadingProgress,
          minHeight: 8,
        ),
      ],
    );
  }

  Widget _downloadButton(List<Benchmark> benchmarks, String title) {
    return TextButton(
      onPressed: downloading
          ? null
          : () async {
              await state.loadResources(
                downloadMissing: true,
                benchmarks: benchmarks,
              );
              if (state.error != null) {
                if (!mounted) return;
                await showErrorDialog(
                    context, <String>[state.error.toString()]);
                // Reset both the error and stacktrace for further operation
                state.error = null;
                state.stackTrace = null;
              }
            },
      style: _downloadButtonStyle,
      child: Text(title),
    );
  }

  Widget _clearCacheButton() {
    return TextButton(
      onPressed: downloading
          ? null
          : () async {
              final dialogAction = await showConfirmDialog(
                  context, l10n.settingsClearCacheConfirm);
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
      style: _clearButtonStyle,
      child: Text(l10n.resourceClearAll),
    );
  }
}

ButtonStyle _downloadButtonStyle = ButtonStyle(
  backgroundColor: ResourceButtonColor(),
  foregroundColor: const MaterialStatePropertyAll(Colors.white),
);

ButtonStyle _clearButtonStyle = ButtonStyle(
  backgroundColor: ResourceButtonColor(defaultColor: Colors.red),
  foregroundColor: const MaterialStatePropertyAll(Colors.white),
);

class ResourceButtonColor extends MaterialStateColor {
  ResourceButtonColor({this.defaultColor = Colors.blue})
      : super(defaultColor.value);

  final Color defaultColor;

  @override
  Color resolve(Set<MaterialState> states) {
    if (states.contains(MaterialState.disabled)) return Colors.grey;
    return defaultColor;
  }
}

class _ResourcesTable extends StatelessWidget {
  final String taskName;
  final String modeName;
  final List<String> missing;
  final List<String> existed;

  const _ResourcesTable({
    required this.taskName,
    required this.modeName,
    required this.missing,
    required this.existed,
  });

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Column(
          children: [
            Text(taskName),
            Text(modeName),
          ],
        ),
      ),
      body: SingleChildScrollView(
        scrollDirection: Axis.vertical,
        child: Column(
          children: [
            const SizedBox(height: 20),
            Table(
              columnWidths: const {
                0: FixedColumnWidth(40),
                1: FlexColumnWidth(),
              },
              border: TableBorder.all(color: Colors.grey),
              defaultVerticalAlignment: TableCellVerticalAlignment.top,
              children: [
                for (var path in missing) _row(path, false),
                for (var path in existed) _row(path, true),
              ],
            ),
          ],
        ),
      ),
    );
  }

  TableRow _row(String path, bool existed) {
    const downloadedIcon = Icon(Icons.download_done, color: Colors.green);
    const notDownloadedIcon = Icon(Icons.download_done, color: Colors.grey);
    return TableRow(
      children: [
        Padding(
          padding: const EdgeInsets.all(8.0),
          child: existed ? downloadedIcon : notDownloadedIcon,
        ),
        Padding(
          padding: const EdgeInsets.all(8.0),
          child: Text(path),
        ),
      ],
    );
  }
}
