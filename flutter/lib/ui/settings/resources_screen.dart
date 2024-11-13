import 'package:bot_toast/bot_toast.dart';
import 'package:flutter/material.dart';
import 'package:mlperfbench/benchmark/benchmark.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/benchmark/run_mode.dart';
import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/store.dart';
import 'package:mlperfbench/ui/confirm_dialog.dart';

class ResourcesScreen extends StatefulWidget {
  const ResourcesScreen({super.key});

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
  Widget build(BuildContext context) {
    store = context.watch<Store>();
    state = context.watch<BenchmarkState>();
    l10n = AppLocalizations.of(context)!;

    final children = <Widget>[];

    for (var benchmark in state.benchmarks) {
      children.add(_listTileBuilder(benchmark));
      children.add(const Divider(height: 20));
    }
    children.add(const SizedBox(height: 20));
    children.add(_downloadProgress());
    children.add(_downloadButton());
    children.add(const SizedBox(height: 20));
    children.add(_clearCacheButton());

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
            _downloadStatus(benchmark, BenchmarkRunMode.performance),
            _downloadStatus(benchmark, BenchmarkRunMode.accuracy),
          ],
        ),
      ),
    );
  }

  Widget _downloadStatus(Benchmark benchmark, BenchmarkRunMode mode) {
    return FutureBuilder<bool>(
      future: state.validator.validateResourcesExist(benchmark, mode),
      builder: (BuildContext context, AsyncSnapshot<bool> snapshot) {
        if (snapshot.hasData && snapshot.data != null) {
          final downloaded = snapshot.data!;
          const downloadedIcon =
              Icon(Icons.check_circle, size: 16, color: Colors.green);
          const notDownloadedIcon =
              Icon(Icons.check_circle_outline, size: 16, color: Colors.grey);
          return Row(
            children: [
              downloaded ? downloadedIcon : notDownloadedIcon,
              const SizedBox(width: 10),
              Text(mode.readable),
            ],
          );
        } else {
          return Text(l10n.resourceChecking);
        }
      },
    );
  }

  Widget _downloadProgress() {
    if (!downloading) {
      return const SizedBox(height: 0);
    }
    return SizedBox(
      height: 88,
      child: Column(
        mainAxisAlignment: MainAxisAlignment.start,
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text(
            state.loadingPath,
            maxLines: 5,
            overflow: TextOverflow.ellipsis,
            style: const TextStyle(
              fontSize: 12.0,
            ),
          ),
          const SizedBox(height: 8),
          LinearProgressIndicator(
            value: state.loadingProgress,
            minHeight: 8,
          ),
        ],
      ),
    );
  }

  Widget _downloadButton() {
    return AbsorbPointer(
      absorbing: downloading,
      child: ElevatedButton(
        onPressed: () {
          state.loadResources(downloadMissing: true);
        },
        style: ElevatedButton.styleFrom(
            backgroundColor: downloading ? Colors.grey : Colors.blue),
        child: FittedBox(
          fit: BoxFit.scaleDown,
          child: Text(l10n.resourceDownload),
        ),
      ),
    );
  }

  // TODO: add confirm dialog
  Widget _clearCacheButton() {
    return AbsorbPointer(
      absorbing: downloading,
      child: ElevatedButton(
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
        style: ElevatedButton.styleFrom(
            backgroundColor: downloading ? Colors.grey : Colors.red),
        child: FittedBox(
          fit: BoxFit.scaleDown,
          child: Text(l10n.resourceClear),
        ),
      ),
    );
  }
}
