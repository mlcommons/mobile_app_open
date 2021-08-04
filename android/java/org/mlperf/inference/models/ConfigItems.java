package org.mlperf.inference.models;

import static org.mlperf.inference.activities.CalculatingActivity.middleInterface;

import android.content.Context;
import java.util.ArrayList;
import org.mlperf.proto.BackendSetting;
import org.mlperf.proto.BenchmarkSetting;

public class ConfigItems {

  @SuppressWarnings({"unused", "RedundantSuppression"})
  public final Context context;

  public final String key;
  public final ArrayList<ConfigItem> options;

  public ConfigItems(Context context, String key, ArrayList<ConfigItem> options) {
    this.context = context;
    this.key = key;
    this.options = options;
  }

  public String getSelectedId() {

    try {

      BackendSetting bs = middleInterface.getSettings();
      for (BenchmarkSetting bms : bs.getBenchmarkSettingList()) {
        if (bms.getBenchmarkId().equals(key)) {
          return bms.getConfiguration();
        }
      }
      return null;
    } catch (Exception e) {
      e.printStackTrace();
      return null;
    }
  }
}
