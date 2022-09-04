import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/localizations/app_localizations.dart';
import '../utils.dart';
import 'filter_terms.dart';

class FilterScreen extends StatefulWidget {
  const FilterScreen({Key? key}) : super(key: key);

  @override
  State<StatefulWidget> createState() {
    return _FilterScreen();
  }
}

class _FilterScreen extends State<FilterScreen> {
  late AppLocalizations l10n;
  late HistoryHelperUtils helper;
  late FilterTerms filterTerms;

  @override
  void dispose() {
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    l10n = AppLocalizations.of(context);
    helper = HistoryHelperUtils(l10n);
    filterTerms = context.watch<FilterTerms>();

    return Scaffold(
      appBar: helper.makeAppBar(l10n.filterTitle),
      body: ListView(
        padding: const EdgeInsets.only(top: 20),
        children: [
          const Divider(),
          _makeOsSelector(),
          const Divider(),
        ],
      ),
    );
  }

  Widget _makeSelectionItem({
    required String text,
    required bool Function() get,
    required void Function(bool) set,
  }) {
    return ListTile(
      title: Padding(
        padding: const EdgeInsets.only(bottom: 5),
        child: Text(text),
      ),
      trailing: Checkbox(
        onChanged: (value) => set(value!),
        value: get(),
      ),
      onTap: () => setState(() => set(!get())),
    );
  }

  Widget _makeOsSelector() {
    return Column(
      children: [
        helper.makeHeader(l10n.filterOsTitle),
        ...filterTerms.osOptions
            .map((e) => _makeSelectionItem(
                  text: e.name,
                  get: () => e.value,
                  set: (value) => e.value = value,
                ))
            .toList(),
      ],
    );
  }
}
