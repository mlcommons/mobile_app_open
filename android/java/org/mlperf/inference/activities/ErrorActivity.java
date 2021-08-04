package org.mlperf.inference.activities;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.text.method.ScrollingMovementMethod;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import java.util.HashMap;
import org.mlperf.inference.R;
import org.mlperf.inference.activities.settings.ConfigSettingsActivity;

public class ErrorActivity extends BaseActivity {

  private Button primaryButton;
  private Button secondaryButton;
  private TextView errorText;
  private TextView errorSubtitle;

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_error);

    setStatusBarColor(getColor(R.color.statusBarColor));

    errorText = findViewById(R.id.errorText);
    errorSubtitle = findViewById(R.id.errorSubtitle);
    primaryButton = findViewById(R.id.primaryButton);
    secondaryButton = findViewById(R.id.secondaryButton);
    setLayoutType(getIntent().getIntExtra(KEY_ERROR_TYPE, ERROR_TYPE_UNKNOWN));
  }

  private void setLayoutType(int type) {
    secondaryButton.setOnClickListener(
        new View.OnClickListener() {
          @Override
          public void onClick(View v) {
            reportBug();
          }
        });
    switch (type) {
      case ERROR_TYPE_UNKNOWN:
        errorText.setText(getText(R.string.genericError));
        errorSubtitle.setText("");
        primaryButton.setText(getText(R.string.testAgain));
        primaryButton.setOnClickListener(
            new View.OnClickListener() {
              @Override
              public void onClick(View v) {
                // test again
                CalculatingActivity.start(ErrorActivity.this, true);
                finish();
              }
            });
        primaryButton.setVisibility(View.VISIBLE);
        secondaryButton.setVisibility(View.VISIBLE);
        break;
      case ERROR_TYPE_CONFIG:
        errorText.setText(getText(R.string.genericError));
        errorSubtitle.setText(getText(R.string.configError));
        primaryButton.setText(getText(R.string.modify_config));
        final Context ctx = this;
        primaryButton.setOnClickListener(
            new View.OnClickListener() {
              @Override
              public void onClick(View v) {
                new ConfigSettingsActivity().start(ctx);
                finish();
              }
            });
        primaryButton.setVisibility(View.VISIBLE);
        secondaryButton.setVisibility(View.VISIBLE);
        break;
      case ERROR_TYPE_UNSUPPORTED:
        errorText.setText(getIntent().getStringExtra(KEY_ERROR_DETAILS));
        errorText.setMovementMethod(ScrollingMovementMethod.getInstance());
        errorSubtitle.setText(getText(R.string.error_try_another_device));
        primaryButton.setText(getText(R.string.quit));
        primaryButton.setOnClickListener(
            new View.OnClickListener() {
              @Override
              public void onClick(View v) {
                finishAffinity();
              }
            });
        primaryButton.setVisibility(View.VISIBLE);
        secondaryButton.setVisibility(View.GONE);
        break;
      case ERROR_TYPE_RAM_SIZE:
        errorText.setText(getIntent().getStringExtra(KEY_ERROR_DETAILS));
        errorSubtitle.setText(getText(R.string.error_try_another_device));
        primaryButton.setText(getText(R.string.quit));
        primaryButton.setOnClickListener(
            new View.OnClickListener() {
              @Override
              public void onClick(View v) {
                finishAffinity();
              }
            });
        primaryButton.setVisibility(View.VISIBLE);
        secondaryButton.setVisibility(View.GONE);
        break;
      case ERROR_TYPE_DATASET_CORRUPTION:
        errorText.setText(getIntent().getStringExtra(KEY_ERROR_DETAILS));
        errorSubtitle.setText(getText(R.string.error_try_restart_application));
        primaryButton.setText(getText(R.string.quit));
        primaryButton.setOnClickListener(
            new View.OnClickListener() {
              @Override
              public void onClick(View v) {
                finishAffinity();
              }
            });
        primaryButton.setVisibility(View.VISIBLE);
        secondaryButton.setVisibility(View.GONE);
        break;
      case ERROR_TYPE_NETWORK_PROBLEMS:
        errorText.setText(getIntent().getStringExtra(KEY_ERROR_DETAILS));
        errorSubtitle.setText(getText(R.string.error_try_restart_application));
        primaryButton.setText(getText(R.string.tryAgain));
        secondaryButton.setText(getText(R.string.quit));
        primaryButton.setOnClickListener(
            new View.OnClickListener() {
              @Override
              public void onClick(View v) {
                // try again
                CalculatingActivity.start(ErrorActivity.this, false);
              }
            });
        primaryButton.setVisibility(View.VISIBLE);
        secondaryButton.setVisibility(View.VISIBLE);
        secondaryButton.setOnClickListener(
            new View.OnClickListener() {
              @Override
              public void onClick(View v) {
                finishAffinity();
              }
            });
        break;
    }
  }

  @Override
  public void onBackPressed() {
    super.onBackPressed();
  }

  private void reportBug() {

    String url = getString(R.string.reportBugUrl);

    // TODO: MLCommons - define parameters for form submission
    // syntax: <form-url>?<key-1>=<value-1>&<key-2>=<value-2>
    HashMap<String, String> params = new HashMap<>();
    params.put("type", getIntent().getIntExtra(KEY_ERROR_TYPE, -1) + "");
    params.put("details", getIntent().getStringExtra(KEY_ERROR_DETAILS));

    if (params.size() > 0) url = url + "?";

    for (HashMap.Entry<String, String> param : params.entrySet()) {
      url = url + param.getKey() + "=" + param.getValue() + "&";
    }

    url = url.substring(0, url.lastIndexOf("&"));

    this.startActivity((new Intent("android.intent.action.VIEW")).setData(Uri.parse(url)));
  }

  public static final String KEY_ERROR_TYPE = "ERROR_TYPE_CONFIG";
  public static final String KEY_ERROR_DETAILS = "ERROR_DETAILS";
  public static final int ERROR_TYPE_UNKNOWN = 0;
  public static final int ERROR_TYPE_CONFIG = 1;
  public static final int ERROR_TYPE_UNSUPPORTED = 2;
  public static final int ERROR_TYPE_RAM_SIZE = 3;
  public static final int ERROR_TYPE_DATASET_CORRUPTION = 4;
  public static final int ERROR_TYPE_NETWORK_PROBLEMS = 5;

  /**
   * Launch error activity.
   *
   * <p>TODO: MLCommons You may want to supply different data other than a string for errorDetails.
   * Update as needed
   */
  public static void launchError(Context context, int errorType, String errorDetails) {
    Intent intent = new Intent(context, ErrorActivity.class);
    intent.putExtra(KEY_ERROR_TYPE, errorType);
    intent.putExtra(KEY_ERROR_DETAILS, errorDetails);
    context.startActivity(intent);
  }
}
