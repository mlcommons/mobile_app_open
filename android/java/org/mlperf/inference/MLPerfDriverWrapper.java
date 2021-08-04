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

import java.util.ArrayList;

/** A class that wraps functionality around tflite::mlperf::MlperfDriver. */
@SuppressWarnings("SpellCheckingInspection")
public final class MLPerfDriverWrapper implements AutoCloseable {
  /**
   * MLPerfDriverWrapper constructor is marked as private since the dataset pointer should be hold,
   * managed and deleted by MlperfDriver. Letting it to be initialized outside this class can lead
   * to various memory management problems.
   *
   * @param scenario is a custom scenario string to use custom config in mlperf_driver.cc.
   */
  private MLPerfDriverWrapper(long datasetHandle, long backendHandle, String scenario, int batch) {
    driverHandle = nativeInit(datasetHandle, backendHandle, scenario, batch);
  }

  /**
   * {@link runMLPerf} runs a specific model with mlperf.
   *
   * @param mode could be a string of PerformanceOnly, AccuracyOnly or SubmissionRun (both).
   * @param minQueryCount is the minimum number of samples should be run.
   * @param minDurationMs is the minimum duration in ms. After both conditions are met, the test
   *     ends.
   * @param outputDir is the directory to store the log files.
   */
  @SuppressWarnings("JavadocReference")
  public void runMLPerf(String mode, int minQueryCount, int minDurationMs, String outputDir) {
    nativeRun(driverHandle, mode, minQueryCount, minDurationMs, outputDir);
  }

  // The latency in ms is formatted with two decimal places.
  public float getLatency() {
    return nativeGetLatency(this.driverHandle);
  }

  // The groundtruth file and format of the accuracy string is up to tasks.
  // Ex: mobilenet image classification returns accuracy as 12.34%.
  public String getAccuracy() {
    return nativeGetAccuracy(this.driverHandle);
  }

  // Return the number of sample reported by loadgen.
  public long getNumSamples() {
    return nativeGetNumSamples(this.driverHandle);
  }

  // Return the duration reported by loadgen.
  public float getDurationMs() {
    return nativeGetDurationMs(this.driverHandle);
  }

  @Override
  public void close() {
    nativeDelete(driverHandle);
  }

  // List devices available for NNAPI. This only works on API >= 29, otherwise it returns an
  // empty list.
  public static native ArrayList<String> listDevicesForNNAPI();

  // Convert text config proto file to binary proto.
  public static native byte[] readConfigFromFile(String text);

  // Convert text backend_settings file to binary proto.
  public static native byte[] readSettingsFromFile(String text);

  // Native functions.
  private native long nativeInit(
      long datasetHandle, long backendHandle, String scenario, int batch);

  private native void nativeRun(
      long driverHandle, String jmode, int minQueryCount, int minDuration, String outputDir);

  private native float nativeGetLatency(long handle);

  private native String nativeGetAccuracy(long handle);

  private native long nativeGetNumSamples(long handle);

  private native float nativeGetDurationMs(long handle);

  // Nullness of the pointer is checked inside nativeDelete. Callers can skip that check.
  private native void nativeDelete(long handle);

  // Native functions for dataset manipulation. Nullness of the pointer is checked
  // inside nativeDeleteDataset. Callers can skip that check.
  private static native void nativeDeleteDataset(long handle);

  // Return a pointer of a new Imagenet C++ object.
  private static native long imagenet(
      long backendHandle,
      String imageDir,
      String groundtruthFile,
      int offset,
      int imageWidth,
      int imageHeight);

  // Return a pointer of a new Coco C++ object.
  private static native long coco(
      long backendHandle,
      String imageDir,
      String groundtruthFile,
      int offset,
      int numClasses,
      int imageWidth,
      int imageHeight);

  // Return a pointer of a new Squad C++ object.
  private static native long squad(long backendHandle, String inputFile, String groundtruthFile);

  // Return a pointer of a new ADE20K C++ object.
  private static native long ade20k(
      long backendHandle,
      String imageDir,
      String groundtruthDir,
      int numClasses,
      int imageWidth,
      int imageHeight);

  // Native functions for backend manipulation. Nullness of the pointer is checked
  // inside nativeDeleteBackend. Callers can skip that check.
  private static native void nativeDeleteBackend(long handle);

  // Return a pointer of a new external Backend object.
  private static native long externalBackend(
      String modelFilePath, String libPath, byte[] settings, String nativeLibPath);

  // driverHandle holds a pointer of TfliteMlperfDriver.
  private final long driverHandle;

  /**
   * The Builder class for MLPerfDriverWrapper.
   *
   * <p>The dataset should be set by one of the functions like: useImagenet,...
   */
  public static class Builder implements AutoCloseable {
    private long backend = 0;
    private long dataset = 0;

    public Builder() {}

    public Builder useExternalBackend(
        String modelFilePath, String libPath, byte[] settings, String nativeLibPath) {
      nativeDeleteBackend(backend);
      backend = externalBackend(modelFilePath, libPath, settings, nativeLibPath);
      return this;
    }

    // Offset is used to match ground-truth categories with model output.
    // Some models assume class 0 is background class thus they have offset=1.
    public Builder useImagenet(
        String imageDir, String groundtruthFile, int offset, int imageWidth, int imageHeight) {
      nativeDeleteDataset(dataset);
      dataset = imagenet(getBackend(), imageDir, groundtruthFile, offset, imageWidth, imageHeight);
      return this;
    }

    // Some models assume class 0 is null class thus they have offset=1.
    public Builder useCoco(
        String imageDir,
        String groundtruthFile,
        int offset,
        int numClasses,
        int imageWidth,
        int imageHeight) {
      nativeDeleteDataset(dataset);
      dataset =
          coco(
              getBackend(), imageDir, groundtruthFile, offset, numClasses, imageWidth, imageHeight);
      return this;
    }

    public Builder useSquad(String inputFile, String groundtruthFile) {
      nativeDeleteDataset(dataset);
      dataset = squad(getBackend(), inputFile, groundtruthFile);
      return this;
    }

    public Builder useAde20k(
        String imageDir, String groundtruthDir, int numClasses, int imageWidth, int imageHeight) {
      nativeDeleteDataset(dataset);
      dataset = ade20k(getBackend(), imageDir, groundtruthDir, numClasses, imageWidth, imageHeight);
      return this;
    }

    public MLPerfDriverWrapper build(String scenario, int batch) {
      MLPerfDriverWrapper result =
          new MLPerfDriverWrapper(getDataset(), getBackend(), scenario, batch);
      dataset = 0;
      backend = 0;
      return result;
    }

    private long getBackend() {
      if (backend == 0) {
        throw new java.lang.IllegalArgumentException("Backend should be set first");
      }
      return backend;
    }

    private long getDataset() {
      if (dataset == 0) {
        throw new java.lang.IllegalArgumentException("Dataset should be set first");
      }
      return dataset;
    }

    @Override
    public void close() {
      nativeDeleteDataset(dataset);
      nativeDeleteBackend(backend);
    }
  }

  static {
    NativeEvaluation.init();
  }
}
