package org.mlperf.inference.ui;

import android.content.Context;
import android.graphics.Typeface;
import android.transition.TransitionManager;
import android.util.AttributeSet;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.TextView;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.constraintlayout.widget.ConstraintLayout;
import androidx.constraintlayout.widget.ConstraintSet;
import androidx.core.content.res.ResourcesCompat;
import org.mlperf.inference.R;

public class TabSwitcher extends LinearLayout implements View.OnClickListener {

  private OnTabClickedListener onTabClickedListener;

  public TabSwitcher(@NonNull Context context) {
    super(context);
    inflate(context, R.layout.tab_switcher, this);
    findViewById(R.id.performanceButton).setOnClickListener(this);
    findViewById(R.id.accuracyButton).setOnClickListener(this);
  }

  public TabSwitcher(@NonNull Context context, @Nullable AttributeSet attrs) {
    super(context, attrs);
    inflate(context, R.layout.tab_switcher, this);
    findViewById(R.id.performanceButton).setOnClickListener(this);
    findViewById(R.id.accuracyButton).setOnClickListener(this);
  }

  public TabSwitcher(@NonNull Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
    super(context, attrs, defStyleAttr);
    inflate(context, R.layout.tab_switcher, this);
    findViewById(R.id.performanceButton).setOnClickListener(this);
    findViewById(R.id.accuracyButton).setOnClickListener(this);
  }

  public TabSwitcher(
      @NonNull Context context, @Nullable AttributeSet attrs, int defStyleAttr, int defStyleRes) {
    super(context, attrs, defStyleAttr, defStyleRes);
    inflate(context, R.layout.tab_switcher, this);
    findViewById(R.id.performanceButton).setOnClickListener(this);
    findViewById(R.id.accuracyButton).setOnClickListener(this);
  }

  @Override
  public void onClick(View v) {
    if (v.getId() == R.id.performanceButton) showTab(TAB_TYPE_PERFORMANCE);
    else if (v.getId() == R.id.accuracyButton) showTab(TAB_TYPE_ACCURACY);
    else showTab(TAB_TYPE_DEFAULT);
  }

  public void showTab(int type) {
    ConstraintLayout layout = findViewById(R.id.tabSwitcherRoot);
    ConstraintSet constraintSetTo = new ConstraintSet();
    constraintSetTo.clone(layout);

    TextView perf = findViewById(R.id.performanceButton);
    TextView acc = findViewById(R.id.accuracyButton);

    Typeface sb = ResourcesCompat.getFont(getContext(), R.font.source_sans_pro_semi_bold);
    Typeface reg = ResourcesCompat.getFont(getContext(), R.font.source_sans_pro_regular);

    if (type == TAB_TYPE_PERFORMANCE) {
      perf.setTypeface(sb);
      acc.setTypeface(reg);
      // anchor the underline to the bottom of the performance button, centered
      constraintSetTo.connect(
          R.id.underlineView, ConstraintSet.START, R.id.performanceButton, ConstraintSet.START);
      constraintSetTo.connect(
          R.id.underlineView, ConstraintSet.END, R.id.performanceButton, ConstraintSet.END);
      constraintSetTo.connect(
          R.id.underlineView, ConstraintSet.TOP, R.id.performanceButton, ConstraintSet.BOTTOM);
      setType(TAB_TYPE_PERFORMANCE);
    } else if (type == TAB_TYPE_ACCURACY) {
      perf.setTypeface(reg);
      acc.setTypeface(sb);
      // anchor the underline to the bottom of the accuracy button, centered
      constraintSetTo.connect(
          R.id.underlineView, ConstraintSet.START, R.id.accuracyButton, ConstraintSet.START);
      constraintSetTo.connect(
          R.id.underlineView, ConstraintSet.END, R.id.accuracyButton, ConstraintSet.END);
      constraintSetTo.connect(
          R.id.underlineView, ConstraintSet.TOP, R.id.accuracyButton, ConstraintSet.BOTTOM);
      setType(TAB_TYPE_ACCURACY);
    }

    TransitionManager.beginDelayedTransition(layout);
    constraintSetTo.applyTo(layout);
  }

  private int resultTabType = TAB_TYPE_PERFORMANCE;

  public int getType() {
    return this.resultTabType;
  }

  public void setType(int value) {
    if (value != this.resultTabType) {
      this.resultTabType = value;
      if (onTabClickedListener != null) onTabClickedListener.OnTabClicked(resultTabType);
    }
  }

  public void setOnTabClickedListener(OnTabClickedListener onTabClickedListener) {
    this.onTabClickedListener = onTabClickedListener;
  }

  public interface OnTabClickedListener {
    void OnTabClicked(int tabType);
  }

  public void Reset() {
    showTab(TAB_TYPE_DEFAULT);
  }

  public static final int TAB_TYPE_DEFAULT = 0;
  public static final int TAB_TYPE_PERFORMANCE = 0;
  public static final int TAB_TYPE_ACCURACY = 1;
}
