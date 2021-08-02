package org.mlperf.inference.activities.settings;

import android.os.Bundle;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import org.mlperf.inference.R;
import org.mlperf.inference.activities.BaseActivity;
import org.mlperf.inference.activities.CalculatingActivity;

public class DeveloperSettingsActivity extends BaseActivity {

  @Override
  public int[] getMenuOptions() {
    return new int[HOME_UP_BUTTON_MENU_OPTION];
  }

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_developer_settings);

    getSupportActionBar().setDisplayHomeAsUpEnabled(true);
    getSupportActionBar().setHomeAsUpIndicator(R.drawable.ic_arrow_back);

    Button b = findViewById(R.id.runDeveloperInputButton);
    EditText et = findViewById(R.id.inputDeveloperField);
    final TextView output = findViewById(R.id.developerLogsTextView);
    b.setOnClickListener(
        v -> {
          et.clearFocus();
          output.setText(runCommand(et.getText().toString()));
        });
  }

  private String runCommand(String text) {
    return CalculatingActivity.middleInterface.runDiagnostic(text);
  }
}
