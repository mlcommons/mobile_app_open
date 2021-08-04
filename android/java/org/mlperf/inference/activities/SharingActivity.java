package org.mlperf.inference.activities;

import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import androidx.annotation.Nullable;
import org.mlperf.inference.R;

public final class SharingActivity extends BaseActivity {

  public void onCreate(@Nullable Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    this.setContentView(R.layout.activity_sharing);

    setStatusBarColor(getColor(R.color.white));

    (this.findViewById(R.id.disallowSharingButton))
        .setOnClickListener((v -> SharingActivity.this.setConsentAndLaunch(false)));
    (this.findViewById(R.id.allowSharingButton))
        .setOnClickListener((v -> SharingActivity.this.setConsentAndLaunch(true)));
    (this.findViewById(R.id.privacyPolicyButton))
        .setOnClickListener(
            v -> {
              this.startActivity(
                  (new Intent("android.intent.action.VIEW"))
                      .setData(Uri.parse(this.getString(R.string.privacyPolicyUrl))));
            });
  }

  private final void setConsentAndLaunch(boolean b) {
    Intent i = new Intent(this, CalculatingActivity.class);
    i.putExtra(CalculatingActivity.SHARE_RESULTS, b);
    this.startActivity(i);
    this.finish();
  }
}
