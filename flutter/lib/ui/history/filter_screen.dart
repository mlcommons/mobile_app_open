import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/localizations/app_localizations.dart';
import 'utils.dart';

class FilterScreen extends StatefulWidget {
  const FilterScreen({Key? key}) : super(key: key);

  @override
  State<StatefulWidget> createState() {
    return _FilterScreen();
  }
}

class FilterTerms {
  OsSelector osSelector = OsSelector();
}

class OsSelector {
  bool windows = false;
  bool android = false;
  bool ios = false;
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
      appBar: helper.makeAppBar('Filter results'),
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
    final selector = filterTerms.osSelector;
    return Column(
      children: [
        helper.makeHeader('Operating system'),
        _makeSelectionItem(
          text: 'Windows',
          get: () => selector.windows,
          set: (value) => selector.windows = value,
        ),
        _makeSelectionItem(
          text: 'Android',
          get: () => selector.android,
          set: (value) => selector.android = value,
        ),
        _makeSelectionItem(
          text: 'iOS',
          get: () => selector.ios,
          set: (value) => selector.ios = value,
        ),
      ],
    );
  }
}
