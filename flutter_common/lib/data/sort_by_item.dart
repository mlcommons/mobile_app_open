enum SortByValues { dateAsc, dateDesc, taskThroughputDesc }

class SortByItem {
  String label;
  SortByValues value;

  SortByItem(this.label, this.value);
}
