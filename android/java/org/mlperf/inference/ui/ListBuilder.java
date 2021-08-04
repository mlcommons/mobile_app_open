package org.mlperf.inference.ui;

import android.content.Context;
import android.util.AttributeSet;
import android.widget.LinearLayout;
import androidx.annotation.Nullable;

public class ListBuilder extends LinearLayout {
  public ListBuilder(Context context) {
    super(context);
  }

  public ListBuilder(Context context, @Nullable AttributeSet attrs) {
    super(context, attrs);
  }

  public ListBuilder(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
    super(context, attrs, defStyleAttr);
  }

  @SuppressWarnings({"unused", "RedundantSuppression"})
  public ListBuilder(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
    super(context, attrs, defStyleAttr, defStyleRes);
  }
}
