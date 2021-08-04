package org.mlperf.inference.ui;

import android.animation.AnimatorSet;
import android.animation.ObjectAnimator;
import android.animation.ValueAnimator;
import android.content.Context;
import android.graphics.drawable.Drawable;
import android.os.Handler;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.widget.FrameLayout;
import android.widget.ImageView;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import java.util.ArrayList;
import org.mlperf.inference.R;

public class PerformanceMeter extends FrameLayout {
  private View dialHand, dialHandLayout, dialGlow;
  private PerformanceMeterProgressBar progressBar;
  private float resultDegrees;
  private boolean displayResult;
  private float performanceMeterProgressSize;
  private float dotSize;
  private boolean dotVisibility = true;
  private boolean dotInterrupt = false;
  private int dotIndex = 0;

  @SuppressWarnings("deprecation")
  private final Handler dotHandler = new Handler();

  public PerformanceMeter(@NonNull Context context) {
    super(context);
    init(context);
  }

  public PerformanceMeter(@NonNull Context context, @Nullable AttributeSet attrs) {
    super(context, attrs);
    init(context);
  }

  public PerformanceMeter(
      @NonNull Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
    super(context, attrs, defStyleAttr);
    init(context);
  }

  public PerformanceMeter(
      @NonNull Context context, @Nullable AttributeSet attrs, int defStyleAttr, int defStyleRes) {
    super(context, attrs, defStyleAttr, defStyleRes);
    init(context);
  }

  public void init(Context context) {
    inflate(context, R.layout.performance_meter, this);
    progressBar = findViewById(R.id.progressBar);
    dialHand = findViewById(R.id.performanceDialHand);
    dialHandLayout = findViewById(R.id.performanceDialHandLayout);
    dialGlow = findViewById(R.id.dialGlow);
    createBlinkDots(context);
  }

  final ArrayList<View> dots = new ArrayList<>();

  private void createBlinkDots(Context context) {
    performanceMeterProgressSize =
        getResources().getDimension(R.dimen.performance_meter_progress_size);
    Drawable drawable = context.getDrawable(R.drawable.performance_meter_blink_circle);
    dotSize = drawable.getIntrinsicWidth();

    int numDots = 12;

    for (int i = 0; i < numDots; i++) {
      ImageView iv = new ImageView(getContext());
      iv.setImageDrawable(drawable);

      LayoutParams lp = new LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
      iv.setLayoutParams(lp);

      addView(iv);

      dots.add(iv);

      iv.setVisibility(View.INVISIBLE);
    }
  }

  @Override
  protected void onLayout(boolean changed, int left, int top, int right, int bottom) {
    super.onLayout(changed, left, top, right, bottom);

    // layout dots around center circle
    float width = right - left - dotSize;
    float height = bottom - top - dotSize;
    float centerX = width * .5f;
    float centerY = height * .5f;
    float dist = performanceMeterProgressSize * .5f - dotSize * .25f;

    float angle = 0;
    float angleBetween = (float) (2 * Math.PI / dots.size());
    for (View dot : dots) {
      dot.setTranslationX((float) (centerX - Math.sin(angle) * dist));
      dot.setTranslationY((float) (centerY + Math.cos(angle) * dist));
      angle += angleBetween;
    }
  }

  private void displayResults() {
    ObjectAnimator alphaAnim = ObjectAnimator.ofFloat(dialHand, "alpha", 0f, 1f).setDuration(100);

    int duration = 900;
    ObjectAnimator rotationAnim =
        ObjectAnimator.ofFloat(dialHandLayout, "rotation", 0f, resultDegrees).setDuration(duration);
    ValueAnimator progressbarAnim = new ValueAnimator();
    progressbarAnim.setDuration(duration);
    progressbarAnim.setFloatValues(0, resultDegrees);
    progressbarAnim.addUpdateListener(
        valueAnimator -> progressBar.setProgressEnd((float) valueAnimator.getAnimatedValue()));
    ObjectAnimator glowAnim = ObjectAnimator.ofFloat(dialGlow, "alpha", 0f, 1f).setDuration(300);

    AnimatorSet resultSet = new AnimatorSet();
    resultSet.playTogether(rotationAnim, progressbarAnim, glowAnim);

    AnimatorSet finalSet = new AnimatorSet();
    finalSet.playSequentially(alphaAnim, resultSet);
    finalSet.start();
  }

  private void reset() {
    Log.d("PerformanceMeter", "reset");
    displayResult = false;
    dialHandLayout.setRotation(0);
    progressBar.setProgressEnd(0);
    progressBar.setProgressStart(0);
    dialHand.setAlpha(0);
    dialGlow.setAlpha(0);

    for (View dot : dots) {
      dot.setVisibility(View.INVISIBLE);
      dot.setAlpha(1f);
    }
    dotVisibility = false;
    dotIndex = 0;
  }

  public void start() {
    Log.d("PerformanceMeter", "start");
    reset();
    dotInterrupt = false;

    dotHandler.post(dotProgressRunnable);
  }

  public void stop() {
    if (!displayResult) {
      dotInterrupt = true;
      reset();
    }
  }

  public void setResult(float value, float maxValue) {
    Log.d("PerformanceMeter", "setResult");
    resultDegrees = 360f * value / maxValue;
    displayResult = true;

    // display result without transitioning from progress spinner
    displayResults();

    // hide dots
    int delay = 0;
    for (View dot : dots) {
      if (dot.getVisibility() == View.VISIBLE) {
        dot.animate().alpha(0f).setDuration(100).setStartDelay(delay);
        delay += 10;
      }
    }
  }

  private final Runnable dotProgressRunnable =
      new Runnable() {
        @Override
        public void run() {
          if (dotInterrupt) {
            dotInterrupt = false;
            return;
          }
          if (displayResult) return;
          if (dotIndex == 0) {
            dotVisibility = !dotVisibility;
          }
          dots.get(dotIndex).setVisibility(dotVisibility ? View.VISIBLE : View.INVISIBLE);
          dotIndex = (dotIndex + 1) % dots.size();

          dotHandler.postDelayed(dotProgressRunnable, 500);
        }
      };
}
