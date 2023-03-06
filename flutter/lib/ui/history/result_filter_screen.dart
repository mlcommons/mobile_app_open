import 'package:flutter/material.dart';

import 'package:mlperfbench_common/data/environment/environment_info.dart';
import 'package:mlperfbench_common/data/result_filter.dart';
import 'package:provider/provider.dart';

import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/resources/result_manager.dart';
import 'package:mlperfbench/ui/history/utils.dart';

class ResultFilterScreen extends StatefulWidget {
  const ResultFilterScreen({Key? key}) : super(key: key);

  @override
  State<StatefulWidget> createState() {
    return _ResultFilterScreenState();
  }
}

class _ResultFilterScreenState extends State<ResultFilterScreen> {
  @override
  Widget build(BuildContext context) {
    final state = context.watch<BenchmarkState>();
    final l10n = AppLocalizations.of(context);
    final helper = HistoryHelperUtils(l10n);
    final filter = state.resourceManager.resultManager.resultFilter;

    return Scaffold(
        appBar: helper.makeAppBar('Filter',
            actions: [_clearFilterButton(state.resourceManager.resultManager)]),
        body: Padding(
          padding: const EdgeInsets.all(8.0),
          child: Column(
            mainAxisAlignment: MainAxisAlignment.start,
            children: [
              const SizedBox(height: 18),
              _platformFilter(filter),
              const SizedBox(height: 12),
              _deviceModelFilter(filter),
              const SizedBox(height: 12),
              _backendNameFilter(filter),
              const SizedBox(height: 12),
              _manufacturerFilter(filter),
              const SizedBox(height: 12),
              _socFilter(filter),
            ],
          ),
        ));
  }

  Widget _platformFilter(ResultFilter filter) {
    return Row(mainAxisAlignment: MainAxisAlignment.spaceBetween, children: [
      const Text('Platform'),
      ToggleButtons(
        direction: Axis.horizontal,
        borderRadius: const BorderRadius.all(Radius.circular(8)),
        constraints: const BoxConstraints(
          minHeight: 40.0,
          minWidth: 80.0,
        ),
        onPressed: (int index) {
          setState(() {
            switch (index) {
              case 0:
                filter.platform = EnvPlatform.android;
                break;
              case 1:
                filter.platform = EnvPlatform.ios;
                break;
              case 2:
                filter.platform = EnvPlatform.windows;
                break;
            }
          });
        },
        isSelected: <bool>[
          filter.platform == EnvPlatform.android,
          filter.platform == EnvPlatform.ios,
          filter.platform == EnvPlatform.windows,
        ],
        children: const <Widget>[Text('Android'), Text('iOS'), Text('Windows')],
      )
    ]);
  }

  Widget _backendNameFilter(ResultFilter filter) {
    TextEditingController controller = TextEditingController();
    if (filter.backendName != null) {
      controller.text = filter.backendName!;
    }
    return TextField(
      controller: controller,
      autocorrect: false,
      decoration: const InputDecoration(
        border: OutlineInputBorder(),
        labelText: 'Backend Name',
      ),
      onSubmitted: (value) {
        filter.backendName = value.isEmpty ? null : value;
      },
    );
  }

  Widget _deviceModelFilter(ResultFilter filter) {
    TextEditingController controller = TextEditingController();
    if (filter.deviceModel != null) {
      controller.text = filter.deviceModel!;
    }
    return TextField(
      controller: controller,
      autocorrect: false,
      decoration: const InputDecoration(
        border: OutlineInputBorder(),
        labelText: 'Device Model',
      ),
      onSubmitted: (value) {
        filter.deviceModel = value.isEmpty ? null : value;
      },
    );
  }

  Widget _manufacturerFilter(ResultFilter filter) {
    TextEditingController controller = TextEditingController();
    if (filter.manufacturer != null) {
      controller.text = filter.manufacturer!;
    }
    return TextField(
      controller: controller,
      autocorrect: false,
      decoration: const InputDecoration(
        border: OutlineInputBorder(),
        labelText: 'Manufacturer',
      ),
      onSubmitted: (value) {
        filter.manufacturer = value.isEmpty ? null : value;
      },
    );
  }

  Widget _socFilter(ResultFilter filter) {
    TextEditingController controller = TextEditingController();
    if (filter.soc != null) {
      controller.text = filter.soc!;
    }
    return TextField(
      controller: controller,
      autocorrect: false,
      decoration: const InputDecoration(
        border: OutlineInputBorder(),
        labelText: 'SoC',
      ),
      onSubmitted: (value) {
        filter.soc = value.isEmpty ? null : value;
      },
    );
  }

  Widget _clearFilterButton(ResultManager resultManager) {
    return IconButton(
      icon: const Icon(Icons.clear),
      tooltip: 'Clear',
      onPressed: () {
        setState(() {
          resultManager.resultFilter = ResultFilter();
          print('Clear: ${resultManager.resultFilter.toJson()}');
        });
      },
    );
  }
}
