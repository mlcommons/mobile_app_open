// Unit tests for the Store.appLocale setting backing the language switcher.

import 'package:flutter_test/flutter_test.dart';
import 'package:shared_preferences/shared_preferences.dart';

import 'package:mlperfbench/store.dart';

void main() {
  TestWidgetsFlutterBinding.ensureInitialized();

  setUp(() {
    SharedPreferences.setMockInitialValues({});
  });

  test('appLocale defaults to empty string (follow system language)', () async {
    final store = await Store.create();
    expect(store.appLocale, '');
  });

  test('appLocale persists the chosen language code', () async {
    final store = await Store.create();

    store.appLocale = 'zh';
    expect(store.appLocale, 'zh');

    store.appLocale = 'en';
    expect(store.appLocale, 'en');

    store.appLocale = '';
    expect(store.appLocale, '');
  });

  test('appLocale notifies listeners when changed', () async {
    final store = await Store.create();
    var notifications = 0;
    store.addListener(() => notifications++);

    store.appLocale = 'zh';

    expect(notifications, 1);
  });
}
