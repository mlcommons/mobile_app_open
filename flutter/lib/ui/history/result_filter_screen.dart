import 'package:flutter/material.dart';

import 'package:collection/collection.dart';
import 'package:intl/intl.dart';
import 'package:mlperfbench_common/data/environment/environment_info.dart';
import 'package:mlperfbench_common/data/result_filter.dart';
import 'package:mlperfbench_common/data/result_sort.dart';
import 'package:provider/provider.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/resources/result_manager.dart';

class ResultFilterScreen extends StatefulWidget {
  const ResultFilterScreen({Key? key}) : super(key: key);

  @override
  State<StatefulWidget> createState() {
    return _ResultFilterScreenState();
  }
}

class _ResultFilterScreenState extends State<ResultFilterScreen> {
  late AppLocalizations l10n;

  @override
  Widget build(BuildContext context) {
    l10n = AppLocalizations.of(context);
    final state = context.watch<BenchmarkState>();
    final filter = state.resourceManager.resultManager.resultFilter;

    return Scaffold(
        appBar: AppBar(
          title: Text(l10n.historyFilterTitle),
          actions: [_clearFilterButton(state.resourceManager.resultManager)],
        ),
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
              const SizedBox(height: 12),
              _sortBy(filter),
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
      decoration: InputDecoration(
        border: const OutlineInputBorder(),
        labelText: l10n.historyFilterCreationDate,
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
        labelText: l10n.historyFilterPlatform,
        choices: EnvPlatform.values
            .map((e) => DropdownOption(e.name, e.name))
            .toList(),
        value: filter.platform != null
            ? DropdownOption(filter.platform, filter.platform ?? '')
            : null,
        onChanged: (option) => setState(() {
              filter.platform = option?.value as String;
            }));
  }

  Widget _benchmarkIdFilter(ResultFilter filter) {
    return _makeDropDownFilter(
        labelText: l10n.historyFilterBenchmarkID,
        choices: BenchmarkId.allIds
            .map((benchmarkId) => DropdownOption(benchmarkId, benchmarkId))
            .toList(),
        value: filter.benchmarkId != null
            ? DropdownOption(filter.benchmarkId, filter.benchmarkId ?? '')
            : null,
        onChanged: (option) => setState(() {
              filter.benchmarkId = option?.value as String;
            }));
  }

  Widget _backendNameFilter(ResultFilter filter) {
    return _makeDropDownFilter(
        labelText: l10n.historyFilterBackendID,
        choices: BackendId.allIds
            .map((backendId) => DropdownOption(backendId, backendId))
            .toList(),
        value: filter.backend != null
            ? DropdownOption(
                filter.backend, filter.backend ?? BackendId.allIds[0])
            : null,
        onChanged: (option) => setState(() {
              filter.backend = option?.value as String;
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
      decoration: InputDecoration(
        border: const OutlineInputBorder(),
        labelText: l10n.historyFilterDeviceModel,
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
      decoration: InputDecoration(
        border: const OutlineInputBorder(),
        labelText: l10n.historyFilterManufacturer,
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
      decoration: InputDecoration(
        border: const OutlineInputBorder(),
        labelText: l10n.historyFilterSoC,
      ),
      onSubmitted: (value) {
        filter.soc = value.isEmpty ? null : value;
      },
    );
  }

  Widget _sortBy(ResultFilter filter) {
    List<SortByItem> sortItems = SortBy.options(l10n);
    SortByItem? selectedSortItem = sortItems.firstWhereOrNull(
        (SortByItem sortItem) => sortItem.value == filter.sortBy);
    return _makeDropDownFilter(
        labelText: l10n.historySortBy,
        choices: sortItems
            .map((sortItem) => DropdownOption(sortItem.value, sortItem.label))
            .toList(),
        value: selectedSortItem != null
            ? DropdownOption(selectedSortItem.value, selectedSortItem.label)
            : null,
        onChanged: (option) => setState(() {
              filter.sortBy = option?.value as SortByValues;
            }));
  }

  Widget _clearFilterButton(ResultManager resultManager) {
    return IconButton(
      icon: const Icon(Icons.clear),
      tooltip: l10n.historyFilterClear,
      onPressed: () {
        setState(() {
          resultManager.resultFilter = ResultFilter();
        });
      },
    );
  }

  Widget _makeDropDownFilter(
      {required String labelText,
      required DropdownOption? value,
      required List<DropdownOption> choices,
      required void Function(DropdownOption?)? onChanged}) {
    final items = choices
        .map((e) => DropdownMenuItem<DropdownOption>(
              value: e,
              child: Text(e.label),
            ))
        .toList();
    return Container(
      decoration: const ShapeDecoration(
        shape: RoundedRectangleBorder(
          side: BorderSide(
              color: Colors.grey, width: 1.0, style: BorderStyle.solid),
          borderRadius: BorderRadius.all(Radius.circular(3.0)),
        ),
      ),
      child: DropdownButtonFormField<DropdownOption>(
          isExpanded: true,
          decoration: InputDecoration(
            enabledBorder: InputBorder.none,
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

class DropdownOption<T> {
  final T value;
  final String label;

  DropdownOption(this.value, this.label);

  @override
  bool operator ==(dynamic other) =>
      other != null && other is DropdownOption && this.value == other.value;

  @override
  int get hashCode => value.hashCode ^ label.hashCode;
}
