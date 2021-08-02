package org.mlperf.inference;

import static org.mlperf.inference.activities.CalculatingActivity.middleInterface;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import androidx.preference.PreferenceManager;
import java.io.BufferedReader;
import java.io.FileReader;
import org.mlperf.proto.Setting;

public class Util {
  private static final String SHARE_REQUEST_SUCCEED = "SHARE_REQUEST_SUCCEED";
  private static final String ALLOW_SHARE_RESULTS = "ALLOW_SHARE_RESULTS";

  public static void setUserShareConsent(Context context, boolean value) {
    SharedPreferences.Editor prefsEdit =
        context.getSharedPreferences(context.getPackageName(), Context.MODE_PRIVATE).edit();
    prefsEdit.putBoolean(SHARE_REQUEST_SUCCEED, true);
    prefsEdit.apply();

    prefsEdit = PreferenceManager.getDefaultSharedPreferences(context).edit();
    prefsEdit.putBoolean(ALLOW_SHARE_RESULTS, value);
    prefsEdit.apply();

    try {
      Setting s = middleInterface.getCommonSetting("share_results");
      Setting.Value v = s.getAcceptableValue(value ? 0 : 1);
      s = s.toBuilder().setValue(v).build();
      middleInterface.setCommonSetting(s);
    } catch (Exception e) {
      e.printStackTrace();
    }
  }

  public static boolean getUserShareConsent() {
    try {
      Setting s = middleInterface.getCommonSetting("share_results");
      return s.getValue().getValue().equals("1");
    } catch (Exception e) {
      e.printStackTrace();
      return false;
    }
  }

  public static boolean hasUserConsented(Context context) {
    return context
        .getSharedPreferences(context.getPackageName(), Context.MODE_PRIVATE)
        .getBoolean(SHARE_REQUEST_SUCCEED, false);
  }

  @SuppressWarnings("SpellCheckingInspection")
  public static void setCooldown(boolean value) {
    try {
      Setting s = middleInterface.getCommonSetting("cooldown");
      Setting.Value v = s.getAcceptableValue(value ? 0 : 1);
      s = s.toBuilder().setValue(v).build();
      middleInterface.setCommonSetting(s);
    } catch (Exception e) {
      e.printStackTrace();
    }
  }

  @SuppressWarnings("SpellCheckingInspection")
  public static boolean getCooldown() {
    try {
      Setting s = middleInterface.getCommonSetting("cooldown");
      return s.getValue().getValue().equals("1");
    } catch (Exception e) {
      e.printStackTrace();
      return false;
    }
  }

  public static void setCooldownPause(Context context, int value) {
    SharedPreferences sharedPref = PreferenceManager.getDefaultSharedPreferences(context);
    SharedPreferences.Editor preferencesEditor = sharedPref.edit();

    preferencesEditor.putInt(AppConstants.COOLDOWN_PAUSE, value);
    preferencesEditor.apply();
  }

  public static int getCooldownPause(Context context) {
    SharedPreferences sharedPref = PreferenceManager.getDefaultSharedPreferences(context);

    return sharedPref.getInt(AppConstants.COOLDOWN_PAUSE, AppConstants.DEFAULT_COOLDOWN_PAUSE);
  }

  public static void setSubmissionMode(Context context, boolean value) {
    SharedPreferences sharedPref = PreferenceManager.getDefaultSharedPreferences(context);
    SharedPreferences.Editor preferencesEditor = sharedPref.edit();

    preferencesEditor.putString(
        AppConstants.RUN_MODE,
        value ? AppConstants.SUBMISSION_MODE : AppConstants.PERFORMANCE_LITE_MODE);
    preferencesEditor.apply();
  }

  public static boolean getSubmissionMode(Context context) {
    SharedPreferences sharedPref = PreferenceManager.getDefaultSharedPreferences(context);

    return sharedPref.getString(AppConstants.RUN_MODE, "").equals(AppConstants.SUBMISSION_MODE);
  }

  public static Intent getShareResultsIntent(Context context) {
    String filePath = org.mlperf.inference.MLPerfTasks.getResultsJsonPath();
    String results;
    StringBuilder sb = new StringBuilder();
    try (BufferedReader reader = new BufferedReader(new FileReader(filePath))) {
      String line;
      while ((line = reader.readLine()) != null) {
        sb.append(line);
        sb.append(System.lineSeparator());
      }
      results = sb.toString();
    } catch (Exception e) {
      e.printStackTrace();
      results = "Fail to get results";
    }

    Intent shareIntent = new Intent();

    shareIntent.setAction(Intent.ACTION_SEND);
    shareIntent.putExtra(Intent.EXTRA_TEXT, results);
    shareIntent.setType("text/json");
    shareIntent.putExtra(Intent.EXTRA_SUBJECT, "Experiment results");
    shareIntent.putExtra(Intent.EXTRA_EMAIL, new String[] {});

    return shareIntent;
  }

  public static void setTestActivity(Context context, String id, boolean value) {
    SharedPreferences sharedPref = PreferenceManager.getDefaultSharedPreferences(context);
    sharedPref.edit().putBoolean(AppConstants.ACTIVE_TESTS + id, value).apply();
  }

  public static boolean getTestActivity(Context context, String id) {
    SharedPreferences sharedPref = PreferenceManager.getDefaultSharedPreferences(context);
    return sharedPref.getBoolean(AppConstants.ACTIVE_TESTS + id, true);
  }
}
