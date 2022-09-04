import 'package:flutter/widgets.dart';

import 'package:mlperfbench/localizations/app_localizations.dart';

abstract class TabInterface {
  Widget build(BuildContext context);
  List<Widget>? getBarButtons(BuildContext context, AppLocalizations l10n);
  String getTabName(AppLocalizations l10n);
}
