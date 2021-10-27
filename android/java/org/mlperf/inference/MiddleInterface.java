/* Copyright 2020 The MLPerf Authors. All Rights Reserved.

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

import static org.mlperf.inference.Backends.BACKEND_LIST;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.Build;
import android.os.HandlerThread;
import android.util.Base64;
import android.util.Log;
import androidx.preference.PreferenceManager;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import org.mlperf.inference.exceptions.UnsupportedDeviceException;
import org.mlperf.proto.BackendSetting;
import org.mlperf.proto.BenchmarkSetting;
import org.mlperf.proto.MLPerfConfig;
import org.mlperf.proto.ModelConfig;
import org.mlperf.proto.Setting;
import org.mlperf.proto.Setting.Value;
import org.mlperf.proto.SettingList;
import org.mlperf.proto.TaskConfig;

/** The Interface for UI to interact with middle end. */
@SuppressWarnings("SpellCheckingInspection")
public final class MiddleInterface implements AutoCloseable, RunMLPerfWorker.Callback {
  public static final String TAG = "MiddleInterface";
  public static final String BATCH = "batch";
  public static final String BACKEND_SETTINGS_KEY_POSFIX = "_backend_settings";

  // Non-static variables.
  private String backendName;
  private SharedPreferences sharedPref;
  private HandlerThread workerThread;
  private RunMLPerfWorker workerHandler;
  private final ProgressData progressData = new ProgressData();
  private Callback callback;
  // Map setting id of common settings to its index.
  private HashMap<String, Integer> commonSettingIdToIndexMap = null;
  // Map benchmark_id to its index.
  private HashMap<String, Integer> benchmarkIdToIndexMap = null;
  // Map setting id of benchmark settings to benchmark's and its index.
  private BackendSetting backendSettings;

  public MiddleInterface(Callback callback) throws UnsupportedDeviceException {
    if (backendName == null) {
      for (String b : BACKEND_LIST) {
        Log.i(TAG, "Checking support for backend: " + b);
        String backendlibname = "lib" + b + "backend.so";
        String[] retstrings = isBackendSupported(backendlibname, Build.MANUFACTURER, Build.MODEL);
        String msg = retstrings[0];

        //noinspection IfCanBeSwitch
        if (msg.equals("Supported")) {
          try {
            Log.i(TAG, "### Config: " + retstrings[1]);
            byte[] settingProto = convertProto(retstrings[1]);
            backendSettings = BackendSetting.parseFrom(settingProto);
            Log.i(TAG, "Backend supported");
            backendName = backendlibname;
          } catch (Exception e) {
            throw new RuntimeException(e);
          }
          break;
        } else if (msg.equals("UnsupportedSoc")) {
          Log.i(TAG, "Soc Not supported. Trying next lib.");
        } else if (msg.equals("Unsupported")) {
          Log.e(TAG, "Soc detected but it is unsupported");
          // TODO - display a dialog that the phone is not supported and exit the app
          throw UnsupportedDeviceException.make("Soc detected but phone is unsupported", b);
        }
      }
    }

    if (backendName == null) {
      Log.e(TAG, "No viable backends found");
      return;
    }

    this.callback = callback;
    sharedPref = PreferenceManager.getDefaultSharedPreferences(MLCtx.getInstance().getContext());

    // Generate mappings.
    commonSettingIdToIndexMap = new HashMap<>();
    benchmarkIdToIndexMap = new HashMap<>();
    BackendSetting settings = getSettings();
    for (int i = 0; i < settings.getCommonSettingCount(); ++i) {
      commonSettingIdToIndexMap.put(settings.getCommonSetting(i).getId(), i);
    }
    for (int i = 0; i < settings.getBenchmarkSettingCount(); ++i) {
      BenchmarkSetting bm = settings.getBenchmarkSetting(i);
      benchmarkIdToIndexMap.put(bm.getBenchmarkId(), i);
    }
  }

  // Get the list of benchmarks.
  public ArrayList<Benchmark> getBenchmarks() {
    MLPerfConfig mlperfTasks = MLPerfTasks.getConfig();
    ArrayList<Benchmark> benchmarks = new ArrayList<>();
    for (TaskConfig task : mlperfTasks.getTaskList()) {
      for (ModelConfig model : task.getModelList()) {
        if (!hasBenchmark(model.getId())) {
          continue;
        }
        benchmarks.add(new Benchmark(task, model));
      }
    }
    return benchmarks;
  }

  // Run all benchmarks with their current settings.
  public void runBenchmarks(String mode) throws Exception {
    progressData.results.clear();
    // If the phone SoC was detected but the phone is not supported, then return
    if (backendName == null) {
      return;
    }
    if (workerThread == null) {
      workerThread = new HandlerThread("MLPerf.Worker");
      workerThread.start();
      workerHandler = new RunMLPerfWorker(workerThread.getLooper(), this);
    }

    ArrayList<Benchmark> unfilteredBenchmarks = getBenchmarks();
    ArrayList<Benchmark> benchmarks = new ArrayList<Benchmark>();
    Context ctx = MLCtx.getInstance().getContext();
    float product = 1f;
    for (Benchmark bm : unfilteredBenchmarks) {
      if (Util.getTestActivity(ctx, bm.getId())) {
        if (mode.equals(AppConstants.SUBMISSION_MODE)) {
          if (!bm.checkGroundtruthFile()) {
            String error = "Groundtruth file is missed for task " + bm.getTaskName();
            throw new Exception(error);
          }
        }
        benchmarks.add(bm);
        product *= bm.getMaxScore();
      }
    }
    float geomean = Math.round(Math.pow(product, 1.0 / (float) (benchmarks.size())));
    Benchmark.setSummaryMaxScore(geomean);

    String runMode =
        sharedPref.getString(AppConstants.RUN_MODE, AppConstants.PERFORMANCE_LITE_MODE);
    int benchmarksSize = benchmarks.size();
    progressData.numFinished = 0;
    progressData.numBenchmarks =
        runMode.equals(AppConstants.SUBMISSION_MODE) ? benchmarksSize * 2 : benchmarksSize;
    class StartData {
      public final String id;
      public final String logDir;
      public final String mode;
      public final int batch;

      public StartData(String id, String logDir, String mode, int batch) {
        this.id = id;
        this.logDir = logDir;
        this.mode = mode;
        this.batch = batch;
      }
    }
    StartData[] startingList = new StartData[progressData.numBenchmarks];
    int counter = 0;
    for (Benchmark bm : benchmarks) {
      // Get batch size
      int batch = 1;
      try {
        batch = getBenchmark(bm.getId()).getBatchSize();
      } catch (Exception e) {
        Log.e(
            TAG, "Batch size is not defined for \"" + bm.getId() + "\" benchmark or is incorrect");
        batch = 1;
      }
      if (runMode.equals(AppConstants.SUBMISSION_MODE)) {
        String logDir = AppConstants.RESULTS_DIR + "log_performance/" + bm.getId();
        File appDir = new File(logDir);
        appDir.mkdirs();
        Log.d(TAG, "log_performance file path: " + logDir);
        startingList[counter] =
            new StartData(bm.getId(), logDir, AppConstants.PERFORMANCE_LITE_MODE, batch);
        logDir = AppConstants.RESULTS_DIR + "log_accuracy/" + bm.getId();
        appDir = new File(logDir);
        appDir.mkdirs();
        Log.d(TAG, "log_accuracy file path: " + logDir);

        startingList[counter + benchmarksSize] =
            new StartData(bm.getId(), logDir, AppConstants.ACCURACY_MODE, batch);
      } else {
        String logDirBase;
        if (runMode.equals(AppConstants.PERFORMANCE_LITE_MODE)
            || runMode.equals(AppConstants.PERFORMANCE_MODE)
            || runMode.equals(AppConstants.TESTING_MODE)) {
          logDirBase = "log_performance/";
        } else if (runMode.equals(AppConstants.ACCURACY_MODE)) {
          logDirBase = "log_accuracy/";
        } else {
          logDirBase = "log_undefined/";
        }
        String logDir = AppConstants.RESULTS_DIR + logDirBase + bm.getId();
        File appDir = new File(logDir);
        appDir.mkdirs();
        Log.d(TAG, logDirBase + " file path: " + logDir);

        startingList[counter] = new StartData(bm.getId(), logDir, runMode, batch);
      }
      counter++;
    }
    int benchmarkOrderNumber = 0;
    for (StartData sd : startingList) {
      workerHandler.scheduleBenchmark(sd.id, sd.logDir, sd.mode, sd.batch, benchmarkOrderNumber);
      benchmarkOrderNumber++;
    }
  }

  public void abortBenchmarks() {
    // It is not possible to stop the loadgen. So just cancel all waiting jobs.
    if (workerHandler != null) {
      workerHandler.removeMessages();
    }

    // Update the number of benchmarks got run.
    if (progressData != null) progressData.numBenchmarks = progressData.numStarted;
  }

  public BackendSetting getSettings() {
    try {
      String backendPref = sharedPref.getString(getBackendKey(), null);
      if (backendPref != null && !backendPref.isEmpty()) {
        return BackendSetting.parseFrom(Base64.decode(backendPref, Base64.DEFAULT));
      }

      // If setting not found in the SharedPreference, get the default settings and
      // store it in the SharedPreference.
      setSetting(backendSettings);
      return backendSettings;
    } catch (Exception e) {
      throw new RuntimeException(e);
    }
  }

  // Setting will be stored in the SharedPreference. It will be passed to the real backend when
  // creating a C++ backend object to run the benchmark.
  @SuppressLint("ApplySharedPref")
  public void setSetting(BackendSetting settings) {
    SharedPreferences.Editor preferencesEditor = sharedPref.edit();
    String settingData = Base64.encodeToString(settings.toByteArray(), Base64.DEFAULT);
    preferencesEditor.putString(getBackendKey(), settingData);
    preferencesEditor.commit();
  }

  // Get a single setting in the common settings section by the setting id.
  public Setting getCommonSetting(String settingId) {
    return getSettings().getCommonSetting(commonSettingIdToIndexMap.get(settingId));
  }

  // Load settings from file pushed to device.
  public void loadSettingFromFile(String settingsPath) {
    try {
      BackendSetting backendSetting =
          BackendSetting.parseFrom(MLPerfDriverWrapper.readSettingsFromFile(settingsPath));

      clearAllSettings();
      for (Setting s : backendSetting.getCommonSettingList()) {
        setCommonSetting(s);
      }
      for (BenchmarkSetting bms : backendSetting.getBenchmarkSettingList()) {
        setBenchmarkSetting(bms);
      }
    } catch (IOException e) {
      Log.e(TAG, "Could not find file " + settingsPath);
    } catch (Exception e) {
      Log.e(TAG, e.getLocalizedMessage());
    }
  }

  // Check existance of a benchmark in Backend config
  public boolean hasBenchmark(String benchmarkId) {
    return benchmarkIdToIndexMap.containsKey(benchmarkId);
  }

  // Get a BenchmarkSetting object from Backend
  public BenchmarkSetting getBenchmark(String benchmarkId) {
    BackendSetting backendSettings = getSettings();
    int index = benchmarkIdToIndexMap.get(benchmarkId);
    return backendSettings.getBenchmarkSetting(index);
  }

  // Get full list of settings for a benchmark, including both common and benchmark setting.
  public SettingList getSettingList(String benchmarkId) {
    BackendSetting settings = getSettings();
    SettingList.Builder builder = SettingList.newBuilder();
    for (Setting s : settings.getCommonSettingList()) {
      builder.addSetting(s);
    }
    BenchmarkSetting bms = settings.getBenchmarkSetting(benchmarkIdToIndexMap.get(benchmarkId));
    builder.setBenchmarkSetting(bms);

    SettingList newsetting = builder.build();
    for (Setting s : newsetting.getSettingList()) {
      Log.i(TAG, "JAVA Common setting" + s.getId() + " " + s.getName());
    }
    Log.i(
        TAG,
        "JAVA SettingList"
            + newsetting.getBenchmarkSetting().getAccelerator()
            + " "
            + newsetting.getBenchmarkSetting().getConfiguration());
    return newsetting;
  }

  // Set a single setting in the common settings section by the setting id.
  public void setCommonSetting(Setting setting) {
    BackendSetting originalSettings = getSettings();
    // Build a new setting with updated field.
    BackendSetting.Builder builder = BackendSetting.newBuilder();
    for (Setting s : originalSettings.getCommonSettingList()) {
      if (s.getId().equals(setting.getId()) && isAcceptableValue(setting.getValue(), s)) {
        Setting newSetting = Setting.newBuilder(s).setValue(setting.getValue()).build();
        builder.addCommonSetting(newSetting);
      } else {
        builder.addCommonSetting(s);
      }
    }
    for (BenchmarkSetting bm : originalSettings.getBenchmarkSettingList()) {
      builder.addBenchmarkSetting(bm);
    }
    BackendSetting updatedSettings = builder.build();

    // Update the new settings.
    setSetting(updatedSettings);
  }

  // Set a single setting in the benchmark settings section.
  public void setBenchmarkSetting(BenchmarkSetting new_bm) {
    BackendSetting originalSettings = getSettings();
    // Build a new setting with updated field.
    BackendSetting.Builder builder = BackendSetting.newBuilder();
    for (Setting s : originalSettings.getCommonSettingList()) {
      builder.addCommonSetting(s);
    }
    for (BenchmarkSetting bm : originalSettings.getBenchmarkSettingList()) {
      if (bm.getBenchmarkId().equals(new_bm.getBenchmarkId())) {
        builder.addBenchmarkSetting(new_bm);
      } else {
        builder.addBenchmarkSetting(bm);
      }
    }
    BackendSetting updatedSettings = builder.build();

    // Update the new settings.
    setSetting(updatedSettings);
  }

  private void clearAllSettings() {
    BackendSetting.Builder builder = BackendSetting.newBuilder();
    BackendSetting emptySettings = builder.build();

    setSetting(emptySettings);
  }

  private boolean isAcceptableValue(Value value, Setting originalSetting) {
    for (Value v : originalSetting.getAcceptableValueList()) {
      if (value.getName().equals(v.getName()) && value.getValue().equals(v.getValue())) {
        return true;
      }
    }
    return false;
  }

  // run an arbitrary diagnostic command, return arbitrary multi-line text, used by diagnostic
  // screen
  @SuppressWarnings("SameReturnValue")
  public String runDiagnostic(String command) {
    return "runDiagnostic not yet implemented";
  }

  // get the url to open if user elects to share scores
  @SuppressWarnings("SameReturnValue")
  public String getShareUrl() {
    return "getShareUrl not yet implemented";
  }

  @Override
  public void close() {
    if (workerThread != null) {
      workerThread.quit();
    }
  }

  @Override
  public void onBenchmarkStarted(String benchmarkId) {
    synchronized (progressData) {
      progressData.numStarted++;
    }
    callback.onbenchmarkStarted(benchmarkId);
  }

  @Override
  public void onBenchmarkFinished(ResultHolder result) {
    if (callback == null) {
      return;
    }

    // Update the progress.
    synchronized (progressData) {
      progressData.numFinished++;
      progressData.results.add(result);
      if (!result.getMode().equals(AppConstants.ACCURACY_MODE)) {
        progressData.summaryScore *= result.getScore();
        progressData.numScores++;
      }
      callback.onProgressUpdate(progressData.getProgress());
      callback.onbenchmarkFinished(result);
      if (progressData.numFinished == progressData.numBenchmarks) {
        float summaryScore = 0.0f;
        if (progressData.numScores == 0) {
          progressData.summaryScore = 0;
        } else {
          // summary score is geometric mean
          summaryScore = (float) Math.pow(progressData.summaryScore, 1.0 / progressData.numScores);
        }
        callback.onAllBenchmarksFinished(summaryScore, progressData.results);
      }
    }
  }

  @Override
  public void onCoolingStarted() {
    callback.oncoolingStarted();
  }

  @Override
  public void onCoolingFinished() {
    callback.oncoolingFinished();
  }

  private String getBackendKey() {
    return backendName + BACKEND_SETTINGS_KEY_POSFIX;
  }

  public String getBackendName() {
    return backendName;
  }

  public static native String[] isBackendSupported(
      String lib_path, String manufacturer, String model);

  // Convert text proto file to binary proto.
  public static native byte[] convertProto(String text);

  /** Callback with progres update. */
  public interface Callback {
    // Notify a change in the progress.
    void onProgressUpdate(int percent);

    // Notify that a benchmark is done.
    void onbenchmarkFinished(ResultHolder result);

    void onbenchmarkStarted(String benchmarkId);

    void oncoolingStarted();

    void oncoolingFinished();

    // Notify when all
    void onAllBenchmarksFinished(float summaryScore, ArrayList<ResultHolder> results);
  }

  // A class to bind all variable related to progress update.
  private static class ProgressData {
    // Total number of benchmarks.
    public int numBenchmarks;
    // Number of benchmarks started.
    public int numStarted;
    // Number of benchmarks finished.
    public int numFinished;
    // Number of benchmarks with Scores.
    public int numScores;
    // Summary score will be calculated if all benchmarks finished.
    public float summaryScore;
    // Results of finished benchmarks.
    public final ArrayList<ResultHolder> results;

    public ProgressData() {
      numBenchmarks = 0;
      numStarted = 0;
      numFinished = 0;
      numScores = 0;
      summaryScore = 1;
      results = new ArrayList<>();
    }

    public int getProgress() {
      return (100 * numFinished) / numBenchmarks;
    }
  }

  static {
    NativeEvaluation.init();
  }
}
