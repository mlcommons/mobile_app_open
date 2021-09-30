package org.mlperf.inference.activities;

import android.annotation.SuppressLint;
import android.app.ActivityManager;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.content.res.Resources;
import android.graphics.Point;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.SpannableString;
import android.text.Spanned;
import android.text.TextUtils;
import android.text.style.RelativeSizeSpan;
import android.util.Base64;
import android.util.Log;
import android.util.TypedValue;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;
import androidx.constraintlayout.widget.ConstraintLayout;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.preference.PreferenceManager;
import androidx.recyclerview.widget.GridLayoutManager;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;
import androidx.test.espresso.IdlingResource;
import java.io.BufferedInputStream;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.OutputStream;
import java.lang.ref.WeakReference;
import java.net.URL;
import java.nio.file.DirectoryStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Objects;
import java.util.Set;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;
import org.apache.commons.io.FileUtils;
import org.mlperf.inference.AppConstants;
import org.mlperf.inference.Benchmark;
import org.mlperf.inference.BuildTimeConstants;
import org.mlperf.inference.MLCtx;
import org.mlperf.inference.MLPerfTasks;
import org.mlperf.inference.MiddleInterface;
import org.mlperf.inference.R;
import org.mlperf.inference.ResultHolder;
import org.mlperf.inference.Util;
import org.mlperf.inference.adapters.DetailedResultItemAdapter;
import org.mlperf.inference.adapters.MeasureItemUtil;
import org.mlperf.inference.adapters.ResultSummaryItemAdapter;
import org.mlperf.inference.exceptions.UnsupportedDeviceException;
import org.mlperf.inference.models.ItemStatus;
import org.mlperf.inference.ui.DiagnosticBackgroundView;
import org.mlperf.inference.ui.LockableScrollView;
import org.mlperf.inference.ui.PerformanceMeter;
import org.mlperf.inference.ui.SnackBarResultLayout;
import org.mlperf.inference.ui.TabSwitcher;
import org.mlperf.proto.DatasetConfig;
import org.mlperf.proto.MLPerfConfig;
import org.mlperf.proto.ModelConfig;
import org.mlperf.proto.TaskConfig;

/** Go Screen, and Screen when running profiling */
@SuppressWarnings({"unused", "SpellCheckingInspection", "deprecation"})
public class CalculatingActivity extends BaseActivity
    implements MiddleInterface.Callback, Handler.Callback, IdlingResource {

  public static final String AUTO_START = "AUTO_START";
  public static final String SHARE_RESULTS = "SHARE_RESULTS";
  public static final String SUMMARY_SCORE_PREF = "SUMMARY_SCORE";
  public static final String RESULTS_PREF = "RESULTS";
  public static final String CONFIG_PATH = "CONFIG_PATH";
  public static final String CONFIG_SDCARD_PATH =
      "/storage/emulated/0/mlperf_datasets/tasks_v2.pbtxt";
  public static final String CONFIG_DEFAULT_URL =
      "https://github.com/mlcommons/mobile_models/raw/main/v1_0/assets/tasks_v2.pbtxt";
  public static final String SETTINGS_PATH = "SETTINGS_PATH";

  private float summaryScore;
  public static final int STATE_LOADING = -1;
  public static final int STATE_IDLE = 0;
  public static final int STATE_RUNNING = 1;
  public static final int STATE_RESULTS = 2;
  public static final int STATE_EXITING = 3;
  private static final int MSG_PROGRESS = 1;
  private static final int MSG_COMPLETE = 2;
  private static final int BUFFER_SIZE = 4096;
  private static final int BUFFERS_COUNT_IN_ONE_MB = 1024 * 1024 / BUFFER_SIZE;
  private static final int MIN_RAM_SIZE_GB = 2;
  private static final double BYTES_IN_GB = 1000000000d;
  private static final float MAX_PERCENTAGE_NUMBER = 100.0f;
  private static final String TAG = "CalculatingActivity";
  private static final String PERCENTAGE_CHAR = "%";
  private static final int MAX_TASKS = 10;
  private static boolean isStatusViewsSetup = false;
  public static MiddleInterface middleInterface;
  private final ArrayList<ResultHolder> results = new ArrayList<>();
  private int state = STATE_IDLE;
  private int pendingState = STATE_IDLE;
  private LinearLayout itemContentView;
  private TextView resultsDescriptionText;
  private RecyclerView detailedResultsRecyclerView;
  private RecyclerView resultSummaryRecyclerView;
  private TabSwitcher tabSwitcher;
  private View downArrowScroll;
  private View testAgainButton;
  private View shareResultsButton;
  private LockableScrollView calculatingScrollView;
  private TextView detailedResultsLabel;
  private PerformanceMeter progressMeter;
  private View highlight1, highlight2;
  private View performanceDetailsArea;
  private Button startCalculatingButton;
  private TextView cancelButton;
  private Handler handler = null;
  private final ArrayList<ItemStatus> statuses = new ArrayList<>();
  private View progressContainer;
  private TextView progressMessage;
  private TextView dontCloseApptext;
  private TextView calculatingText;
  private DiagnosticBackgroundView diagnosticBackgroundView;
  private SharedPreferences sharedPref;
  private boolean modelIsAvailable = false;
  private boolean taskConfigIsAvailable = false;
  private boolean autoStart = false;
  private boolean displayPreviousResults = false;
  private boolean allowShareResults = false;
  private float previousScore = -1;
  private ArrayList<ResultHolder> previousResults = null;
  private TextView loadingContentText;
  private float mCircularRevealDistFillScreen = -1;
  private String runMode;
  private volatile IdlingResource.ResourceCallback idleCallback;

  private static long downloadedDatasetsSize = 0;
  private static long totalDatasetsSize = 0;
  private static int downloadedPercentage = 0;
  private static boolean benchmarksAreRunning = false;

  public static void start(Context context, boolean autoStart) {
    Intent intent = new Intent(context, CalculatingActivity.class);
    intent.putExtra(AUTO_START, autoStart);
    intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
    context.startActivity(intent);
  }

  public int getState() {
    return state;
  }

  public void setState(int _state) {
    Log.d(TAG, "setState " + _state);
    this.state = _state;
    onNewState(_state);
  }

  @Override
  public int[] getMenuOptions() {
    if (state == STATE_LOADING) {
      return null;
    } else {
      return getDefaultMenuOptions();
    }
  }

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_calculating);
    MLCtx.getInstance(this);
    setupViews();
    handler = new Handler();
    getSupportActionBar().setElevation(0);

    ActivityManager activityManager = (ActivityManager) getSystemService(Context.ACTIVITY_SERVICE);

    ActivityManager.MemoryInfo memoryInfo = new ActivityManager.MemoryInfo();

    activityManager.getMemoryInfo(memoryInfo);

    int totalMemoryGB = (int) Math.ceil(memoryInfo.totalMem / BYTES_IN_GB);

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
            + "\nRAM size (GB): "
            + totalMemoryGB
            + "\n\n************ FIRMWARE ************\n"
            + "SDK: "
            + Build.VERSION.SDK_INT
            + "\nRelease: "
            + Build.VERSION.RELEASE
            + "\nIncremental: "
            + Build.VERSION.INCREMENTAL
            + "\n\n";
    Log.d(TAG, sysReport);

    if (totalMemoryGB < MIN_RAM_SIZE_GB) {
      ErrorActivity.launchError(
          this,
          ErrorActivity.ERROR_TYPE_RAM_SIZE,
          getString(R.string.ramSizeError, MIN_RAM_SIZE_GB, totalMemoryGB));
      setState(STATE_EXITING);
      return;
    }

    handler = new Handler(this.getMainLooper(), this);

    String cacheDirName = MLPerfTasks.getCacheDirName();
    String currentAppVersion;
    try {
      currentAppVersion =
          getApplicationContext()
              .getPackageManager()
              .getPackageInfo(getApplicationContext().getPackageName(), 0)
              .versionName
              .concat(BuildTimeConstants.HASH);
    } catch (Exception e) {
      currentAppVersion = BuildTimeConstants.HASH;
    }

    Log.i(TAG, "Current application version is " + currentAppVersion);
    File cacheDir = new File(cacheDirName);
    String storedVersionFileName = cacheDirName + "/.version";
    File storedVersionFile = new File(storedVersionFileName);
    boolean arePrevResultsValid = true;
    boolean isVersionChanged = false;

    if (storedVersionFile.exists()) {
      String storedVersion;
      try {
        storedVersion = new String(Files.readAllBytes(Paths.get(storedVersionFileName)));
      } catch (Exception e) {
        Log.i(TAG, "Unable to read version from " + storedVersionFileName);
        storedVersion = "";
      }

      isVersionChanged = !storedVersion.equals(currentAppVersion);
      Log.i(TAG, "Version is changed: " + isVersionChanged);
      long diff = new Date().getTime() - cacheDir.lastModified();
      Log.i(TAG, "Stored version is: " + storedVersion);

      if (diff > Long.valueOf(AppConstants.MAX_FILE_AGE_IN_DAYS) * 24 * 60 * 60 * 1000
          && isVersionChanged) {
        try {
          FileUtils.forceDelete(cacheDir);
        } catch (Exception e) {
          Log.i(TAG, "Fail to remove old cache folder");
        }
        ;
        arePrevResultsValid = false;
      }
    } else {
      isVersionChanged = true;
    }
    cacheDir.mkdirs();
    if (isVersionChanged) {
      try {
        Files.write(Paths.get(storedVersionFileName), currentAppVersion.getBytes());
      } catch (Exception e) {
        Log.i(TAG, "Unable to store version to " + storedVersionFileName);
      }
    }

    String configIntentPath = getIntent().getStringExtra(CONFIG_PATH);
    File file = new File(CONFIG_SDCARD_PATH);
    String configSdcardPath = file.exists() ? CONFIG_SDCARD_PATH : "";
    MLPerfTasks.setConfigPath(configIntentPath, configSdcardPath, CONFIG_DEFAULT_URL);

    try {
      middleInterface = new MiddleInterface(this);
    } catch (UnsupportedDeviceException e) {
      e.printStackTrace();
      ErrorActivity.launchError(this, ErrorActivity.ERROR_TYPE_UNSUPPORTED, e.getMessage());
      setState(STATE_EXITING);
      return;
    }

    String settingsPath = getIntent().getStringExtra(SETTINGS_PATH);
    if (!TextUtils.isEmpty(settingsPath)) {
      middleInterface.loadSettingFromFile(settingsPath);
    }

    sharedPref = PreferenceManager.getDefaultSharedPreferences(this);

    if (getIntent().getBooleanExtra(AUTO_START, false)) {
      autoStart = true;
      pendingState = STATE_RUNNING;
    } else {
      previousScore = sharedPref.getFloat(SUMMARY_SCORE_PREF, -1);
      Set<String> previousResultsStrings =
          sharedPref.getStringSet(RESULTS_PREF, Collections.emptySet());

      if (previousScore != -1 && arePrevResultsValid && previousResultsStrings != null) {
        displayPreviousResults = true;
        state = STATE_RESULTS;
        pendingState = STATE_RESULTS;
        previousResults = new ArrayList<>();

        HashMap<String, Float> maxScores = new HashMap();
        ArrayList<Benchmark> benchmarks = middleInterface.getBenchmarks();
        for (Benchmark bm : benchmarks) {
          maxScores.put(bm.getId(), bm.getMaxScore());
        }
        float product = 1f;
        int count = 0;
        ResultHolder[] tmpArray = new ResultHolder[MAX_TASKS];
        for (String s : previousResultsStrings) {
          ResultHolder result = deserializeResult(s);

          // FIXME - Is it safe to call getOrder here? Has the tasks_v2.pbtxt been
          // downloaded yet? If not can this be deferred until we have loaded the file?
          int order = getOrder(result.getId());
          if (order < 0) continue;
          if (tmpArray[order] == null)
            tmpArray[order] = new ResultHolder(result.getTestName(), result.getId());
          String mode = result.getMode() == null ? "" : result.getMode();
          if (mode.equals(AppConstants.ACCURACY_MODE))
            tmpArray[order].setAccuracy(result.getAccuracy());
          else if (mode.equals(AppConstants.PERFORMANCE_MODE)
              || mode.equals(AppConstants.PERFORMANCE_LITE_MODE)) {
            tmpArray[order].setScore(result.getScore());
            product = product * maxScores.get(result.getId());
            count++;
          } else {
            tmpArray[order].setScore(result.getScore());
            tmpArray[order].setAccuracy(result.getAccuracy());
            product = product * maxScores.get(result.getId());
            count++;
          }
        }
        float geomean = Math.round(Math.pow(product, 1.0 / (float) (count)));
        Benchmark.setSummaryMaxScore(geomean);
        for (ResultHolder rh : tmpArray) {
          if (rh != null) previousResults.add(rh);
        }
      }
    }

    allowShareResults = Util.getUserShareConsent();
  }

  @Override
  protected void onPause() {
    super.onPause();
    handler.removeCallbacksAndMessages(null);

    if (state == STATE_RUNNING) {
      getSupportActionBar()
          .setShowHideAnimationEnabled(
              false); // this fixes the actionbar animation getting interrupted and not fully
      // showing
    }
  }

  @Override
  public void onRestart() {
    super.onRestart();
    if (state == STATE_EXITING) finish();
  }

  @Override
  public void onResume() {
    super.onResume();
    taskConfigIsAvailable = false;
    modelIsAvailable = false;
    if (state == STATE_EXITING) return;

    if (getIntent().getBooleanExtra(AppConstants.SUBMISSION_MODE, false)) {
      Log.i(TAG, "Using submission mode");
      Util.setSubmissionMode(this, true);
    }

    runMode = sharedPref.getString(AppConstants.RUN_MODE, AppConstants.PERFORMANCE_LITE_MODE);
    if (!autoStart) {
      setState(state);
    }
    getSupportActionBar().setShowHideAnimationEnabled(true);

    String configIntentPath = getIntent().getStringExtra(CONFIG_PATH);
    File file = new File(CONFIG_SDCARD_PATH);
    String configSdcardPath = file.exists() ? CONFIG_SDCARD_PATH : "";
    MLPerfTasks.setConfigPath(configIntentPath, configSdcardPath, CONFIG_DEFAULT_URL);

    // Checks if models are available.
    checkModelIsAvailable();

    // set share setting from SharingActivity extra
    if (getIntent().hasExtra(SHARE_RESULTS)) {
      allowShareResults = getIntent().getBooleanExtra(SHARE_RESULTS, false);
      Util.setUserShareConsent(this, allowShareResults);
      getIntent().removeExtra(SHARE_RESULTS);
    } else allowShareResults = Util.getUserShareConsent();

    if (displayPreviousResults) {
      displayPreviousResults = false;
      showResults(previousScore, previousResults);
    }
  }

  @Override
  public void onDestroy() {
    super.onDestroy();
    if (runMode.equals(AppConstants.TESTING_MODE)) return;
    // Kill current process so the app will be restart on a new process.
    android.os.Process.killProcess(android.os.Process.myPid());
  }

  private void setupViews() {
    Objects.requireNonNull(getSupportActionBar()).setElevation(0);
    resultsDescriptionText = findViewById(R.id.resultsDescriptionText);
    calculatingScrollView = findViewById(R.id.calculatingScrollView);
    progressMeter = findViewById(R.id.performanceMeter);
    itemContentView = findViewById(R.id.contentItemListView);
    startCalculatingButton = findViewById(R.id.startButton);
    highlight1 = findViewById(R.id.startButtonHighlight);
    highlight1.setAlpha(0f);
    highlight2 = findViewById(R.id.performanceMeterHighlight2);
    highlight2.setAlpha(0f);
    cancelButton = findViewById(R.id.cancelButton);
    progressContainer = findViewById(R.id.progressContainer);
    progressMessage = findViewById(R.id.progressMessage);
    performanceDetailsArea = findViewById(R.id.performanceDetailsArea);
    detailedResultsLabel = findViewById(R.id.detailedResultsLabel);
    detailedResultsRecyclerView = findViewById(R.id.detailedResultsRecyclerView);
    resultSummaryRecyclerView = findViewById(R.id.resultSummaryRecyclerView);
    downArrowScroll = findViewById(R.id.downArrowScroll);
    dontCloseApptext = findViewById(R.id.dontCloseAppText);
    calculatingText = findViewById(R.id.calculatingText);
    ConstraintLayout constraintLayout = findViewById(R.id.constraintLayout);
    testAgainButton = findViewById(R.id.testAgainButton);
    shareResultsButton = findViewById(R.id.shareResultsButton);
    loadingContentText = findViewById(R.id.loadingContent);
    ViewGroup.LayoutParams params = constraintLayout.getLayoutParams();
    params.height = getWindowHeight();
    constraintLayout.setLayoutParams(params);
    detailedResultsRecyclerView.setLayoutManager(
        new LinearLayoutManager(this) {
          @SuppressWarnings({"SameReturnValue"})
          @Override
          public boolean canScrollVertically() {
            return false;
          }
        });
    resultSummaryRecyclerView.setLayoutManager(
        new GridLayoutManager(this, 4) {
          @SuppressWarnings("SameReturnValue")
          @Override
          public boolean canScrollHorizontally() {
            return false;
          }

          @SuppressWarnings("SameReturnValue")
          @Override
          public boolean canScrollVertically() {
            return false;
          }
        });
    tabSwitcher = findViewById(R.id.tabSwitcher);
    tabSwitcher.setOnTabClickedListener(this::onTabSwitched);
    startCalculatingButton.setOnClickListener(this::startCalculating);
    cancelButton.setOnClickListener(
        new View.OnClickListener() {
          @Override
          public void onClick(View v) {
            cancelBenchmarks();
          }
        });
    downArrowScroll.setOnClickListener(
        (nil) -> calculatingScrollView.smoothScrollTo(0, (int) downArrowScroll.getY()));
    testAgainButton.setOnClickListener(this::startCalculating);
    shareResultsButton.setOnClickListener(this::shareResults);

    diagnosticBackgroundView = findViewById(R.id.diagnosticBackgroundView);
  }

  private void setupItemStatusViews() {
    if (isStatusViewsSetup == true) return;
    MLPerfConfig mlperfTasks = MLPerfTasks.getConfig();
    for (TaskConfig task : mlperfTasks.getTaskList()) {
      for (ModelConfig model : task.getModelList()) {
        String token = model.getId().substring(0, 2);
        if (middleInterface.hasBenchmark(model.getId())) {
          statuses.add(
              new ItemStatus(
                  task.getName(),
                  token.equals("IC") ? true : false,
                  getIcon(model.getId()),
                  token));
        }
      }
    }
    for (ItemStatus status : statuses) {
      MeasureItemUtil.addItemToParent(
          this, itemContentView, status, (view) -> onItemClicked((String) view.getTag()));
    }
    isStatusViewsSetup = true;
  }

  private void cancelBenchmarks() {
    Log.d(TAG, "cancelBenchmarks");
    setState(STATE_IDLE);
    if (idleCallback != null) idleCallback.onTransitionToIdle();
    stopCalculating();
    progressMeter.stop();
  }

  private int getWindowHeight() {
    Point point = new Point();
    getWindow().getWindowManager().getDefaultDisplay().getSize(point);
    int windowHeight = point.y;
    Resources r = getResources();
    int px =
        (int)
            TypedValue.applyDimension( // status bar probably
                TypedValue.COMPLEX_UNIT_DIP, 24, r.getDisplayMetrics());
    return windowHeight - px;
  }

  private void onItemClicked(String id) {
    SnackBarResultLayout.make(getWindow().getDecorView(), id).show();
  }

  public static String getReadableBenchmarkTitle(String benchmarkId) {
    MLPerfConfig mlperfTasks = MLPerfTasks.getConfig();
    for (TaskConfig task : mlperfTasks.getTaskList()) {
      for (ModelConfig model : task.getModelList()) {
        if (model.getId().equals(benchmarkId)) {
          return task.getName();
        }
      }
    }
    return benchmarkId;
  }

  public static int getIcon(String benchmarkId) {
    int icon = R.drawable.ic_image_classification;
    if (benchmarkId.startsWith("IS_")) {
      icon = R.drawable.ic_image_segmentation;
    } else if (benchmarkId.startsWith("OD_")) {
      icon = R.drawable.ic_object_detection;
    } else if (benchmarkId.startsWith("LU_")) {
      icon = R.drawable.ic_language_processing;
    }
    return icon;
  }

  public void clearUI() {
    invalidateOptionsMenu();
    detailedResultsLabel.setVisibility(View.GONE);
    detailedResultsRecyclerView.setVisibility(View.GONE);
    performanceDetailsArea.setVisibility(View.INVISIBLE);
    itemContentView.setVisibility(View.GONE);
    resultsDescriptionText.setVisibility(View.GONE);
    calculatingScrollView.setScrollEnabled(false);
    startCalculatingButton.setVisibility(View.GONE);
    cancelButton.setVisibility(View.GONE);
    progressMeter.setVisibility(View.INVISIBLE);
    progressMessage.setVisibility(View.GONE);
    tabSwitcher.setVisibility(View.INVISIBLE);
    dontCloseApptext.setVisibility(View.GONE);
    calculatingText.setVisibility(View.GONE);
    downArrowScroll.setVisibility(View.GONE);
    resultSummaryRecyclerView.setVisibility(View.GONE);
    loadingContentText.setVisibility(View.INVISIBLE);
  }

  private void updateCircle(boolean fullscreen, View view) {
    if (fullscreen) {
      view.getViewTreeObserver()
          .addOnPreDrawListener(
              new ViewTreeObserver.OnPreDrawListener() {
                @Override
                public boolean onPreDraw() {
                  view.getViewTreeObserver().removeOnPreDrawListener(this);
                  float bottom = diagnosticBackgroundView.getHeight() * 1.1f;
                  diagnosticBackgroundView.setFill(bottom);
                  return false;
                }
              });
    } else {
      view.getViewTreeObserver()
          .addOnPreDrawListener(
              new ViewTreeObserver.OnPreDrawListener() {
                @Override
                public boolean onPreDraw() {
                  view.getViewTreeObserver().removeOnPreDrawListener(this);
                  view.measure(0, 0);
                  view.getMeasuredHeight();
                  float offset =
                      view.getTop() - getResources().getDimension(R.dimen.measure_label_margin);
                  diagnosticBackgroundView.setFill(offset);
                  return false;
                }
              });
    }
  }

  private synchronized void onNewState(int state) {
    clearUI();
    switch (state) {
      case STATE_LOADING:
        highlight1.animate().cancel();
        highlight2.animate().cancel();
        highlight1.setAlpha(0);
        highlight2.setAlpha(0);

        testAgainButton.setVisibility(View.GONE);
        shareResultsButton.setVisibility(View.GONE);
        loadingContentText.setVisibility(View.VISIBLE);
        startCalculatingButton.setOnClickListener(null);
        startCalculatingButton.setBackground(
            ContextCompat.getDrawable(this, R.drawable.circle_disabled));
        startCalculatingButton.setEnabled(false);
        itemContentView.setVisibility(View.VISIBLE);
        resultsDescriptionText.setVisibility(View.GONE);
        calculatingScrollView.setScrollEnabled(false);
        startCalculatingButton.setText(
            ((Context) this)
                .getString(R.string.downloadedPercent, Integer.toString(downloadedPercentage)));
        startCalculatingButton.setTextColor(ContextCompat.getColor(this, R.color.white));
        startCalculatingButton.setVisibility(View.VISIBLE);
        progressContainer.setVisibility(View.GONE);
        cancelButton.setVisibility(View.INVISIBLE);

        // ensure bg fill is set up
        updateCircle(true, itemContentView);

        if (getSupportActionBar() != null) {
          getSupportActionBar().setTitle(R.string.calculatingActivityLabel);
          getSupportActionBar().show();
        }
        break;
      case STATE_IDLE:
        // Transition animations
        highlight1.animate().alpha(1f).start();
        highlight2.animate().alpha(0f).start();

        testAgainButton.setVisibility(View.GONE);
        shareResultsButton.setVisibility(View.GONE);
        highlight2.setVisibility(View.INVISIBLE);
        startCalculatingButton.setOnClickListener(this::startCalculating);
        startCalculatingButton.setBackground(
            ContextCompat.getDrawable(this, R.drawable.circle_gradient));
        startCalculatingButton.setText(getText(benchmarksAreRunning ? R.string.wait : R.string.go));
        startCalculatingButton.setTextColor(ContextCompat.getColor(this, R.color.ml_blue));
        startCalculatingButton.setEnabled(!benchmarksAreRunning);
        itemContentView.setVisibility(View.VISIBLE);
        resultsDescriptionText.setVisibility(View.VISIBLE);
        calculatingScrollView.setScrollEnabled(false);
        startCalculatingButton.setVisibility(View.VISIBLE);
        progressContainer.setVisibility(View.GONE);
        cancelButton.setVisibility(View.INVISIBLE);

        if (benchmarksAreRunning) {
          loadingContentText.setText(getText(R.string.waiting_for_benchmark_finish));
          loadingContentText.setVisibility(View.VISIBLE);
        }

        // Ensure bg fill is set up
        updateCircle(false, resultsDescriptionText);
        resultsDescriptionText.forceLayout();

        if (getSupportActionBar() != null && !benchmarksAreRunning) {
          getSupportActionBar().setTitle(R.string.calculatingActivityLabel);
          getSupportActionBar().show();
        } else {
          getSupportActionBar().hide();
        }
        break;
      case STATE_RUNNING:
        testAgainButton.setVisibility(View.GONE);
        shareResultsButton.setVisibility(View.GONE);
        highlight2.setVisibility(View.VISIBLE);
        calculatingScrollView.setScrollEnabled(false);

        // Transition animations
        highlight1.animate().alpha(0f).start();
        highlight2.animate().translationY(0).alpha(1f).start();

        progressMessage.setTranslationY(-32 * getResources().getDisplayMetrics().density);
        progressMessage.setAlpha(0);
        progressMessage.animate().translationY(0).alpha(1).start();
        progressMessage.setVisibility(View.VISIBLE);
        progressMeter.setTranslationY(-32 * getResources().getDisplayMetrics().density);
        progressMeter.setAlpha(0);
        progressMeter.animate().translationY(0).alpha(1).setStartDelay(66).start();

        progressMeter.setVisibility(View.VISIBLE);
        progressMeter.start();

        performanceDetailsArea.setVisibility(View.VISIBLE);

        cancelButton.setVisibility(View.VISIBLE);
        dontCloseApptext.setVisibility(View.VISIBLE);
        calculatingText.setVisibility(View.VISIBLE);
        progressContainer.setVisibility(View.VISIBLE);

        // ensure bg fill is set up
        updateCircle(true, diagnosticBackgroundView);

        if (getSupportActionBar() != null) getSupportActionBar().hide();
        break;
      case STATE_RESULTS:
        // set data
        detailedResultsRecyclerView.setAdapter(
            new DetailedResultItemAdapter(
                results, (view) -> onItemClicked((String) view.getTag())));

        resultSummaryRecyclerView.setAdapter(
            new ResultSummaryItemAdapter(results, tabSwitcher.getType()));

        // set visibility
        progressContainer.setVisibility(View.GONE);
        testAgainButton.setVisibility(View.VISIBLE);
        if (allowShareResults) shareResultsButton.setVisibility(View.VISIBLE);
        else shareResultsButton.setVisibility(View.GONE);
        progressMeter.setVisibility(View.VISIBLE);
        detailedResultsLabel.setVisibility(View.VISIBLE);
        resultSummaryRecyclerView.setVisibility(View.VISIBLE);
        tabSwitcher.setVisibility(View.VISIBLE);
        downArrowScroll.setVisibility(View.VISIBLE);

        calculatingScrollView.setScrollEnabled(true);

        resultSummaryRecyclerView.setTranslationY(
            getResources().getDimension(R.dimen.results_fill_margin_bottom));
        resultSummaryRecyclerView.setAlpha(0);

        // add slight delay before displaying detailedResultsRecyclerView
        // to avoid scrolling flash on repeat diagnostic tests
        handler.postDelayed(
            new Runnable() {
              @Override
              public void run() {
                detailedResultsRecyclerView.setVisibility(View.VISIBLE);
              }
            },
            50);

        // Summary score & dial display
        // Currently disabling score display
        // TODO MLCommons - re-enable if & else statements below to display summary score
        // if (summaryScore <= 0){
        //     // hide summary Score
        //     progressMessage.setVisibility(View.INVISIBLE);
        //     progressMeter.setResult(3, 5);

        //     // distribute dial & summary results vertically
        //     progressMeter.getViewTreeObserver().addOnPreDrawListener(new
        // ViewTreeObserver.OnPreDrawListener() {
        //       @Override
        //       public boolean onPreDraw() {
        //         progressMeter.getViewTreeObserver().removeOnPreDrawListener(this);

        //         float top = progressMeter.getTop() - tabSwitcher.getBottom();
        //         float bottom = resultSummaryRecyclerView.getTop() - progressMeter.getBottom();

        //         float offset = (top+bottom) / 4.5f;

        //         progressMeter.animate().translationY(-top+2*offset);
        //         highlight2.animate().translationY(-top+2*offset);
        //         resultSummaryRecyclerView.animate().translationY(-offset).alpha(1).start();

        //         return false;
        //       }
        //     });
        // // TODO MLCommons - additional code to enable for summary score
        // } else {
        //     progressMessage.setText(String.valueOf((int) summaryScore));

        //     // TODO: MLCommons team to link max score once available in api
        //     progressMeter.setResult(summaryScore, Benchmark.getSummaryMaxScore());
        //     // TODO: show summary score when agreement about calculation method for it will be
        // achieved
        //     progressMessage.setVisibility(View.VISIBLE);

        //
        // resultSummaryRecyclerView.animate().translationY(getResources().getDimension(R.dimen.summary_translation)).alpha(1).start();
        // }
        progressMessage.setVisibility(View.INVISIBLE);
        if (summaryScore <= 0) {
          progressMeter.setResult(3, 5);
        } else {
          progressMeter.setResult(summaryScore, Benchmark.getSummaryMaxScore());
        }
        progressMeter
            .getViewTreeObserver()
            .addOnPreDrawListener(
                new ViewTreeObserver.OnPreDrawListener() {
                  @Override
                  public boolean onPreDraw() {
                    progressMeter.getViewTreeObserver().removeOnPreDrawListener(this);

                    float top = progressMeter.getTop() - tabSwitcher.getBottom();
                    float bottom = resultSummaryRecyclerView.getTop() - progressMeter.getBottom();

                    float offset = (top + bottom) / 4.5f;

                    progressMeter.animate().translationY(-top + 2 * offset);
                    highlight2.animate().translationY(-top + 2 * offset);
                    resultSummaryRecyclerView.animate().translationY(-offset).alpha(1).start();

                    return false;
                  }
                });

        resultSummaryRecyclerView
            .getViewTreeObserver()
            .addOnPreDrawListener(
                new ViewTreeObserver.OnPreDrawListener() {
                  @Override
                  public boolean onPreDraw() {
                    resultSummaryRecyclerView.getViewTreeObserver().removeOnPreDrawListener(this);
                    // see which is lower, the results, or the default position for the fill
                    float defaultFill =
                        getWindowHeight() - 64 * getResources().getDisplayMetrics().density;
                    float summaryBottom =
                        resultSummaryRecyclerView.getBottom()
                            + 24 * getResources().getDisplayMetrics().density;

                    // check if we've drawn background before
                    if (mCircularRevealDistFillScreen == -1) {
                      // first draw
                      diagnosticBackgroundView.setFillImmediate(
                          Math.max(summaryBottom, defaultFill));
                      // also set up running state
                      mCircularRevealDistFillScreen = diagnosticBackgroundView.getHeight() * 1.1f;
                    } else {
                      diagnosticBackgroundView.setFill(Math.max(summaryBottom, defaultFill));
                    }

                    return false;
                  }
                });

        resultSummaryRecyclerView.setVisibility(View.VISIBLE);

        detailedResultsLabel
            .animate()
            .translationY(getResources().getDimension(R.dimen.detailed_translation))
            .start();
        downArrowScroll
            .animate()
            .translationY(getResources().getDimension(R.dimen.detailed_translation))
            .start();
        detailedResultsRecyclerView
            .animate()
            .translationY(getResources().getDimension(R.dimen.detailed_translation))
            .start();
        testAgainButton
            .animate()
            .translationY(getResources().getDimension(R.dimen.detailed_translation))
            .start();
        if (allowShareResults) {
          shareResultsButton
              .animate()
              .translationY(getResources().getDimension(R.dimen.detailed_translation))
              .start();
        }

        tabSwitcher.setVisibility(View.VISIBLE);

        if (getSupportActionBar() != null) {
          if (tabSwitcher.getType() == tabSwitcher.TAB_TYPE_PERFORMANCE) {
            getSupportActionBar().setTitle(R.string.resultsTitlePerformance);
          } else {
            getSupportActionBar().setTitle(R.string.resultsTitleAccuracy);
          }
          getSupportActionBar().show();
        }
        downArrowScroll.setVisibility(View.VISIBLE);
        break;
    }

    handler.post(
        () -> {
          calculatingScrollView.scrollTo(0, 0); // scroll to top when scrolling is disabled
        });
  }

  // call this to stop the calculating, and launch our error dialog
  private void onError() {
    ErrorActivity.launchError(this, ErrorActivity.ERROR_TYPE_UNKNOWN, null);
    stopCalculating();
  }

  private void shareResults(View v) {
    Intent shareIntent = Util.getShareResultsIntent(v.getContext());

    startActivity(Intent.createChooser(shareIntent, getResources().getText(R.string.sendTo)));
  }

  private void startCalculating(View v) {
    if (state == STATE_RUNNING) return; // already running
    calculating();
  }

  private void calculating() {
    if (modelIsAvailable || checkModelIsAvailable()) {
      getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
      try {
        middleInterface.runBenchmarks(runMode);
      } catch (Exception e) {
        ErrorActivity.launchError(
            (Context) CalculatingActivity.this,
            ErrorActivity.ERROR_TYPE_DATASET_CORRUPTION,
            e.getMessage());
      }

      setProgressText(0);

      setState(STATE_RUNNING);
      for (ItemStatus status : statuses) {
        status.value = false;
      }

    } else {
      Log.i(TAG, "Models are not available.");
    }
  }

  private boolean checkModelIsAvailable() {
    if (checkSelfPermission(android.Manifest.permission.WRITE_EXTERNAL_STORAGE)
        == PackageManager.PERMISSION_GRANTED) {
      return doCheckModelIsAvailable();
    } else {
      ActivityCompat.requestPermissions(
          this, new String[] {android.Manifest.permission.WRITE_EXTERNAL_STORAGE}, 1);
      return false;
    }
  }

  interface DownloadingProgressCallback {
    /**
     * Callback function which will be called when downloading percentage will be changed
     *
     * @param percentage number of current downloading percentage
     */
    void notify(int percentage);
  }

  private boolean doCheckModelIsAvailable() {

    if (!modelIsAvailable || !taskConfigIsAvailable) {
      DownloadingProgressCallback callback =
          new DownloadingProgressCallback() {
            public void notify(int percentage) {
              // TODO implement percentage control update/refresh
              runOnUiThread(
                  () -> {
                    if (CalculatingActivity.this.startCalculatingButton != null) {
                      CalculatingActivity.this.startCalculatingButton.setText(
                          ((Context) CalculatingActivity.this)
                              .getString(
                                  R.string.downloadedPercent,
                                  Integer.toString(downloadedPercentage)));
                    }
                  });
            }
          };
      if (!taskConfigIsAvailable) {
        String taskConfigPath = MLPerfTasks.getConfigPath();
        Log.i(TAG, "Task config path is " + taskConfigPath);
        boolean needExtractConfig = ExtractCommon.doesFileNeedExtract(taskConfigPath);
        if (needExtractConfig) {
          Log.i(TAG, "Task config needs extracting");
          if (state != STATE_LOADING) {
            setState(STATE_LOADING);
          }
          new ExtractTaskConfig(this, taskConfigPath, callback).execute();
          return false;
        }
        setTaskConfigIsAvailable();
      }
      Log.i(TAG, "Task config extracted");
      MLPerfConfig mlperfTasks = MLPerfTasks.getConfig();
      for (TaskConfig task : mlperfTasks.getTaskList()) {
        for (ModelConfig model : task.getModelList()) {
          if (!middleInterface.hasBenchmark(model.getId())) {
            continue;
          }
          String src = middleInterface.getBenchmark(model.getId()).getSrc();
          if (ExtractCommon.doesFileNeedExtract(src)) {
            if (state != STATE_LOADING) {
              setState(STATE_LOADING);
            }
            new ModelExtractTask(this, mlperfTasks, callback).execute();
            return false;
          }
        }

        List<DatasetConfig> ds = new ArrayList<DatasetConfig>();

        if (runMode.equals(AppConstants.TESTING_MODE)) {
          if (task.hasTestDataset()) {
            ds.add(task.getTestDataset());
          }
        } else {
          if (runMode.equals(AppConstants.PERFORMANCE_MODE)
              || runMode.equals(AppConstants.SUBMISSION_MODE)) {
            ds.add(task.getDataset());
          }
          if (task.hasLiteDataset()) {
            ds.add(task.getLiteDataset());
          }
        }

        for (DatasetConfig dataset : ds) {
          boolean needExtractDataset = ExtractCommon.doesFileNeedExtract(dataset.getPath());
          needExtractDataset =
              needExtractDataset
                  ? needExtractDataset
                  : runMode.equals(AppConstants.SUBMISSION_MODE)
                      && ExtractCommon.isFileNotAvailable(dataset.getPath());
          needExtractDataset =
              needExtractDataset
                  ? needExtractDataset
                  : dataset.hasGroundtruthSrc()
                      && ExtractCommon.doesFileNeedExtract(dataset.getGroundtruthSrc());

          if (needExtractDataset) {
            if (state != STATE_LOADING) {
              setState(STATE_LOADING);
            }
            new ModelExtractTask(this, mlperfTasks, callback).execute();
            return false;
          }
        }
      }

      setModelIsAvailable();
    }
    return true;
  }

  public void setTaskConfigIsAvailable() {
    taskConfigIsAvailable = true;
  }

  public void setModelIsAvailable() {
    modelIsAvailable = true;

    // Now that we have the set of tasks and benchmarks, list the runnable set
    setupItemStatusViews();

    if (autoStart) {
      autoStart = false;
      calculating();
    } else if (pendingState == STATE_IDLE || pendingState == STATE_RESULTS) {
      if (state != pendingState) setState(pendingState);
      if (idleCallback != null) idleCallback.onTransitionToIdle();
    }
  }

  private void stopCalculating() {
    for (ItemStatus status : statuses) {
      status.value = false;
    }

    middleInterface.abortBenchmarks();
  }

  void setProgressText(int progressText) {
    String message = getString(R.string.progressMessage, progressText + PERCENTAGE_CHAR);
    final SpannableString span = new SpannableString(message);
    span.setSpan(
        new RelativeSizeSpan(.3f),
        message.length() - 1,
        message.length(),
        Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
    progressMessage.setText(span, TextView.BufferType.SPANNABLE);
  }

  @Override
  public void onProgressUpdate(int percent) {
    Message message = handler.obtainMessage(MSG_PROGRESS, percent, 0);
    handler.sendMessage(message);
  }

  @Override
  public void onbenchmarkFinished(ResultHolder result) {
    Message message = handler.obtainMessage(MSG_COMPLETE, result);
    handler.sendMessage(message);
  }

  @Override
  public void onbenchmarkStarted(String benchmarkId) {
    benchmarksAreRunning = true;
    runOnUiThread(
        () -> {
          TextView title = findViewById(R.id.performanceDetailsTitle);
          title.setText(getReadableBenchmarkTitle(benchmarkId));

          ImageView icon = findViewById(R.id.performanceDetailsIcon);

          for (Benchmark bm : middleInterface.getBenchmarks()) {
            if (bm.getId().equals(benchmarkId)) {
              icon.setImageResource(bm.getIcon());
              break;
            }
          }
        });
  }

  @Override
  public void oncoolingStarted() {
    runOnUiThread(
        () -> {
          int cooldownMinutes = Util.getCooldownPause((Context) CalculatingActivity.this);
          calculatingText.setText(getText(R.string.cooldown));
          dontCloseApptext.setText(getString(R.string.cooldown_subtitle, cooldownMinutes));
        });
  }

  @Override
  public void oncoolingFinished() {
    runOnUiThread(
        () -> {
          calculatingText.setText(getText(R.string.calculating));
          dontCloseApptext.setText(getText(R.string.dontCloseApp));
        });
  }

  @Override
  public void onAllBenchmarksFinished(float summaryScore, ArrayList<ResultHolder> results) {
    benchmarksAreRunning = false;
    if (getState() == STATE_IDLE) {
      runOnUiThread(
          () -> {
            setState(STATE_IDLE);
          });
    } else {
      sharedPref.edit().putFloat(SUMMARY_SCORE_PREF, summaryScore).apply();

      Set<String> set = new HashSet<>();

      for (ResultHolder result : results) {
        set.add(serializeResult(result));
      }

      sharedPref.edit().putStringSet(RESULTS_PREF, set).apply();
      handler.post(() -> showResults(summaryScore, results));
    }
  }

  private int getOrder(String benchmarkId) {
    int order = 0;
    MLPerfConfig mlperfTasks = MLPerfTasks.getConfig();
    for (TaskConfig task : mlperfTasks.getTaskList()) {
      for (ModelConfig model : task.getModelList()) {
        if (benchmarkId.equals(model.getId())) {
          return order;
        }
      }
      order++;
    }
    return -1;
  }

  private void showResults(float summaryScore, ArrayList<ResultHolder> results) {
    this.results.clear();
    tabSwitcher.Reset();

    ArrayList<ResultHolder> finalResults = generateResults(results, runMode);
    ResultHolder[] tmpArray = new ResultHolder[MAX_TASKS];
    int index = 0;
    for (ResultHolder rh : finalResults) {
      String id = rh.getId();
      String mode = rh.getMode() == null ? "" : rh.getMode();
      int order = getOrder(id);
      if (mode.equals(AppConstants.PERFORMANCE_MODE)
          || mode.equals(AppConstants.PERFORMANCE_LITE_MODE)) {
        rh.setAccuracy("N/A");
        finalResults.set(index, rh);
      }
      index++;

      if (order < 0) continue;
      if (tmpArray[order] == null) tmpArray[order] = new ResultHolder(rh.getTestName(), id);
      if (mode.equals(AppConstants.ACCURACY_MODE)) tmpArray[order].setAccuracy(rh.getAccuracy());
      else if (mode.equals(AppConstants.PERFORMANCE_MODE)) tmpArray[order].setScore(rh.getScore());
      else {
        tmpArray[order].setScore(rh.getScore());
        tmpArray[order].setAccuracy(rh.getAccuracy());
      }
    }
    ArrayList<ResultHolder> displayedResults = new ArrayList<>();
    for (ResultHolder rh : tmpArray) {
      if (rh == null) continue;
      displayedResults.add(rh);
    }
    this.results.addAll(displayedResults);
    MLPerfTasks.resultsToFile(finalResults, runMode);

    this.summaryScore = summaryScore;
    pendingState = STATE_RESULTS;
    setState(STATE_RESULTS);
    if (idleCallback != null) idleCallback.onTransitionToIdle();
  }

  @Override
  public boolean handleMessage(Message inputMessage) {
    if (inputMessage.what != MSG_PROGRESS) return false;

    int percent = inputMessage.arg1;
    setProgressText(percent);
    return true;
  }

  private void onTabSwitched(int type) {
    if (getSupportActionBar() != null) {
      if (tabSwitcher.getType() == tabSwitcher.TAB_TYPE_PERFORMANCE) {
        getSupportActionBar().setTitle(R.string.resultsTitlePerformance);
      } else {
        getSupportActionBar().setTitle(R.string.resultsTitleAccuracy);
      }
    }
    ((ResultSummaryItemAdapter) resultSummaryRecyclerView.getAdapter()).setType(type);
    ((DetailedResultItemAdapter) detailedResultsRecyclerView.getAdapter()).setType(type);
  }

  public static class ExtractCommon extends AsyncTask<Void, String, Void> {
    protected final WeakReference<Context> contextRef;
    protected final Context context;
    protected boolean success = true;
    protected String error;
    protected final DownloadingProgressCallback callback;

    public ExtractCommon(Context context, DownloadingProgressCallback callback) {
      contextRef = new WeakReference<>(context);
      this.context = context;
      this.callback = callback;
    }

    @Override
    protected Void doInBackground(Void... voids) {
      return null;
    }

    @Override
    protected void onPreExecute() {
      Log.i(TAG, "Extracting files...");
    }

    @Override
    protected void onProgressUpdate(String... filenames) {
      Log.i(TAG, "Extracting " + filenames[0] + "...");
    }

    @Override
    protected void onPostExecute(Void result) {}

    public static boolean doesFileNeedExtract(String path) {
      if (new File(MLPerfTasks.getLocalPath(path)).canRead()) {
        return false;
      }
      return path.startsWith("http://") || path.startsWith("https://");
    }

    public static boolean isFileNotAvailable(String path) {
      try {
        File file = new File(MLPerfTasks.getLocalPath(path));

        if (!file.canRead()) return true;
        if (!file.isDirectory()) return false;
        try (DirectoryStream<Path> directoryStream = Files.newDirectoryStream(file.toPath())) {
          // return true if there is a file
          return !directoryStream.iterator().hasNext();
        }
      } catch (IOException e) {
        Log.e(TAG, "Error in file '" + path + "' availability checking: " + e.getMessage());
      }

      return true;
    }

    @SuppressWarnings("ResultOfMethodCallIgnored")
    protected boolean extractFileFail(String src) {
      String dest = MLPerfTasks.getLocalPath(src);
      File destFile = new File(dest);
      publishProgress(destFile.getName());
      destFile.getParentFile().mkdirs();
      // Extract to a temporary file first, so the app can detects if the extraction failed.
      File tmpFile = new File(dest + ".tmp");
      try {
        InputStream in;
        if (src.startsWith("http://") || src.startsWith("https://")) {
          Log.i(TAG, "Attempting to download: " + src);
          ConnectivityManager cm =
              (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
          @SuppressLint("MissingPermission")
          NetworkInfo activeNetwork = cm.getActiveNetworkInfo();
          boolean isConnected = activeNetwork != null && activeNetwork.isConnectedOrConnecting();
          if (!isConnected) {
            error = "Error: No network connected.";
            return true;
          }
          in = new URL(src).openStream();
        } else {
          in = new FileInputStream(src);
        }
        OutputStream out = new FileOutputStream(tmpFile, false);
        copyFile(in, out, callback);
        if (MLPerfTasks.isZipFile(src)) {
          if (!unZip(tmpFile, dest)) {
            tmpFile.delete();
            return true;
          }
          tmpFile.delete();
          Log.d(TAG, "Unzipped " + src + " to " + dest);
        } else {
          tmpFile.renameTo(destFile);
        }
      } catch (IOException e) {
        Log.e(TAG, "Failed to prepare file " + dest + ": " + e.getMessage());
        error = "Error: " + e.getMessage();
        return true;
      }

      Log.d(TAG, destFile.getName() + " are extracted.");
      return false;
    }

    protected void copyFile(InputStream in, OutputStream out, DownloadingProgressCallback callback)
        throws IOException {
      byte[] buffer = new byte[BUFFER_SIZE];
      int read;
      int throttleCounter = 0;
      int incrementSize = 0;

      while ((read = in.read(buffer)) != -1) {
        out.write(buffer, 0, read);
        incrementSize += read;
        throttleCounter++;
        // call callback for each downloaded 1Mb of data
        if (throttleCounter % BUFFERS_COUNT_IN_ONE_MB == 0) {
          if (incrementDownloadedAmount(incrementSize)) callback.notify(downloadedPercentage);
          incrementSize = 0;
        }
      }

      if (incrementSize != 0 && incrementDownloadedAmount(incrementSize))
        callback.notify(downloadedPercentage);
    }

    @SuppressWarnings("ResultOfMethodCallIgnored")
    protected boolean unZip(File inputFile, String dest) {
      InputStream is;
      ZipInputStream zis;
      File destFile = new File(dest);
      destFile.mkdirs();
      try {
        String filename;
        is = new FileInputStream(inputFile);
        zis = new ZipInputStream(new BufferedInputStream(is));
        ZipEntry ze;
        byte[] buffer = new byte[1024];
        int count;

        while ((ze = zis.getNextEntry()) != null) {
          filename = ze.getName();
          // Need to create directories if not exists.
          if (ze.isDirectory()) {
            File fmd = new File(dest, filename);
            fmd.mkdirs();
            continue;
          }

          FileOutputStream fOut = new FileOutputStream(new File(dest, filename));
          while ((count = zis.read(buffer)) != -1) {
            fOut.write(buffer, 0, count);
          }
          fOut.close();
          zis.closeEntry();
        }

        zis.close();
      } catch (IOException e) {
        Log.e(TAG, "Failed to unzip file " + dest + ".zip: " + e.getMessage());
        return false;
      }
      return true;
    }
  }

  // ExtractTaskConfig downloads the tasks_v2.pbtxt file
  public static class ExtractTaskConfig extends ExtractCommon {
    private Context context;
    private String taskConfig;
    private final SharedPreferences sharedPref;

    public ExtractTaskConfig(
        Context context, String taskConfig, DownloadingProgressCallback callback) {
      super(context, callback);
      this.context = context;
      this.taskConfig = taskConfig;
      sharedPref = PreferenceManager.getDefaultSharedPreferences(context);
    }

    @Override
    protected Void doInBackground(Void... voids) {
      if (doesFileNeedExtract(this.taskConfig)) {
        if (extractFileFail(this.taskConfig)) {
          success = false;
        }
      }
      return null;
    }

    @Override
    protected void onPostExecute(Void result) {
      if (success) {
        Log.i(TAG, "Files were extracted.");
        ((CalculatingActivity) contextRef.get()).setTaskConfigIsAvailable();
        downloadedPercentage = 0;
        callback.notify(downloadedPercentage);
        MLPerfConfig mlperfTasks = MLPerfTasks.getConfig();
        new ModelExtractTask(context, mlperfTasks, callback).execute();
      } else {
        Log.e(TAG, error);
        if (error.equals("Error: No network connected.")) {
          ErrorActivity.launchError(
              contextRef.get(), ErrorActivity.ERROR_TYPE_NETWORK_PROBLEMS, error);
        } else {
          ErrorActivity.launchError(
              contextRef.get(), ErrorActivity.ERROR_TYPE_DATASET_CORRUPTION, error);
        }
        ((CalculatingActivity) contextRef.get()).setState(STATE_EXITING);
        SharedPreferences sharedPref =
            PreferenceManager.getDefaultSharedPreferences(contextRef.get());
        SharedPreferences.Editor preferencesEditor = sharedPref.edit();
        preferencesEditor.clear();
        preferencesEditor.commit();
      }
    }
  }

  // ModelExtractTask copies or downloads files to their location (external storage) when
  // they're not available.
  public static class ModelExtractTask extends ExtractCommon {
    private final SharedPreferences sharedPref;
    private final MLPerfConfig mlperfTasks;

    public ModelExtractTask(
        Context context, MLPerfConfig mlperfTasks, DownloadingProgressCallback callback) {
      super(context, callback);
      this.mlperfTasks = mlperfTasks;
      sharedPref = PreferenceManager.getDefaultSharedPreferences(context);
    }

    @Override
    protected Void doInBackground(Void... voids) {
      String runMode =
          sharedPref.getString(AppConstants.RUN_MODE, AppConstants.PERFORMANCE_LITE_MODE);
      callback.notify(0);
      getDatasetSizeToDownload(runMode);
      for (TaskConfig task : mlperfTasks.getTaskList()) {
        // Extract model file.
        for (ModelConfig model : task.getModelList()) {
          if (!middleInterface.hasBenchmark(model.getId())) {
            continue;
          }
          String src = middleInterface.getBenchmark(model.getId()).getSrc();
          if (doesFileNeedExtract(src)) {
            if (extractFileFail(src)) {
              success = false;
            }
          }
        }

        // Extract dataset and groundtruth file.
        List<DatasetConfig> ds = new ArrayList<DatasetConfig>();

        if (runMode.equals(AppConstants.TESTING_MODE)) {
          if (task.hasTestDataset()) {
            ds.add(task.getTestDataset());
          } else {
            success = false;
            error = "Task '" + task.getName() + "' is missing the test dataset";
          }
        } else {
          if (runMode.equals(AppConstants.PERFORMANCE_MODE)
              || runMode.equals(AppConstants.SUBMISSION_MODE)) {
            ds.add(task.getDataset());
          }
          if (task.hasLiteDataset()) {
            ds.add(task.getLiteDataset());
          } else {
            success = false;
            error = "Task '" + task.getName() + "' has not lite dataset";
          }
        }

        for (DatasetConfig dataset : ds) {
          if (doesFileNeedExtract(dataset.getPath())) {
            if (extractFileFail(dataset.getPath())) {
              success = false;
            }
          } else {
            // Check if full dataset has any data files
            if (runMode.equals(AppConstants.SUBMISSION_MODE)
                && isFileNotAvailable(dataset.getPath())) {
              success = false;
              error = "Dataset is unavailable: " + dataset.getPath();
            }
          }
          if (runMode.equals(AppConstants.SUBMISSION_MODE)
              && doesFileNeedExtract(dataset.getGroundtruthSrc())) {
            if (extractFileFail(dataset.getGroundtruthSrc())) {
              success = false;
            }
          }
        }
      }
      downloadedDatasetsSize = 0;
      totalDatasetsSize = 0;
      downloadedPercentage = 0;
      callback.notify(downloadedPercentage);
      return null;
    }

    @SuppressWarnings("SameReturnValue")
    protected Void getDatasetSizeToDownload(String runMode) {
      for (TaskConfig task : mlperfTasks.getTaskList()) {
        for (ModelConfig model : task.getModelList()) {
          if (!middleInterface.hasBenchmark(model.getId())) {
            continue;
          }
          String src = middleInterface.getBenchmark(model.getId()).getSrc();
          if (doesFileNeedExtract(src)) totalDatasetsSize += getFileSize(src);
        }

        List<DatasetConfig> ds = new ArrayList<DatasetConfig>();

        if (runMode.equals(AppConstants.TESTING_MODE)) {
          if (task.hasTestDataset()) {
            ds.add(task.getTestDataset());
          }
        } else {
          if (runMode.equals(AppConstants.PERFORMANCE_MODE)
              || runMode.equals(AppConstants.SUBMISSION_MODE)) {
            ds.add(task.getDataset());
          }
          if (task.hasLiteDataset()) {
            ds.add(task.getLiteDataset());
          }
        }

        for (DatasetConfig dataset : ds) {
          if (doesFileNeedExtract(dataset.getPath())) {
            totalDatasetsSize += getFileSize(dataset.getPath());
          }
          if (doesFileNeedExtract(dataset.getGroundtruthSrc())) {
            totalDatasetsSize += getFileSize(dataset.getGroundtruthSrc());
          }
        }
      }
      return null;
    }

    private long getFileSize(String src) {
      long size;
      try {
        if (src.startsWith("http://") || src.startsWith("https://")) {
          ConnectivityManager cm =
              (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
          @SuppressLint("MissingPermission")
          NetworkInfo activeNetwork = cm.getActiveNetworkInfo();
          boolean isConnected = activeNetwork != null && activeNetwork.isConnectedOrConnecting();
          if (!isConnected) {
            error = "Error: No network connected.";
            return 0;
          }
          size = new URL(src).openConnection().getContentLengthLong();
        } else {
          size = new File(src).length();
        }
      } catch (IOException e) {
        Log.e(TAG, "Cannot get size of " + src + ": " + e.getMessage());
        return 0;
      }
      return size;
    }

    public static boolean doesFileNeedExtract(String path) {
      if (new File(MLPerfTasks.getLocalPath(path)).canRead()) {
        return false;
      }
      return path.startsWith("http://") || path.startsWith("https://");
    }

    @Override
    protected void onPreExecute() {
      Log.i(TAG, "Extracting missing files...");
    }

    @Override
    protected void onProgressUpdate(String... filenames) {
      Log.i(TAG, "Extracting " + filenames[0] + "...");
    }

    @Override
    protected void onPostExecute(Void result) {
      if (success) {
        Log.i(TAG, "All missing files are extracted.");
        ((CalculatingActivity) contextRef.get()).setModelIsAvailable();
      } else {
        Log.e(TAG, error);
        if (error.equals("Error: No network connected.")) {
          ErrorActivity.launchError(
              contextRef.get(), ErrorActivity.ERROR_TYPE_NETWORK_PROBLEMS, error);
        } else {
          ErrorActivity.launchError(
              contextRef.get(), ErrorActivity.ERROR_TYPE_DATASET_CORRUPTION, error);
        }
        ((CalculatingActivity) contextRef.get()).setState(STATE_EXITING);
        SharedPreferences sharedPref =
            PreferenceManager.getDefaultSharedPreferences(contextRef.get());
        SharedPreferences.Editor preferencesEditor = sharedPref.edit();
        preferencesEditor.clear();
        preferencesEditor.commit();
      }
    }
  }

  private ArrayList<ResultHolder> generateResults(ArrayList<ResultHolder> rawResults, String mode) {
    ArrayList<ResultHolder> resultHolders = new ArrayList<>();

    for (ResultHolder rh : rawResults) {
      ResultHolder tmp = new ResultHolder(rh.getTestName(), rh.getId());
      tmp.setScore(rh.getScore());
      if (mode.equals(AppConstants.SUBMISSION_MODE) || mode.equals(AppConstants.ACCURACY_MODE))
        tmp.setAccuracy(rh.getAccuracy());
      else tmp.setAccuracy("N/A");
      tmp.setMinSamples(rh.getMinSamples());
      tmp.setNumSamples(rh.getNumSamples());
      tmp.setMinDuration(rh.getMinDuration());
      tmp.setDuration(rh.getDuration());
      tmp.setMode(rh.getMode());
      resultHolders.add(tmp);
    }

    return resultHolders;
  }

  String serializeResult(ResultHolder result) {
    try {
      ByteArrayOutputStream arrayStream = new ByteArrayOutputStream();
      ObjectOutputStream objectStream = new ObjectOutputStream(arrayStream);
      objectStream.writeObject(result);
      objectStream.close();
      return Base64.encodeToString(arrayStream.toByteArray(), Base64.DEFAULT);
    } catch (Exception e) {
      Log.e(TAG, "Cannot serialize result: " + e.getMessage());
      return null;
    }
  }

  ResultHolder deserializeResult(String s) {
    try {
      byte[] data = Base64.decode(s, Base64.DEFAULT);
      ObjectInputStream ois = new ObjectInputStream(new ByteArrayInputStream(data));
      Object o = ois.readObject();
      ois.close();
      return (ResultHolder) o;
    } catch (Exception e) {
      Log.e(TAG, "Cannot deserialize result: " + e.getMessage());
      return null;
    }
  }

  public String getName() {
    return CalculatingActivity.class.getName();
  }

  public boolean isIdleNow() {
    return modelIsAvailable && state != STATE_RUNNING;
  }

  public void registerIdleTransitionCallback(IdlingResource.ResourceCallback resourceCallback) {
    idleCallback = resourceCallback;
  }

  /**
   * Function accepts number of downloaded bytes and return true if downloaded percentage is changed
   *
   * @param size number of downloaded bytes
   * @return true if downloaded percentage is changed
   */
  static boolean incrementDownloadedAmount(long size) {
    downloadedDatasetsSize += size;
    int newDownloadedPercentage =
        Math.round(MAX_PERCENTAGE_NUMBER * downloadedDatasetsSize / totalDatasetsSize);
    newDownloadedPercentage = Math.min(Math.round(MAX_PERCENTAGE_NUMBER), newDownloadedPercentage);
    if (newDownloadedPercentage != downloadedPercentage) {
      downloadedPercentage = newDownloadedPercentage;
      return true;
    }

    return false;
  }
}
