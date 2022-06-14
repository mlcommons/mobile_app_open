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

import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Messenger;
import android.util.Log;
import androidx.annotation.NonNull;
import java.io.File;
import org.mlperf.proto.BenchmarkSetting;
import org.mlperf.proto.DatasetConfig;
import org.mlperf.proto.ModelConfig;
import org.mlperf.proto.TaskConfig;

/**
 * Worker utility to run MLPerf.
 *
 * <p>RunMLPerfWorker is designed to run a single model to avoid restarting a big work if
 * terminated.
 */
@SuppressWarnings("SpellCheckingInspection")
public final class RunMLPerfWorker implements Handler.Callback {
  public static final int MSG_RUN = 1;
  public static final int PAUSE_1_MINUTE = 60 * 1000; // milliseconds
  public static final float MILLISECONDS_IN_SECOND = 1000f;
  public static final String TAG = "RunMLPerfWorker";
  public static final int MAX_LATENCY_NS = 1000000;

  private final Handler handler;
  private final Callback callback;

  public RunMLPerfWorker(@NonNull Looper looper, @NonNull Callback callback) {
    handler = new Handler(looper, this);
    this.callback = callback;
  }

  @Override
  public boolean handleMessage(Message msg) {
    // Gets the data.
    WorkerData data = (WorkerData) msg.obj;
    Messenger messenger = msg.replyTo;
    Log.i(TAG, "handleMessage() " + data.benchmarkId);
    callback.onBenchmarkStarted(data.benchmarkId);
    int batch = data.batch;

    // Runs the model.
    String mode = "SubmissionRun";
    TaskConfig taskConfig = MLPerfTasks.getTaskConfig(data.benchmarkId);
    ModelConfig modelConfig = MLPerfTasks.getModelConfig(data.benchmarkId);
    DatasetConfig dataset = taskConfig.getDataset();
    String modelName = modelConfig.getName();
    String runtime;

    int minQueryCount = taskConfig.getMinQueryCount();
    int minDurationMs = taskConfig.getMinDurationMs();

    if (data.mode.equals(AppConstants.SUBMISSION_MODE)) {
      mode = "SubmissionRun";
    } else if (data.mode.equals(AppConstants.ACCURACY_MODE)) {
      mode = "AccuracyOnly";
    } else if (data.mode.equals(AppConstants.PERFORMANCE_MODE)) {
      mode = "PerformanceOnly";
    } else if (data.mode.equals(AppConstants.PERFORMANCE_LITE_MODE)) {
      mode = "PerformanceOnly";
      if (taskConfig.hasLiteDataset()) {
        dataset = taskConfig.getLiteDataset();
      }
    } else if (data.mode.equals(AppConstants.TESTING_MODE)) {
      minQueryCount = 10;
      minDurationMs = 0;
      if (taskConfig.hasTestDataset()) {
        dataset = taskConfig.getTestDataset();
      }
    }

    if (!new File(MLPerfTasks.getLocalPath(dataset.getPath())).canRead()) {
      Log.e(TAG, "Error: Dataset is missing");
      return false;
    }
    Log.i(TAG, "Running inference for \"" + modelName + "\"...");

    try {
      MLPerfDriverWrapper.Builder builder = new MLPerfDriverWrapper.Builder();
      MiddleInterface middleInterface = new MiddleInterface(null);

      // Set the backend.
      String backendName = middleInterface.getBackendName();
      if (backendName != null && !backendName.equals("")) {
        runtime = "External";
        String src = middleInterface.getBenchmark(modelConfig.getId()).getSrc();
        builder.useExternalBackend(
            MLPerfTasks.getLocalPath(src),
            backendName,
            middleInterface.getSettingList(modelConfig.getId()).toByteArray(),
            MLCtx.getInstance().getContext().getApplicationInfo().nativeLibraryDir);
      } else {
        Log.e(TAG, "The provided backend type is not supported");
        return false;
      }

      BenchmarkSetting bm = middleInterface.getBenchmark(modelConfig.getId());

      int singleStreamExpectedLatencyNs = bm.getSingleStreamExpectedLatencyNs();
      if (singleStreamExpectedLatencyNs > MAX_LATENCY_NS) {
        throw new Exception(
            "single_stream_expected_latency_ns must be less than "
                + MAX_LATENCY_NS
                + " but is "
                + singleStreamExpectedLatencyNs);
      }

      // Set the dataset.
      switch (dataset.getType()) {
        case IMAGENET:
          builder.useImagenet(
              MLPerfTasks.getLocalPath(dataset.getPath()),
              MLPerfTasks.getLocalPath(dataset.getGroundtruthSrc()),
              modelConfig.getOffset(),
              modelConfig.getImageWidth() /* 224 */,
              modelConfig.getImageHeight() /* 224 */);
          break;
        case COCO:
          builder.useCoco(
              MLPerfTasks.getLocalPath(dataset.getPath()),
              MLPerfTasks.getLocalPath(dataset.getGroundtruthSrc()),
              modelConfig.getOffset(),
              modelConfig.getNumClasses() /* 91 */,
              modelConfig.getImageWidth() /* 320 */,
              modelConfig.getImageHeight() /* 320 */);
          break;
        case SQUAD:
          builder.useSquad(
              MLPerfTasks.getLocalPath(dataset.getPath()),
              MLPerfTasks.getLocalPath(dataset.getGroundtruthSrc()));
          break;
        case ADE20K:
          builder.useAde20k(
              // The current dataset don't have ground truth images.
              MLPerfTasks.getLocalPath(dataset.getPath()),
              MLPerfTasks.getLocalPath(dataset.getGroundtruthSrc()),
              modelConfig.getNumClasses() /* 31 */,
              modelConfig.getImageWidth() /* 512 */,
              modelConfig.getImageHeight() /* 512 */);
          break;
      }

      // Run cooldown pause only before benchmark in performance mode
      // Do not run cooldown pause before first benchmark
      if (data.benchmarkOrderNumber != 0 && mode.equals("PerformanceOnly") && Util.getCooldown()) {
        callback.onCoolingStarted();
        Log.d(TAG, "Cooldown started");
        Thread.sleep(Util.getCooldownPause(MLCtx.getInstance().getContext()) * PAUSE_1_MINUTE);
        callback.onCoolingFinished();
        Log.d(TAG, "Cooldown finished");
      }

      MLPerfDriverWrapper driverWrapper = builder.build(modelConfig.getScenario(), batch);
      driverWrapper.runMLPerf(
          mode, minQueryCount, minDurationMs, singleStreamExpectedLatencyNs, data.outputFolder);
      Log.i(TAG, "Finished running \"" + modelName + "\".");
      ResultHolder result = new ResultHolder(taskConfig.getName(), data.benchmarkId);
      result.setRuntime(runtime);
      if ("Offline".equals(modelConfig.getScenario())) {
        result.setScore(driverWrapper.getLatency());
      } else {
        result.setScore(MILLISECONDS_IN_SECOND / driverWrapper.getLatency());
      }
      result.setAccuracy(driverWrapper.getAccuracy());
      result.setMinSamples(taskConfig.getMinQueryCount());
      result.setNumSamples(driverWrapper.getNumSamples());
      result.setMinDuration(taskConfig.getMinDurationMs());
      result.setDuration(driverWrapper.getDurationMs());
      result.setMode(data.mode);
      callback.onBenchmarkFinished(result);
      driverWrapper.close();
    } catch (Exception e) {
      Log.e(TAG, "Running \"" + modelName + "\" failed with error: " + e.getMessage());
      Log.e(TAG, Log.getStackTraceString(e));
      return false;
    }
    return true;
  }

  // Schedule a benchmark by sending a message to handler.
  public void scheduleBenchmark(
      String benchmarkId, String outputFolder, String mode, int batch, int benchmarkOrderNumber) {
    WorkerData data = new WorkerData(benchmarkId, outputFolder, mode, batch, benchmarkOrderNumber);
    Message msg = handler.obtainMessage(MSG_RUN, data);
    handler.sendMessage(msg);
  }

  public void removeMessages() {
    handler.removeMessages(MSG_RUN);
  }

  private boolean isExternalBackend(String backend) {
    return backend.startsWith("lib") && backend.endsWith(".so");
  }

  /** Defines data for this worker. */
  public static class WorkerData {
    public WorkerData(
        String benchmarkId, String outputFolder, String mode, int batch, int benchmarkOrderNumber) {
      this.benchmarkId = benchmarkId;
      this.outputFolder = outputFolder;
      this.mode = mode;
      this.batch = batch;
      this.benchmarkOrderNumber = benchmarkOrderNumber;
    }

    protected final String benchmarkId;
    protected final String outputFolder;
    protected final String mode;
    protected final int batch;
    protected final int benchmarkOrderNumber;
  }

  /** Callback interface to return progress and results. */
  public interface Callback {
    // Notify that a benchmark is being run.
    void onBenchmarkStarted(String benchmarkId);

    // Notify that a benchmark is finished.
    void onBenchmarkFinished(ResultHolder result);

    // Notify that a cooling down is started.
    void onCoolingStarted();

    // Notify that a cooling down is finished.
    void onCoolingFinished();
  }
}
