package org.mlperf.inference.models;

public class ConfigItem {
  public final String title;
  public final String description;
  public final String accelerator;
  public final String id;

  public ConfigItem(String title, String description, String id) {
    this.title = title;
    this.description = description;
    this.accelerator = null;
    this.id = id;
  }

  public ConfigItem(String title, String description, String accelerator, String id) {
    this.title = title;
    this.description = description;
    this.accelerator = accelerator;
    this.id = id;
  }

  public String getId() {
    return id;
  }
}
