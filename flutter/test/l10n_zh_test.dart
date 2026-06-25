// Verifies Simplified Chinese (zh) localization renders in a real MaterialApp,
// wired with the same delegates/supportedLocales the app uses.

import 'package:flutter/material.dart';

import 'package:flutter_test/flutter_test.dart';

import 'package:mlperfbench/localizations/app_localizations.dart';

Widget _wrap(Locale locale) {
  return MaterialApp(
    locale: locale,
    localizationsDelegates: AppLocalizations.localizationsDelegates,
    supportedLocales: AppLocalizations.supportedLocales,
    home: Builder(
      builder: (context) {
        final l10n = AppLocalizations.of(context)!;
        return Scaffold(
          body: Column(
            children: [
              Text(l10n.menuSettings),
              Text(l10n.menuHistory),
              Text(l10n.mainScreenGo),
              Text(l10n.dialogOk),
            ],
          ),
        );
      },
    ),
  );
}

void main() {
  testWidgets('zh is a supported locale', (tester) async {
    expect(AppLocalizations.supportedLocales, contains(const Locale('zh')));
  });

  testWidgets('renders Chinese text for Locale(zh)', (tester) async {
    await tester.pumpWidget(_wrap(const Locale('zh')));
    await tester.pumpAndSettle();

    expect(find.text('设置'), findsOneWidget); // Settings
    expect(find.text('历史记录'), findsOneWidget); // History
    expect(find.text('开始'), findsOneWidget); // GO
    expect(find.text('确定'), findsOneWidget); // Ok
    // Ensure the English strings are NOT shown.
    expect(find.text('Settings'), findsNothing);
  });

  testWidgets('device locale zh-CN resolves to Chinese', (tester) async {
    await tester.pumpWidget(_wrap(const Locale('zh', 'CN')));
    await tester.pumpAndSettle();
    expect(find.text('设置'), findsOneWidget);
  });

  testWidgets('placeholder template preserved in zh', (tester) async {
    final zh = lookupAppLocalizations(const Locale('zh'));
    expect(zh.mainScreenBenchmarkSelected, contains('<selected>'));
    expect(zh.mainScreenBenchmarkSelected, contains('<total>'));
    expect(zh.mainScreenBenchmarkSelected, contains('基准测试'));
  });
}
