package org.mlperf.inference.ui;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.SweepGradient;
import android.util.AttributeSet;
import android.view.View;
import androidx.annotation.Nullable;

public class PerformanceMeterProgressBar extends View {

  private Paint mPaint;
  private float halfStroke;
  private float progressStart = 0;
  private float progressEnd = 360;
  private SweepGradient gradient;

  public PerformanceMeterProgressBar(Context context) {
    super(context);
    setup();
  }

  public PerformanceMeterProgressBar(Context context, @Nullable AttributeSet attrs) {
    super(context, attrs);
    setup();
  }

  public PerformanceMeterProgressBar(
      Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
    super(context, attrs, defStyleAttr);
    setup();
  }

  @Override
  protected void onSizeChanged(int w, int h, int oldW, int oldH) {
    super.onSizeChanged(w, h, oldW, oldH);
    int[] colors = {0xffc4ddf0, 0xffffffff, 0xff8ff9cc, 0xffffffff, 0xffc4ddf0};
    float[] positions = {0f, .4f, .575f, .75f, 1f};
    gradient = new SweepGradient(w * .5f, h * .5f, colors, positions);
  }

  /** @param progress in range 0..360 */
  @SuppressWarnings({"unused", "RedundantSuppression"})
  public void setProgressEnd(float progress) {
    this.progressEnd = progress;
    if (progressEnd < progressStart) {
      progressStart = 0;
    }
    invalidate();
  }

  @SuppressWarnings({"unused", "RedundantSuppression"})
  public void setProgressStart(float progress) {
    this.progressStart = progress;
    invalidate();
  }

  private void setup() {
    halfStroke = 4 * getResources().getDisplayMetrics().density; // 4dp

    mPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
    mPaint.setStyle(Paint.Style.STROKE);
    mPaint.setStrokeCap(Paint.Cap.ROUND);
    mPaint.setColor(Color.WHITE);
    mPaint.setStrokeWidth(halfStroke * 2); // 8dp
  }

  @Override
  protected void onDraw(Canvas canvas) {
    super.onDraw(canvas);
    mPaint.setShader(gradient);
    canvas.drawArc(
        halfStroke,
        halfStroke,
        getWidth() - halfStroke,
        getHeight() - halfStroke,
        progressStart,
        progressEnd - progressStart,
        false,
        mPaint);
  }
}
