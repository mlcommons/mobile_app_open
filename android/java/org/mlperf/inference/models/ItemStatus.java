package org.mlperf.inference.models;

public class ItemStatus {
  public final String title;
  public String tag;
  public Boolean value;
  public final int resIdSource;

  @SuppressWarnings({"unused", "RedundantSuppression"})
  public ItemStatus(String title, Boolean value, int source) {
    this.title = title;
    this.value = value;
    this.resIdSource = source;
  }

  public ItemStatus(String title, Boolean value, int source, String tag) {
    this.title = title;
    this.tag = tag;
    this.value = value;
    this.resIdSource = source;
  }
}
