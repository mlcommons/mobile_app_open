package org.mlperf.inference.activities.settings;

import static org.mlperf.inference.activities.CalculatingActivity.middleInterface;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.CheckBox;
import android.widget.LinearLayout;
import android.widget.TextView;
import androidx.annotation.Nullable;
import org.mlperf.inference.Benchmark;
import org.mlperf.inference.R;
import org.mlperf.inference.Util;
import org.mlperf.inference.activities.BaseActivity;
import org.mlperf.inference.activities.CalculatingActivity;

public final class ConfigSettingsActivity extends BaseActivity {
  private static final String CONFIGURATION_SETTINGS = "configuration";

  @Nullable
  public int[] getMenuOptions() {
    return new int[HOME_UP_BUTTON_MENU_OPTION];
  }

  public void onCreate(@Nullable Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    this.setContentView(R.layout.config_settings_activity);
    getSupportActionBar().setDisplayHomeAsUpEnabled(true);
    getSupportActionBar().setHomeAsUpIndicator(R.drawable.ic_arrow_back);
  }

  @SuppressWarnings({"unused", "RedundantSuppression"})
  @Override
  protected void onResume() {
    super.onResume();

    LinearLayout container = findViewById(R.id.config_settings_container);

    // remove old views - keep top text and bottom button
    container.removeViews(1, container.getChildCount() - 2);

    String backendName =
        middleInterface.getCommonSetting(CONFIGURATION_SETTINGS).getValue().getName();

    for (Benchmark bm : middleInterface.getBenchmarks()) {
      String label =
          CalculatingActivity.getReadableBenchmarkTitle(this.getApplicationContext(), bm.getId());
      View v = getLayoutInflater().inflate(R.layout.config_item_panel, null);
      TextView title = v.findViewById(R.id.config_item_title);
      title.setText(label);

      TextView subtitle = v.findViewById(R.id.config_item_subtitle);
      subtitle.setText(backendName);

      v.setOnClickListener(
          onClick -> new ConfigurationItemSelectionActivity().start(this, bm.getId(), label));

      CheckBox cb = v.findViewById(R.id.config_item_checkbox);
      Context ctx = v.getContext();
      cb.setChecked(Util.getTestActivity(ctx, bm.getId()));
      cb.setOnCheckedChangeListener(
          (compoundButton, isChecked) -> {
            Util.setTestActivity(ctx, bm.getId(), isChecked);
          });

      container.addView(v, container.getChildCount() - 1);
    }
  }

  @SuppressWarnings({"unused", "RedundantSuppression"})
  public void onRunClicked(View v) {
    CalculatingActivity.start(ConfigSettingsActivity.this, true);
  }

  public final void start(Context context) {
    context.startActivity(new Intent(context, ConfigSettingsActivity.class));
  }
}
