package org.mlperf.inference.exceptions;

import android.os.Build;
import android.os.Build.VERSION;

public class UnsupportedDeviceException extends Exception {
  private static final String LINE_SEPARATOR = "\n";

  UnsupportedDeviceException(String msg) {
    super(msg);
  }

  public static UnsupportedDeviceException make(String msg, String backend) {

    String error =
        "\n*** ERROR ***\n\n"
            + "Backend: "
            + backend
            + LINE_SEPARATOR
            + "Error message: "
            + msg
            + "\n\n*** DEVICE ***\n\n"
            + "Brand: "
            + Build.BRAND
            + LINE_SEPARATOR
            + "Device: "
            + Build.DEVICE
            + LINE_SEPARATOR
            + "Model: "
            + Build.MODEL
            + LINE_SEPARATOR
            + "Id: "
            + Build.ID
            + LINE_SEPARATOR
            + "Product: "
            + Build.PRODUCT
            + LINE_SEPARATOR
            + "\n*** FIRMWARE ***\n\n"
            + "SDK: "
            + VERSION.SDK_INT
            + LINE_SEPARATOR
            + "Release: "
            + VERSION.RELEASE
            + LINE_SEPARATOR
            + "Incremental: "
            + VERSION.INCREMENTAL
            + LINE_SEPARATOR;

    return new UnsupportedDeviceException(error);
  }
}
