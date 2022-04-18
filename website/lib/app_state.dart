import 'package:flutter/widgets.dart';

import 'package:mlperfbench_common/data/extended_result.dart';
import 'package:mlperfbench_common/firebase/manager.dart';

class AppState extends ChangeNotifier {
  static late final AppState instance;

  final Map<String, ExtendedResult> _resultCache = {};
  Map<String, List<String>> _batches = {};
  static const int pageSize = 20;

  final FirebaseManager _firebaseManager;

  AppState(this._firebaseManager);

  Future<void> fetchBatch({required String from}) async {
    List<ExtendedResult> list;
    if (from == '') {
      _batches = {};
      list = await _firebaseManager.restHelper.fetchFirst(pageSize: pageSize);
    } else {
      if (_batches[from] != null) {
        return;
      }
      list = await _firebaseManager.restHelper
          .fetchNext(pageSize: pageSize, uuidCursor: from);
    }
    final orderedResults = <String>[];
    for (var item in list) {
      orderedResults.add(item.meta.uuid);
      _resultCache[item.meta.uuid] = item;
    }
    _batches[from] = orderedResults;
  }

  List<String>? getBatch({required String from}) {
    return _batches[from];
  }

  Future<void> fetchByUuid(String uuid) async {
    final result = await _firebaseManager.restHelper.fetchId(uuid: uuid);
    _resultCache[uuid] = result;
  }

  ExtendedResult? getByUuid(String uuid) {
    return _resultCache[uuid];
  }
}
