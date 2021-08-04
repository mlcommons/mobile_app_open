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

import java.io.Serializable;

/** Object to hold result parameters for each benchmark */
// TODO : This class would be a good candidate for AutoValue: go/autovalue/builders.
public class ResultHolder implements Serializable {
  private final String testName;
  private final String benchmarkId;
  private String runtime;
  private float benchmarkScore;
  private String benchmarkAccuracy;
  private long minSamples;
  private long numSamples;
  private float minDuration;
  private float duration;
  private String mode;

  public ResultHolder(String test_name, String id) {
    testName = test_name;
    benchmarkId = id;
    runtime = "";
    benchmarkScore = 0.0f;
    benchmarkAccuracy = "0";
  }

  public void setRuntime(String runtime) {
    this.runtime = runtime;
  }

  public void setScore(float score) {
    this.benchmarkScore = score;
  }

  public void setAccuracy(String accuracy) {
    this.benchmarkAccuracy = accuracy;
  }

  public void setMinSamples(long minSamples) {
    this.minSamples = minSamples;
  }

  public void setNumSamples(long numSamples) {
    this.numSamples = numSamples;
  }

  public void setMinDuration(float minDuration) {
    this.minDuration = minDuration;
  }

  public void setDuration(float duration) {
    this.duration = duration;
  }

  public void setMode(String mode) {
    this.mode = mode;
  }

  public String getId() {
    return benchmarkId;
  }

  public String getTestName() {
    return testName;
  }

  public String getRuntime() {
    return runtime;
  }

  public float getScore() {
    return benchmarkScore;
  }

  public String getAccuracy() {
    return benchmarkAccuracy;
  }

  public long getMinSamples() {
    return this.minSamples;
  }

  public long getNumSamples() {
    return this.numSamples;
  }

  public float getMinDuration() {
    return this.minDuration;
  }

  public float getDuration() {
    return this.duration;
  }

  public String getMode() {
    return this.mode;
  }

  public void reset() {
    runtime = "";
    benchmarkScore = 0.0f;
    benchmarkAccuracy = "0";
  }
}
