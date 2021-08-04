package org.mlperf.inference;

import android.app.Activity;
import android.content.Context;

/** A singleton class to get global context statically. */
public class MLCtx {
  private static MLCtx mlctx;
  private static Context context;

  private MLCtx() {}

  public static MLCtx getInstance() {
    if (mlctx == null) {
      mlctx = new MLCtx();
    }

    return mlctx;
  }

  public static MLCtx getInstance(Activity activity) {
    if (context == null) {
      context = activity.getApplicationContext();
    }

    return getInstance();
  }

  public Context getContext() {
    return context;
  }
}
