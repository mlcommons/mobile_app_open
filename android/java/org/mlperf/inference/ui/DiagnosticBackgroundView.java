package org.mlperf.inference.ui;

import android.animation.ValueAnimator;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.LinearGradient;
import android.graphics.Paint;
import android.graphics.Shader;
import android.util.AttributeSet;
import android.util.Log;
import android.util.TypedValue;
import android.view.View;
import androidx.annotation.Nullable;
import org.mlperf.inference.R;

public class DiagnosticBackgroundView extends View {

  private float fill;
  Paint mPaint;
  LinearGradient gradient;
  final ValueAnimator animator = new ValueAnimator();
  TypedValue tv;

  public DiagnosticBackgroundView(Context context) {
    super(context);
    setup();
  }

  public DiagnosticBackgroundView(Context context, @Nullable AttributeSet attrs) {
    super(context, attrs);
    setup();
  }

  public DiagnosticBackgroundView(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
    super(context, attrs, defStyleAttr);
    setup();
  }

  /** @param dY y coordinate in pixels to extend fill to. Animates transition to fill height. */
  public void setFill(float dY) {
    if (dY == fill) return;

    animator.cancel();
    animator.setFloatValues(fill, dY);
    animator.start();
  }

  /**
   * @param dY y coordinate in pixels to extend fill to. Immediately applies and does not animate.
   */
  public void setFillImmediate(float dY) {
    fill = dY;
    invalidate();
  }

  @Override
  protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    // fill screen
    setMeasuredDimension(
        getResources().getDisplayMetrics().widthPixels,
        getResources().getDisplayMetrics().heightPixels);
  }

  @Override
  protected void onSizeChanged(int w, int h, int oldW, int oldH) {
    super.onSizeChanged(w, h, oldW, oldH);
    Log.d("DiagnosticBackground", "w=" + w + " h=" + h + " fill=" + fill);
    int[] colors = {0xff31a2e2, 0xff3183e2, 0xff064474};
    float[] positions = {0f, .5f, 1f};
    gradient =
        new LinearGradient(0, (int) (h * .15f), 0, h, colors, positions, Shader.TileMode.CLAMP);
  }

  private void setup() {
    tv = new TypedValue();
    mPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
    mPaint.setStyle(Paint.Style.FILL);

    fill = getResources().getDimension(R.dimen.diagnostic_bg_height_default);
    animator.setDuration(1000);
    animator.addUpdateListener(
        valueAnimator -> {
          fill = (float) valueAnimator.getAnimatedValue();
          invalidate();
        });
  }

  @Override
  protected void onDraw(Canvas canvas) {
    super.onDraw(canvas);
    mPaint.setShader(gradient);

    getResources().getValue(R.dimen.oval_scale, tv, true);
    float scale = tv.getFloat();

    canvas.drawOval(
        -fill * scale + getWidth() * .5f, -fill, fill * scale + getWidth() * .5f, fill, mPaint);
  }
}
