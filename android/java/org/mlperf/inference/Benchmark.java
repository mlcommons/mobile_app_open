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

import java.io.File;
import org.mlperf.proto.DatasetConfig;
import org.mlperf.proto.ModelConfig;
import org.mlperf.proto.TaskConfig;

/* BenchMark represents the benchmark of a model. */
public final class Benchmark {
  public static final String TAG = "Benchmark";
  // The config associated with this Benchmark.
  private final ModelConfig modelConfig;
  private final TaskConfig taskConfig;
  private static float maxSummaryScore;

  public Benchmark(TaskConfig task, ModelConfig model) {
    taskConfig = task;
    modelConfig = model;
  }

  public String getId() {
    return modelConfig.getId();
  }

  public String getConfigName() {
    return modelConfig.getName();
  }

  public String getTaskName() {
    return taskConfig.getName();
  }

  public int getIcon() {
    String id = modelConfig.getId();
    Integer icon;

    if (id.startsWith("IC")) {
      if (id.endsWith("offline")) {
        icon = R.drawable.ic_image_processing_2;
      } else {
        icon = R.drawable.ic_image_classification;
      }
    } else if (id.startsWith("IS")) {
      icon = R.drawable.ic_image_segmentation;
    } else if (id.startsWith("OD")) {
      icon = R.drawable.ic_object_detection;
    } else if (id.startsWith("LU")) {
      icon = R.drawable.ic_language_processing;
    } else {
      // TODO create an icon for custom/unknown test
      icon = R.drawable.ic_image_classification;
    }
    return icon;
  }

  public float getMaxScore() {
    return modelConfig.getScoreMax();
  }

  public static void setSummaryMaxScore(float score) {
    // Max score based on the enabled tests
    // geomean is (A*B*C*D..)^(1/N)
    maxSummaryScore = score;
  }

  public static float getSummaryMaxScore() {
    return maxSummaryScore;
  }

  public ModelConfig getModelConfig() {
    return modelConfig;
  }

  public boolean checkGroundtruthFile() {
    DatasetConfig cfg = taskConfig.getDataset();
    String fileName = MLPerfTasks.getLocalPath(cfg.getGroundtruthSrc());
    return new File(fileName).canRead();
  }
}
