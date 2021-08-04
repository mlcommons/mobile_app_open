package org.mlperf.inference.activities.settings;

import static org.mlperf.inference.activities.CalculatingActivity.middleInterface;

import android.content.Context;
import android.content.Intent;
import android.graphics.Typeface;
import android.os.Bundle;
import android.view.View;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;
import androidx.core.content.ContextCompat;
import androidx.core.content.res.ResourcesCompat;
import java.util.ArrayList;
import org.mlperf.inference.R;
import org.mlperf.inference.activities.BaseActivity;
import org.mlperf.inference.models.ConfigItem;
import org.mlperf.inference.models.ConfigItems;
import org.mlperf.proto.BenchmarkSetting;

public class ConfigurationItemSelectionActivity extends BaseActivity {

  static final String EXTRA_KEY = "CONFIG_KEY";
  static final String EXTRA_LABEL = "CONFIG_LABEL";
  private static final String CONFIGURATION_SETTINGS = "configuration";
  private static final String ACCELERATOR_SETTINGS = "accelerator";

  @Override
  public int[] getMenuOptions() {
    return new int[HOME_UP_BUTTON_MENU_OPTION];
  }

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_configuration_item_selection);
    setTitle(getIntent().getStringExtra(EXTRA_LABEL));
    getSupportActionBar().setDisplayHomeAsUpEnabled(true);
    getSupportActionBar().setHomeAsUpIndicator(R.drawable.ic_arrow_back);
  }

  @SuppressWarnings({"unused", "RedundantSuppression"})
  @Override
  protected void onResume() {
    super.onResume();
    draw();
  }

  void draw() {

    LinearLayout container = findViewById(R.id.config_options_container);
    container.removeAllViews();

    ConfigItems items = getItems();
    String selectedId = items.getSelectedId();

    for (ConfigItem item : items.options) {
      View v = getLayoutInflater().inflate(R.layout.config_option_panel, null);
      Context context = v.getContext();
      boolean selected = item.getId().equals(selectedId);

      ImageView bg = v.findViewById(R.id.config_option_bg);
      bg.setVisibility(selected ? View.VISIBLE : View.GONE);

      TextView title = v.findViewById(R.id.config_option_title);

      title.setText(context.getString(R.string.title_backend, item.title));
      title.setTextColor(
          !selected
              ? ContextCompat.getColor(this, R.color.ml_blue)
              : ContextCompat.getColor(this, R.color.white));

      Typeface typeface =
          ResourcesCompat.getFont(
              this, selected ? R.font.source_sans_pro_semi_bold : R.font.source_sans_pro_regular);
      title.setTypeface(typeface);

      TextView subtitle = v.findViewById(R.id.config_option_subtitle);
      subtitle.setText(context.getString(R.string.title_runtime, item.description));
      subtitle.setTextColor(
          !selected
              ? ContextCompat.getColor(this, R.color.ml_blue)
              : ContextCompat.getColor(this, R.color.white));
      title.setTypeface(typeface);

      TextView subtitle2 = v.findViewById(R.id.config_option_subtitle2);
      subtitle2.setText(context.getString(R.string.title_accelerator, item.accelerator));
      subtitle2.setTextColor(
          !selected
              ? ContextCompat.getColor(this, R.color.ml_blue)
              : ContextCompat.getColor(this, R.color.white));
      title.setTypeface(typeface);

      container.addView(v);
    }
  }

  private ConfigItems getItems() {
    String key = getIntent().getStringExtra(EXTRA_KEY);
    ArrayList<ConfigItem> options = new ArrayList<>();
    String backendName =
        middleInterface.getCommonSetting(CONFIGURATION_SETTINGS).getValue().getName();

    try {
      BenchmarkSetting bm = middleInterface.getBenchmark(key);
      options.add(
          new ConfigItem(
              backendName, bm.getConfiguration(), bm.getAcceleratorDesc(), "configuration"));
    } catch (Exception e) {
      e.printStackTrace();
    }

    return new ConfigItems(this, key, options);
  }

  @SuppressWarnings({"unused", "RedundantSuppression"})
  public void start(Context context, String key, String label) {
    Intent i = new Intent(context, ConfigurationItemSelectionActivity.class);
    i.putExtra(EXTRA_KEY, key);
    i.putExtra(EXTRA_LABEL, label);
    context.startActivity(i);
  }
}
