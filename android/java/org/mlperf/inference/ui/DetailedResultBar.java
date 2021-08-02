package org.mlperf.inference.ui;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.LinearGradient;
import android.graphics.Paint;
import android.graphics.Shader;
import android.util.AttributeSet;
import android.view.View;
import androidx.annotation.Nullable;

/** Draws a horizontal gradient line inside a view with rounded edges. */
public class DetailedResultBar extends View {
  Paint mPaint;
  LinearGradient gradient;
  float progress;

  public DetailedResultBar(Context context, @Nullable AttributeSet attrs) {
    super(context, attrs);
    setup();
  }

  /** @param progress float normalized to 0..1 */
  public void setProgress(float progress) {
    if (progress < 0) progress = 0;
    if (progress > 1) progress = 1;
    this.progress = progress;
    invalidate();
  }

  private void setup() {
    mPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
    mPaint.setStyle(Paint.Style.STROKE);
    mPaint.setStrokeCap(Paint.Cap.ROUND);
    mPaint.setColor(0xffa0f2cf);
  }

  @SuppressLint("DrawAllocation") // Should be drawing very infrequently
  @Override
  protected void onDraw(Canvas canvas) {
    super.onDraw(canvas);

    // setup colors
    int[] colors = {0xff135384, 0xff3183E2, 0xff31B8E2, 0xff53CEA5, 0xffA0F2CF};
    float[] positions = {0f, 0.36f, 0.61f, 0.83f, 1.0f};
    gradient = new LinearGradient(0, 0, getWidth(), 0, colors, positions, Shader.TileMode.CLAMP);

    mPaint.setStrokeWidth(getHeight());
    mPaint.setShader(gradient);
    float halfHeight = getHeight() * .5f;
    //noinspection SuspiciousNameCombination
    canvas.drawLine(
        halfHeight,
        halfHeight,
        (getWidth() - getHeight()) * progress + halfHeight,
        halfHeight,
        mPaint);
  }
}
