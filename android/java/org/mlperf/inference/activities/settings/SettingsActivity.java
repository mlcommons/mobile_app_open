package org.mlperf.inference.activities.settings;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.SeekBar;
import android.widget.TextView;
import androidx.annotation.Nullable;
import androidx.appcompat.widget.AppCompatSeekBar;
import androidx.appcompat.widget.SwitchCompat;
import org.mlperf.inference.R;
import org.mlperf.inference.Util;
import org.mlperf.inference.activities.BaseActivity;

public final class SettingsActivity extends BaseActivity {

  @Nullable
  public int[] getMenuOptions() {
    return new int[HOME_UP_BUTTON_MENU_OPTION];
  }

  public void onCreate(@Nullable Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    this.setContentView(R.layout.activity_settings);

    getSupportActionBar().setDisplayHomeAsUpEnabled(true);
    getSupportActionBar().setHomeAsUpIndicator(R.drawable.ic_arrow_back);

    SwitchCompat switchView = (SwitchCompat) this.findViewById(R.id.sharingSettingsToggle);
    switchView.setChecked(Util.getUserShareConsent());
    //noinspection Convert2Lambda
    switchView.setOnCheckedChangeListener(
        (OnCheckedChangeListener)
            (new OnCheckedChangeListener() {
              public final void onCheckedChanged(CompoundButton $noName_0, boolean isChecked) {
                Util.setUserShareConsent((Context) SettingsActivity.this, isChecked);
              }
            }));

    @SuppressWarnings("SpellCheckingInspection")
    SwitchCompat cooldownSwitch = this.findViewById(R.id.cooldownSettingToggle);
    AppCompatSeekBar cooldownSlider = this.findViewById(R.id.cooldownSlider);
    TextView cooldownSubtitle = this.findViewById(R.id.cooldown_item_subtitle);
    int cooldownMinutes = Util.getCooldownPause((Context) SettingsActivity.this);
    boolean cooldownIsEnabled = Util.getCooldown();

    cooldownSubtitle.setText(getString(R.string.cooldown_subtitle, cooldownMinutes));
    cooldownSwitch.setChecked(cooldownIsEnabled);
    cooldownSlider.setProgress(cooldownMinutes);
    cooldownSlider.setEnabled(cooldownIsEnabled);
    cooldownSwitch.setOnCheckedChangeListener(
        (buttonView, isChecked) -> {
          Util.setCooldown(isChecked);
          cooldownSlider.setEnabled(isChecked);
        });
    cooldownSlider.setOnSeekBarChangeListener(
        new AppCompatSeekBar.OnSeekBarChangeListener() {
          public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
            cooldownSubtitle.setText(getString(R.string.cooldown_subtitle, progress));
            Util.setCooldownPause((Context) SettingsActivity.this, progress);
          }

          public void onStartTrackingTouch(SeekBar seekBar) {
            // TODO Auto-generated method stub
          }

          public void onStopTrackingTouch(SeekBar seekBar) {
            // TODO Auto-generated method stub
          }
        });

    SwitchCompat submissionModeSwitch = this.findViewById(R.id.submissionModeToggle);
    submissionModeSwitch.setChecked(Util.getSubmissionMode((Context) SettingsActivity.this));
    submissionModeSwitch.setOnCheckedChangeListener(
        (buttonView, isChecked) ->
            Util.setSubmissionMode((Context) SettingsActivity.this, isChecked));

    //noinspection Convert2Lambda
    (this.findViewById(R.id.developerToolButton))
        .setOnClickListener(
            (OnClickListener)
                (new OnClickListener() {
                  public final void onClick(View it) {
                    SettingsActivity.this.startActivity(
                        new Intent(
                            (Context) SettingsActivity.this, DeveloperSettingsActivity.class));
                  }
                }));
    //noinspection Convert2Lambda
    (this.findViewById(R.id.privacyPolicyButton))
        .setOnClickListener(
            (OnClickListener)
                (new OnClickListener() {
                  public final void onClick(View it) {
                    SettingsActivity.this.startActivity(
                        (new Intent("android.intent.action.VIEW"))
                            .setData(
                                Uri.parse(
                                    SettingsActivity.this.getString(R.string.privacyPolicyUrl))));
                  }
                }));
    //noinspection Convert2Lambda
    (this.findViewById(R.id.eulaButton))
        .setOnClickListener(
            (OnClickListener)
                (new OnClickListener() {
                  public final void onClick(View it) {
                    SettingsActivity.this.startActivity(
                        (new Intent("android.intent.action.VIEW"))
                            .setData(Uri.parse(SettingsActivity.this.getString(R.string.eulaUrl))));
                  }
                }));
  }

  public static void start(Context context) {
    context.startActivity(new Intent(context, SettingsActivity.class));
  }
}
