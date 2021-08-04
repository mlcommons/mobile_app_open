package org.mlperf.inference;

import static org.mlperf.inference.activities.CalculatingActivity.middleInterface;

import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import org.mlperf.proto.BackendSetting;
import org.mlperf.proto.BenchmarkSetting;

public class ConfigPrefsUtil {
  public final Context context;
  public final SharedPreferences preferences;

  static final String DEFAULT_CONFIG_VALUE = "optimized";

  public ConfigPrefsUtil(Context context) {
    this.context = context;
    //noinspection deprecation
    preferences = PreferenceManager.getDefaultSharedPreferences(context);
  }

  public String getValue(String key) {
    return preferences.getString(key, DEFAULT_CONFIG_VALUE);
  }

  public void setValue(String key, String value) {
    preferences.edit().putString(key, value).apply();
  }

  public String getReadableValue(String key) {
    try {
      BackendSetting bs = middleInterface.getSettings();
      for (BenchmarkSetting bms : bs.getBenchmarkSettingList()) {
        if (bms.getBenchmarkId().equals(key)) {
          return bms.getConfiguration();
        }
      }
    } catch (Exception e) {
      e.printStackTrace();
      return null;
    }
    return null;
  }
}
