import 'package:mlperfbench_common/data/extended_result.dart';
import 'rest_helper.dart';

class FirebaseCacheHelper {
  final RestHelper restHelper;

  FirebaseCacheHelper(this.restHelper);

  // mapping uuid -> result
  final Map<String, ExtendedResult> _resultCache = {};
  // mapping `search uuid` -> `list of uuids`
  // Shold be reset when search terms change.
  // Should be reset for updating list of available results.
  final Map<String, List<String>> _batches = {};
  static const int pageSize = 20;

  void reset() {
    _batches.clear();
  }

  Future<void> fetchBatch({
    required String from,
    String osSelector = '',
  }) async {
    final orderedResults = <String>[];
    try {
      List<ExtendedResult> list;
      if (_batches[from] != null) {
        return;
      }
      list = await restHelper.fetchNext(
        pageSize: pageSize,
        uuidCursor: from,
        osSelector: osSelector,
      );
      for (var item in list) {
        orderedResults.add(item.meta.uuid);
        _resultCache[item.meta.uuid] = item;
      }
    } finally {
      // if an error happened while fetching data
      // we could fall into a loop of retries if we don't save empty batch
      _batches[from] = orderedResults;
    }
  }

  List<String>? getBatch({required String from}) {
    return _batches[from];
  }

  Future<void> fetchByUuid(String uuid) async {
    final result = await restHelper.fetchId(uuid: uuid);
    _resultCache[uuid] = result;
  }

  ExtendedResult? getByUuid(String uuid) {
    return _resultCache[uuid];
  }
}
