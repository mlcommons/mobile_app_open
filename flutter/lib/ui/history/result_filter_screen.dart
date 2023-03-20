import 'package:flutter/material.dart';

import 'package:intl/intl.dart';
import 'package:mlperfbench_common/data/environment/environment_info.dart';
import 'package:mlperfbench_common/data/result_filter.dart';
import 'package:provider/provider.dart';

import 'package:mlperfbench/app_constants.dart';
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
        appBar: helper.makeAppBar(l10n.historyFilterTitle,
            actions: [_clearFilterButton(state.resourceManager.resultManager)]),
        body: Padding(
          padding: const EdgeInsets.all(8.0),
          child: ListView(
            children: [
              const SizedBox(height: 18),
              _creationDateFilter(filter),
              const SizedBox(height: 12),
              _platformFilter(filter),
              const SizedBox(height: 12),
              _benchmarkIdFilter(filter),
              const SizedBox(height: 12),
              _backendNameFilter(filter),
              const SizedBox(height: 12),
              _deviceModelFilter(filter),
              const SizedBox(height: 12),
              _manufacturerFilter(filter),
              const SizedBox(height: 12),
              _socFilter(filter),
            ],
          ),
        ));
  }

  Widget _creationDateFilter(ResultFilter filter) {
    final dateFormat = DateFormat('yyyy-MM-dd');
    final controller = TextEditingController();
    final fromDate = filter.fromCreationDate;
    final toDate = filter.toCreationDate;
    if (fromDate != null && toDate != null) {
      final startDate = dateFormat.format(fromDate);
      final endDate = dateFormat.format(toDate);
      controller.text = '[$startDate] - [$endDate]';
    }
    return TextField(
      controller: controller,
      readOnly: true,
      decoration: const InputDecoration(
        border: OutlineInputBorder(),
        labelText: 'Creation Date',
      ),
      onTap: () async {
        DateTimeRange? picked = await showDateRangePicker(
          context: context,
          firstDate: DateTime(2022, 1, 1),
          lastDate: DateTime.now().add(const Duration(days: 1)),
        );
        if (picked != null) {
          filter.fromCreationDate = picked.start;
          filter.toCreationDate = picked.end;
          final startDate = dateFormat.format(picked.start);
          final endDate = dateFormat.format(picked.end);
          controller.text = '[$startDate] - [$endDate]';
        }
      },
    );
  }

  Widget _platformFilter(ResultFilter filter) {
    return _makeDropDownFilter(
        labelText: 'Platform',
        choices: EnvPlatform.values.map((e) => e.name).toList(),
        value: filter.platform,
        onChanged: (value) => setState(() {
              filter.platform = value!;
            }));
  }

  Widget _benchmarkIdFilter(ResultFilter filter) {
    return _makeDropDownFilter(
        labelText: 'Benchmark ID',
        choices: BenchmarkId.allIds,
        value: filter.benchmarkId,
        onChanged: (value) => setState(() {
              filter.benchmarkId = value!;
            }));
  }

  Widget _backendNameFilter(ResultFilter filter) {
    return _makeDropDownFilter(
        labelText: 'Backend ID',
        choices: BackendId.allIds,
        value: filter.backend,
        onChanged: (value) => setState(() {
              filter.backend = value!;
            }));
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
        });
      },
    );
  }

  Widget _makeDropDownFilter(
      {required String labelText,
      required String? value,
      required List<String> choices,
      required void Function(String?)? onChanged}) {
    final items = choices
        .map((e) => DropdownMenuItem<String>(
              value: e,
              child: Text(e),
            ))
        .toList();
    return Container(
      decoration: const ShapeDecoration(
        shape: RoundedRectangleBorder(
          side: BorderSide(
              color: Colors.grey, width: 1.0, style: BorderStyle.solid),
          borderRadius: BorderRadius.all(Radius.circular(5.0)),
        ),
      ),
      child: DropdownButtonFormField<String>(
          isExpanded: true,
          decoration: InputDecoration(
            filled: true,
            fillColor: Colors.white,
            labelText: labelText,
          ),
          value: value,
          items: items,
          onChanged: onChanged),
    );
  }
}
