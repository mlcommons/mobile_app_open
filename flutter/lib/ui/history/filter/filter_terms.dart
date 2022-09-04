import 'package:mlperfbench/localizations/app_localizations.dart';

class FilterTerms {
  final List<SelectionOption> osOptions;

  FilterTerms({
    required this.osOptions,
  });

  factory FilterTerms.create(AppLocalizations l10n) {
    final List<SelectionOption> osOptions = [
      SelectionOption(
        name: l10n.filterOsAndroid,
        key: 'android',
      ),
      SelectionOption(
        name: l10n.filterOsIos,
        key: 'ios',
      ),
      SelectionOption(
        name: l10n.filterOsWindows,
        key: 'windows',
      ),
    ];
    return FilterTerms(
      osOptions: osOptions,
    );
  }
}

class SelectionOption {
  bool value;
  final String name;
  final String key;

  SelectionOption({
    this.value = false,
    required this.name,
    required this.key,
  });
}
