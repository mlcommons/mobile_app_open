import 'package:mlperfbench/data/extended_result.dart';

enum SortByEnum { dateAsc, dateDesc }

class ResultSort {
  SortByEnum sortBy = SortByEnum.dateDesc;

  ResultSort();

  List<ExtendedResult> apply(List<ExtendedResult> results) {
    List<ExtendedResult> items = List<ExtendedResult>.from(results);
    switch (sortBy) {
      case SortByEnum.dateAsc:
        items
            .sort((a, b) => a.meta.creationDate.compareTo(b.meta.creationDate));
        break;
      case SortByEnum.dateDesc:
        items
            .sort((a, b) => b.meta.creationDate.compareTo(a.meta.creationDate));
        break;
    }
    return items;
  }
}
