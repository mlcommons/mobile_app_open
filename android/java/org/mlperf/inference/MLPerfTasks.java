/* Copyright 2019-2021 The MLPerf Authors. All Rights Reserved.

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

import android.text.TextUtils;
import android.util.Log;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.time.LocalDateTime;
import java.util.ArrayList;
import java.util.HashMap;
import org.json.JSONArray;
import org.json.JSONObject;
import org.mlperf.proto.MLPerfConfig;
import org.mlperf.proto.ModelConfig;
import org.mlperf.proto.TaskConfig;

/** This class reads the tasks_v2.pbtxt and provides quick inference to its values. */
public final class MLPerfTasks {
  private static final String TAG = "MLPerfTasks";
  private static final String ZIP = ".zip";

  private static MLPerfConfig mlperfTasks;
  private static String configPath;
  // Map a benchmark id to its TaskConfig.
  private static HashMap<String, TaskConfig> taskConfigMap;
  // Map a benchmark id to its ModelConfig.
  private static HashMap<String, ModelConfig> modelConfigMap;

  // Make this class not instantiable.
  private MLPerfTasks() {}

  public static void setConfigPath(
      String configIntentPath, String configLocalPath, String configDefaultPath) {
    // Load path specified by ADB if provided
    if (!TextUtils.isEmpty(configIntentPath)) {
      Log.w(TAG, "WARNING Overriding tasks_v2.pbtxt with intent");
      configPath = configIntentPath;
      // Load config from SD card if provided to support no-wifi testing
    } else if (!TextUtils.isEmpty(configLocalPath)) {
      Log.w(TAG, "WARNING Overriding tasks_v2.pbtxt from sdcard");
      configPath = configLocalPath;
      // Use downloaded config from config URL
    } else if (!TextUtils.isEmpty(configDefaultPath)) {
      configPath = configDefaultPath;
    }
    Log.i(TAG, "Set tasks config path: " + configPath);
    mlperfTasks = null;
  }

  public static String getConfigPath() {
    return configPath;
  }

  public static MLPerfConfig getConfig() {
    if (mlperfTasks == null) {
      if (!loadConfigFromUrl(configPath)) {
        Log.e(TAG, "Unable to read config proto file");
      }
      taskConfigMap = new HashMap<>();
      modelConfigMap = new HashMap<>();
      for (TaskConfig task : mlperfTasks.getTaskList()) {
        for (ModelConfig model : task.getModelList()) {
          taskConfigMap.put(model.getId(), task);
          modelConfigMap.put(model.getId(), model);
        }
      }
    }
    return mlperfTasks;
  }

  private static boolean loadConfigFromUrl(String url) {
    String localPath = getLocalPath(url);
    // Load config from file
    try {
      mlperfTasks = MLPerfConfig.parseFrom(MLPerfDriverWrapper.readConfigFromFile(localPath));
      return true;
    } catch (IOException e) {
      Log.e(TAG, "Could not find file  " + localPath);
      return false;
    } catch (Exception e) {
      Log.e(TAG, e.getLocalizedMessage());
      return false;
    }
  }

  public static TaskConfig getTaskConfig(String benchmarkId) {
    if (taskConfigMap == null) {
      getConfig();
    }
    return taskConfigMap.get(benchmarkId);
  }

  public static ModelConfig getModelConfig(String benchmarkId) {
    if (modelConfigMap == null) {
      getConfig();
    }
    return modelConfigMap.get(benchmarkId);
  }

  public static boolean isZipFile(String path) {
    return path.endsWith(ZIP);
  }

  public static String getLocalPath(String path) {

    // Use the localDir's cache directory path only if its a github link or assets link.
    // For local files, use it as is.
    String filename = new File(path).getName();
    if (path.startsWith("http://") || path.startsWith("https://") || isZipFile(filename)) {
      if (isZipFile(filename)) {
        filename = filename.substring(0, filename.length() - ZIP.length());
      }
      String localDir =
          MLCtx.getInstance().getContext().getExternalFilesDir("cache").getAbsolutePath();
      return localDir + "/cache/" + filename;
    } else {
      return path;
    }
  }

  public static String getResultsJsonPath() {
    return "/sdcard/mlperf_results/mlperf/results.json";
  }

  // Update the results.json file.
  public static void resultsToFile(ArrayList<ResultHolder> results, String mode) {
    File resultsFile = new File(MLPerfTasks.getResultsJsonPath());
    File resultsFileDir = new File(resultsFile.getParent());
    resultsFileDir.mkdirs();
    FileWriter writer;
    try {
      JSONArray resultsArray = new JSONArray();
      for (ResultHolder result : results) {
        JSONObject resultObj = new JSONObject();
        resultObj.put("benchmark_id", result.getId());

        JSONObject configObj = new JSONObject();
        configObj.put("runtime", result.getRuntime());
        resultObj.put("configuration", configObj);

        resultObj.put("score", Float.toString(result.getScore()));
        if (mode.equals(AppConstants.PERFORMANCE_LITE_MODE)) resultObj.put("accuracy", "N/A");
        else resultObj.put("accuracy", result.getAccuracy());

        resultObj.put("min_duration", Float.toString(result.getMinDuration()));
        resultObj.put("duration", Float.toString(result.getDuration()));

        resultObj.put("min_samples", Long.toString(result.getMinSamples()));
        resultObj.put("num_samples", Long.toString(result.getNumSamples()));

        resultObj.put("mode", result.getMode());
        resultObj.put("datetime", LocalDateTime.now().toString());
        resultsArray.put(resultObj);
      }
      writer = new FileWriter(resultsFile);
      writer.append(resultsArray.toString(4));
      writer.flush();
      writer.close();
    } catch (Exception e) {
      Log.e(TAG, "Failed to write results.json file: " + e.getMessage());
    }

    Log.i(TAG, "MLPerf Inference Phase Finished");
  }
}
