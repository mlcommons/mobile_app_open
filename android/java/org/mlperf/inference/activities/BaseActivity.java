package org.mlperf.inference.activities;

import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.Window;
import android.view.WindowManager;
import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import org.mlperf.inference.R;
import org.mlperf.inference.activities.settings.ConfigSettingsActivity;
import org.mlperf.inference.activities.settings.SettingsActivity;

/** Base class to handle logic shared by most or all activities */
public abstract class BaseActivity extends AppCompatActivity {
  /**
   * Get options in list to show for this activity. Returns IDs of menuItems to show, or null to
   * hide menu
   */
  public int[] getMenuOptions() {
    return new int[0];
  }

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    int[] options = getMenuOptions();
    if (options != null) {
      for (int option : options) {
        if (option == BaseActivity.HOME_UP_BUTTON_MENU_OPTION) {
          if (getSupportActionBar() != null) getSupportActionBar().setDisplayHomeAsUpEnabled(true);
          getSupportActionBar().setHomeAsUpIndicator(R.drawable.ic_arrow_back);
        }
      }
    }
  }

  @Override
  public boolean onCreateOptionsMenu(Menu menu) {
    int[] options = getMenuOptions();
    if (options == null) {
      return super.onCreateOptionsMenu(menu); // don't inflate if no items shown
    }
    getMenuInflater().inflate(R.menu.base_activity_menu, menu);
    for (int option : options) {
      MenuItem item = menu.findItem(option);
      if (item != null) menu.findItem(option).setVisible(true);
    }
    return true;
  }

  @Override
  public boolean onOptionsItemSelected(@NonNull MenuItem item) {

    if (item.getItemId() == R.id.settingsMenuItem) SettingsActivity.start(this);
    else if (item.getItemId() == R.id.configMenuItem) new ConfigSettingsActivity().start(this);
    else if (item.getItemId() == HOME_UP_BUTTON_MENU_OPTION) onBackPressed();

    return super.onOptionsItemSelected(item);
  }

  public static int[] getDefaultMenuOptions() {
    int[] options = new int[2];
    options[0] = R.id.settingsMenuItem;
    options[1] = R.id.configMenuItem;
    return options;
  }

  public static final int HOME_UP_BUTTON_MENU_OPTION = android.R.id.home;

  public void setStatusBarColor(int color) {
    Window window = getWindow();
    window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);

    window.clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
    window.setStatusBarColor(color);
  }
}
