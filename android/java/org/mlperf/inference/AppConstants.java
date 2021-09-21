package org.mlperf.inference;

public final class AppConstants {
  // Benchmarking modes
  public static final String PERFORMANCE_LITE_MODE = "performance_lite_mode";
  public static final String ACCURACY_MODE = "accuracy_mode";
  public static final String SUBMISSION_MODE = "submission_mode";
  public static final String PERFORMANCE_MODE = "performance_mode";
  public static final String TESTING_MODE = "testing";

  // String constants: modes, fields, etc
  public static final String RUN_MODE = "run_mode";
  public static final String COOLDOWN_PAUSE = "cooldown_pause";
  public static final String ACTIVE_TESTS = "active_tests:";

  public static final int DEFAULT_COOLDOWN_PAUSE = 5;
  public static final int TYPE_PERFORMANCE = 0;
  public static final int TYPE_ACCURACY = 1;
  public static final int MAX_FILE_AGE_IN_DAYS = 90;
}
