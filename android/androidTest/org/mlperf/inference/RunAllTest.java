/* Copyright 2019 The MLPerf Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

package org.mlperf.inference;

import static androidx.test.espresso.Espresso.onView;
import static androidx.test.espresso.action.ViewActions.click;
import static androidx.test.espresso.matcher.ViewMatchers.withId;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.SharedPreferences;
import android.nfc.FormatException;
import android.os.Build;
import android.util.Log;
import android.view.View;
import androidx.preference.PreferenceManager;
import androidx.test.core.app.ActivityScenario;
import androidx.test.core.app.ApplicationProvider;
import androidx.test.espresso.IdlingPolicies;
import androidx.test.espresso.IdlingRegistry;
import androidx.test.espresso.IdlingResource;
import androidx.test.espresso.PerformException;
import androidx.test.espresso.UiController;
import androidx.test.espresso.ViewAction;
import androidx.test.espresso.matcher.ViewMatchers;
import androidx.test.ext.junit.runners.AndroidJUnit4;
import androidx.test.filters.LargeTest;
import androidx.test.rule.GrantPermissionRule;
import java.io.BufferedReader;
import java.io.FileReader;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.TimeUnit;
import org.hamcrest.Matcher;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import org.junit.After;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mlperf.inference.activities.CalculatingActivity;

/** Test for running all models. */
@SuppressWarnings({"RedundantSuppression", "unused"})
@RunWith(AndroidJUnit4.class)
@LargeTest
public class RunAllTest {
  private static final String TAG = "InstrumentedTest";
  private static final int EXPECTED_RESULTS_COUNT = 6;
  /** Reference values: benchmark_id score accuracy */
  @SuppressWarnings("SpellCheckingInspection")
  private static final String[] REFERENCE_RESULTS_STR = {
    "IC_tpu_uint8 40.0 N/A",
    "OD_uint8 40.0 N/A",
    "IS_uint8_deeplabv3 20.0 N/A",
    "IS_uint8_mosaic 30.0 N/A",
    "LU_float32 4.0 N/A",
    "LU_gpu_float32 5.0 N/A",
    "IC_tpu_uint8_offline 10.00 N/A",
  };

  @SuppressWarnings("FieldCanBeLocal")
  private Context ctx;

  @SuppressWarnings("FieldCanBeLocal")
  private SharedPreferences sharedPref;

  private IdlingResource mIdlingResource;

  @SuppressWarnings("rawtypes")
  ActivityScenario activityScenario;

  @Rule
  public GrantPermissionRule permissionRule =
      GrantPermissionRule.grant(android.Manifest.permission.WRITE_EXTERNAL_STORAGE);

  @BeforeClass
  public static void updateIdlingPolicies() {
    IdlingPolicies.setMasterPolicyTimeout(45, TimeUnit.MINUTES);
    IdlingPolicies.setIdlingResourceTimeout(45, TimeUnit.MINUTES);
  }

  @SuppressLint("ApplySharedPref")
  @Before
  public void registerIdlingResource() {
    ctx = ApplicationProvider.getApplicationContext();

    sharedPref = PreferenceManager.getDefaultSharedPreferences(ctx);
    SharedPreferences.Editor preferencesEditor = sharedPref.edit();
    preferencesEditor.putString(AppConstants.RUN_MODE, AppConstants.TESTING_MODE);
    preferencesEditor.commit();

    activityScenario = ActivityScenario.launch(CalculatingActivity.class);
    //noinspection unchecked,Convert2Lambda
    activityScenario.onActivity(
        new ActivityScenario.ActivityAction<CalculatingActivity>() {
          @Override
          public void perform(CalculatingActivity activity) {
            mIdlingResource = activity;
            IdlingRegistry.getInstance().register(mIdlingResource);
          }
        });
  }

  @After
  public void unregisterIdlingResource() {
    if (mIdlingResource != null) {
      IdlingRegistry.getInstance().unregister(mIdlingResource);
    }
    activityScenario.close();
  }

  @Test
  public void testRunAll() throws Exception {
    Map<String, BriefResult> referenceResults;
    referenceResults = new HashMap<>();
    for (String str : REFERENCE_RESULTS_STR) {
      try {
        BriefResult result = new BriefResult(str);
        referenceResults.put(result.benchmarkId, result);
      } catch (Exception e) {
        e.printStackTrace();
        throw new Exception(e.getMessage());
      }
    }
    // Click the play button then wait for all tasks finished.
    onView(withId(R.id.startButton)).perform(click());

    // Wait for benchmark end up to 40 minutes
    onView(withId(R.id.downArrowScroll))
        .perform(WaitForViewsAction.waitFor(ViewMatchers.isDisplayed(), 40 * 60 * 1000));

    // Read benchmark results from JSON
    String filePath = org.mlperf.inference.MLPerfTasks.getResultsJsonPath();
    StringBuilder sb = new StringBuilder();
    try (BufferedReader reader = new BufferedReader(new FileReader(filePath))) {
      String line;
      while ((line = reader.readLine()) != null) {
        sb.append(line);
      }
    } catch (Exception e) {
      e.printStackTrace();
      throw new Exception(e.getMessage());
    }

    // Log system info
    String sysReport =
        "\n************ DEVICE INFORMATION ***********\n"
            + "Brand: "
            + Build.BRAND
            + "\nDevice: "
            + Build.DEVICE
            + "\nModel: "
            + Build.MODEL
            + "\nId: "
            + Build.ID
            + "\nProduct: "
            + Build.PRODUCT
            + "\nBoard: "
            + Build.BOARD
            + "\nHardware: "
            + Build.HARDWARE
            + "\nManufacturer: "
            + Build.MANUFACTURER
            + "\n\n************ FIRMWARE ************\n"
            + "SDK: "
            + Build.VERSION.SDK_INT
            + "\nRelease: "
            + Build.VERSION.RELEASE
            + "\nIncremental: "
            + Build.VERSION.INCREMENTAL
            + "\n\n";
    Log.d(TAG, sysReport);

    // Convert benchmark results to objects, compare with reference values
    try {
      Log.d(TAG, "BENCHMARK RESULT: " + sb.toString());
      JSONArray ja = new JSONArray(sb.toString());
      if (ja.length() != EXPECTED_RESULTS_COUNT)
        throw new AssertionError(
            "Results count (= "
                + ja.length()
                + ") aren't equal to expected (= "
                + EXPECTED_RESULTS_COUNT
                + ")");
      for (int i = 0; i < ja.length(); i++) {
        BriefResult result = new BriefResult(ja.getJSONObject(i));
        BriefResult ref = referenceResults.get(result.benchmarkId);
        if (ref == null)
          throw new AssertionError(
              "Results for benchmark with id = "
                  + result.benchmarkId
                  + " are not not presented in references");
        if (!result.isSimilarTo(ref)) {
          String score = result.score == null ? "N/A" : Float.toString(result.score);
          String accuracy = result.accuracy == null ? "N/A" : Float.toString(result.accuracy);
          String refScore = ref.score == null ? "N/A" : Float.toString(ref.score);
          String refAccuracy = ref.accuracy == null ? "N/A" : Float.toString(ref.accuracy);
          throw new AssertionError(
              "Results for benchmark with id = "
                  + result.benchmarkId
                  + " are not similar with reference values \n"
                  + "Benchmark (score, accuracy) = ("
                  + score
                  + ", "
                  + accuracy
                  + ")\n"
                  + "Reference (score, accuracy) = ("
                  + refScore
                  + ", "
                  + refAccuracy
                  + ")");
        }
      }
    } catch (Exception e) {
      e.printStackTrace();
      throw new Exception(e.getMessage());
    }
  }

  public static class BriefResult {
    public String benchmarkId;
    public Float score;
    public Float accuracy;

    BriefResult(String str) throws FormatException {
      String[] fields = str.trim().split("\\s+");

      if (fields.length != 3) throw new FormatException("Unknown format");
      benchmarkId = fields[0];

      // if Exception: do nothing, because it means that value is "N/A"
      //noinspection CatchMayIgnoreException
      try {
        score = Float.valueOf(fields[1]);
      } catch (Exception e) {
      }
      //noinspection CatchMayIgnoreException
      try {
        accuracy = Float.valueOf(fields[2]);
      } catch (Exception e) {
      }
    }

    BriefResult(JSONObject obj) throws FormatException {
      String scoreStr;
      String accuracyStr;
      try {
        benchmarkId = obj.getString("benchmark_id");
        scoreStr = obj.getString("score");
        accuracyStr = obj.getString("accuracy");
      } catch (JSONException e) {
        throw new FormatException("Unknown format");
      }

      // if Exception do nothing, because it means that value is "N/A"
      //noinspection CatchMayIgnoreException
      try {
        score = Float.valueOf(scoreStr);
      } catch (Exception e) {
      }
      //noinspection CatchMayIgnoreException
      try {
        accuracy = Float.valueOf(accuracyStr);
      } catch (Exception e) {
      }
    }

    /**
     * Function compare current class with "value". Fields "benchmarkId" should must be equal Fields
     * "score" and "accuracy" shouldn't exceed 10x threshold or must be "null"
     *
     * @param value BriefResult object to compare with current
     * @return true if objects are "similar"
     */
    public boolean isSimilarTo(BriefResult value) {
      if (!benchmarkId.equals(value.benchmarkId)) return false;

      if (accuracy == null || value.accuracy == null) {
        //noinspection NumberEquality
        if (accuracy != value.accuracy) return false;
      } else {
        if (accuracy < value.accuracy * 0.01 || accuracy > value.accuracy * 100.0) return false;
      }

      if (score == null || value.score == null) {
        //noinspection NumberEquality,RedundantIfStatement
        if (score != value.score) return false;
      } else {
        //noinspection RedundantIfStatement
        if (score < value.score * 0.01 || score > value.score * 100.0) return false;
      }

      return true;
    }
  }

  public static class WaitForViewsAction implements ViewAction {

    private final Matcher<View> viewMatcher;
    private final long timeoutMs;

    public WaitForViewsAction(Matcher<View> viewMatcher, long timeout) {
      this.viewMatcher = viewMatcher;
      this.timeoutMs = timeout;
    }

    @Override
    public Matcher<View> getConstraints() {
      return ViewMatchers.isDisplayed();
    }

    @Override
    public String getDescription() {
      return "wait until all elements show up";
    }

    @Override
    public void perform(UiController controller, View view) {
      controller.loopMainThreadUntilIdle();
      final long startTime = System.currentTimeMillis();
      final long endTime = startTime + timeoutMs;

      while (System.currentTimeMillis() < endTime) {
        if (viewMatcher.matches(view)) {
          return;
        }

        controller.loopMainThreadForAtLeast(100);
      }

      // Timeout.
      throw new PerformException.Builder()
          .withActionDescription(getDescription())
          .withViewDescription(viewMatcher.toString())
          .build();
    }

    public static ViewAction waitFor(Matcher<View> viewMatcher, long timeout) {
      return new WaitForViewsAction(viewMatcher, timeout);
    }
  }
}
