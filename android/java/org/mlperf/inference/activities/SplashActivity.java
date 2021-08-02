package org.mlperf.inference.activities;

import android.Manifest;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.provider.Settings;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import androidx.annotation.NonNull;
import androidx.appcompat.app.AlertDialog;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import org.mlperf.inference.BuildTimeConstants;
import org.mlperf.inference.MLCtx;
import org.mlperf.inference.R;

public class SplashActivity extends BaseActivity {

  private Handler handler = null;

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_splash);
    MLCtx.getInstance(this);

    setStatusBarColor(getColor(R.color.white));

    // Set version text
    PackageManager manager = getApplicationContext().getPackageManager();
    try {
      PackageInfo info = manager.getPackageInfo(getApplicationContext().getPackageName(), 0);
      ((TextView) findViewById(R.id.versionTextView))
          .setText(getString(R.string.versionLabel, info.versionName));
    } catch (PackageManager.NameNotFoundException e) {
      // hide the version text
      findViewById(R.id.versionTextView).setVisibility(View.INVISIBLE);
    }
    // Set hash text
    if (BuildTimeConstants.HASH.length() == 0) {
      findViewById(R.id.hashTextView).setVisibility(View.INVISIBLE);
    } else {
      ((TextView) findViewById(R.id.hashTextView)).setText("hash: " + BuildTimeConstants.HASH);
    }
    handler = new Handler(this.getMainLooper());
  }

  @Override
  protected void onResume() {
    super.onResume();
    if (ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE)
        != PackageManager.PERMISSION_GRANTED) {

      boolean show =
          shouldShowRequestPermissionRationale(Manifest.permission.WRITE_EXTERNAL_STORAGE);

      if (show) {
        showPermissionDialog();
      } else requestPermission();

    } else {
      continueLaunch();
    }
  }

  @Override
  protected void onPause() {
    super.onPause();
    handler.removeCallbacksAndMessages(null);
  }

  void showPermissionDialog() {
    boolean show = shouldShowRequestPermissionRationale(Manifest.permission.WRITE_EXTERNAL_STORAGE);

    AlertDialog.Builder alertBuilder = new AlertDialog.Builder(this);
    alertBuilder.setCancelable(true);
    alertBuilder.setTitle(getString(R.string.storage_permission));
    alertBuilder.setMessage(getString(R.string.storage_justification));
    if (!show)
      alertBuilder.setPositiveButton(
          getString(R.string.allow_in_settings), (dialog, which) -> openSettings());
    else
      alertBuilder.setPositiveButton(
          getString(R.string.allow_permission),
          (dialog, which) -> {
            dialog.cancel();
            requestPermission();
          });
    alertBuilder.setNegativeButton(getString(R.string.exit_app), (dialog, which) -> finish());

    AlertDialog alert = alertBuilder.create();
    alert.setCanceledOnTouchOutside(false);
    alert.setCancelable(false);
    alert.show();

    Button buttonPositive = alert.getButton(DialogInterface.BUTTON_POSITIVE);
    buttonPositive.setTextColor(ContextCompat.getColor(this, R.color.mlperfBlue));
  }

  void openSettings() {
    Intent i = new Intent();
    i.setAction(Settings.ACTION_APPLICATION_DETAILS_SETTINGS);
    i.addCategory(Intent.CATEGORY_DEFAULT);
    i.setData(Uri.parse("package:" + this.getPackageName()));
    i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
    i.addFlags(Intent.FLAG_ACTIVITY_NO_HISTORY);
    i.addFlags(Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
    this.startActivity(i);
    finish();
  }

  void requestPermission() {
    ActivityCompat.requestPermissions(
        SplashActivity.this, new String[] {Manifest.permission.WRITE_EXTERNAL_STORAGE}, 1);
  }

  @Override
  public void onRequestPermissionsResult(
      int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
    super.onRequestPermissionsResult(requestCode, permissions, grantResults);

    if (grantResults[0] == 0) continueLaunch();
    else showPermissionDialog();
  }

  void continueLaunch() {
    handler.postDelayed(
        new Runnable() {
          @Override
          public void run() {
            // TODO: this part of code was commented as solution for the issue #541
            // default "share_results" value was set to "false" - cpp/backends/tflite_settings.h
            //
            //            if (Util.hasUserConsented(SplashActivity.this)) {
            startActivity(new Intent(SplashActivity.this, CalculatingActivity.class));
            //            } else { // no user consent either way yet. Show them the dialog
            //              startActivity(new Intent(SplashActivity.this, SharingActivity.class));
            //            }
            finish();
          }
        },
        2000);
  }
}
