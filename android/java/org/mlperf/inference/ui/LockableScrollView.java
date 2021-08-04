package org.mlperf.inference.ui;

import android.annotation.SuppressLint;
import android.content.Context;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.widget.ScrollView;

public class LockableScrollView extends ScrollView {
  private boolean scrollAllowed;

  public LockableScrollView(Context context) {
    super(context);
  }

  public LockableScrollView(Context context, AttributeSet attrs) {
    super(context, attrs);
  }

  public LockableScrollView(Context context, AttributeSet attrs, int defStyleAttr) {
    super(context, attrs, defStyleAttr);
  }

  public LockableScrollView(
      Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
    super(context, attrs, defStyleAttr, defStyleRes);
  }

  public void setScrollEnabled(boolean scrollAllowed) {
    this.scrollAllowed = scrollAllowed;
    if (!scrollAllowed) {
      scrollTo(0, 0); // scroll to top when scrolling is disabled
      smoothScrollTo(0, 0);
    }
  }

  @SuppressWarnings({"unused", "RedundantSuppression"})
  public boolean isScrollEnabled() {
    return scrollAllowed;
  }

  @Override
  public boolean onInterceptTouchEvent(MotionEvent ev) {
    if (!scrollAllowed) return false;
    return super.onInterceptTouchEvent(ev);
  }

  @SuppressLint("ClickableViewAccessibility")
  @Override
  public boolean onTouchEvent(MotionEvent ev) {
    if (!scrollAllowed) return false;
    return super.onTouchEvent(ev);
  }
}
