package org.mlperf.inference.activities.settings;

import android.content.Context;
import androidx.preference.Preference;
import org.mlperf.inference.ConfigPrefsUtil;

public class ConfigSummaryProvider implements Preference.SummaryProvider<Preference> {
  public final Context context;

  @SuppressWarnings({"unused", "RedundantSuppression"})
  public ConfigSummaryProvider(Context context) {
    this.context = context;
  }

  @SuppressWarnings({"unused", "RedundantSuppression"})
  @Override
  public CharSequence provideSummary(Preference preference) {
    return new ConfigPrefsUtil(context).getReadableValue(preference.getKey());
  }
}
