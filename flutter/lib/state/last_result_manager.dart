import 'dart:convert';

import 'package:mlperfbench_common/data/extended_result.dart';

import 'package:mlperfbench/store.dart';

// This class is used to store, save, and access the result
// that can be shown on the main screen.
//
// This is different from just the last result in the history of results:
// You can manually delete the last result in history, but the main screen shouldn't change.
// If the last run failed for any reason, main screen should no longer show result data, even though history still contains results.
class LastResultManager {
  final Store _store;

  ExtendedResult? _value;
  ExtendedResult? get value => _value;
  set value(ExtendedResult? newValue) {
    _value = newValue;
    _save();
  }

  LastResultManager(this._store);

  // can throw exceptions
  void restore() {
    // reset value in case parsing throws an exception
    _value = null;
    if (_store.previousExtendedResult.isNotEmpty) {
      _value = ExtendedResult.fromJson(
          jsonDecode(_store.previousExtendedResult) as Map<String, dynamic>);
    }
  }

  void _save() {
    if (value == null) {
      _store.previousExtendedResult = '';
    } else {
      _store.previousExtendedResult =
          const JsonEncoder().convert(value!.toJson());
    }
    print('save exit');
  }
}
