package org.mlperf.inference.ui;

import static org.mlperf.inference.activities.CalculatingActivity.middleInterface;

import android.content.res.Resources;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewParent;
import android.widget.FrameLayout;
import android.widget.TextView;
import androidx.constraintlayout.widget.ConstraintLayout;
import com.google.android.material.snackbar.BaseTransientBottomBar;
import org.mlperf.inference.R;
import org.mlperf.proto.BackendSetting;
import org.mlperf.proto.BenchmarkSetting;

public class SnackBarResultLayout extends BaseTransientBottomBar<SnackBarResultLayout> {

  public SnackBarResultLayout(ViewGroup parent, final ViewGroup layout) {
    super(
        parent,
        layout,
        new com.google.android.material.snackbar.ContentViewCallback() {
          @SuppressWarnings({"unused", "RedundantSuppression"})
          @Override
          public void animateContentIn(int i, int i1) {
            layout.setVisibility(View.VISIBLE);
          }

          @SuppressWarnings({"unused", "RedundantSuppression"})
          @Override
          public void animateContentOut(int i, int i1) {
            layout.setVisibility(View.GONE);
          }
        });
    //noinspection Convert2Lambda
    layout
        .findViewById(R.id.resultsDetailsCloseButton)
        .setOnClickListener(
            new View.OnClickListener() {
              @Override
              public void onClick(View v) {
                dismiss();
              }
            });
    view.setPadding(0, 0, 0, 0); // set padding to 0
    ((View) view).setBackground(null);
  }

  @SuppressWarnings("RedundantSuppression")
  public static SnackBarResultLayout make(View view, String title) {
    // First we find a suitable parent for our custom view
    ViewGroup parent = findSuitableParent(view);
    if (parent == null) {
      throw new IllegalArgumentException(
          "No suitable parent found from the given view. Please provide a valid view.");
    }

    // We inflate our custom view
    ConstraintLayout customView =
        (ConstraintLayout)
            LayoutInflater.from(view.getContext())
                .inflate(R.layout.snackbar_bottom_results_details, parent, false);

    String subtitle = "";
    BackendSetting bs = middleInterface.getSettings();
    for (BenchmarkSetting bms : bs.getBenchmarkSettingList()) {
      if (bms.getBenchmarkId().equals(title)) {
        subtitle = bms.getConfiguration();
      }
    }

    String details;
    String detailsTitle;
    String idPrefix = title.substring(0, 2);
    Resources res = view.getResources();
    switch (idPrefix) {
      case "IC":
        details = res.getString(R.string.snack_title_details_IC);
        detailsTitle = res.getString(R.string.snack_title_IC);
        break;
      case "IS":
        details = res.getString(R.string.snack_title_details_IS);
        detailsTitle = res.getString(R.string.snack_title_IS);
        break;
      case "SM":
        // TODO add description for new image segmentation
        details = "";
        detailsTitle = "Image Segmentation (MOSAIC)";
        break;
      case "OD":
        details = res.getString(R.string.snack_title_details_OD);
        detailsTitle = res.getString(R.string.snack_title_OD);
        break;
      case "LU":
        details = res.getString(R.string.snack_title_details_LU);
        detailsTitle = res.getString(R.string.snack_title_LU);
        break;
      default:
        details = title;
        detailsTitle = title;
        break;
    }
    ((TextView) customView.findViewById(R.id.resultsDetailsSubTitle)).setText(subtitle);
    ((TextView) customView.findViewById(R.id.resultsDetailsTitle)).setText(detailsTitle);
    ((TextView) customView.findViewById(R.id.resultsDetails))
        .setText(details); // TODO replace with description with some kind of lookup

    // We create and return our Snackbar
    SnackBarResultLayout resultLayout = new SnackBarResultLayout(parent, customView);
    resultLayout.setDuration(LENGTH_INDEFINITE);
    resultLayout.setBehavior(
        new Behavior() {
          @SuppressWarnings({"SameReturnValue", "unused"})
          @Override
          public boolean canSwipeDismissView(View child) {
            return false;
          }
        });
    return resultLayout;
  }

  private static ViewGroup findSuitableParent(View view) {
    ViewGroup fallback = null;
    do {

      if (view.getClass().getName().toLowerCase().contains("coordinator")) {
        return (ViewGroup) view;
      } else if (view instanceof FrameLayout) {
        if (view.getId() == android.R.id.content) {
          return (ViewGroup) view;
        } else {
          fallback = (ViewGroup) view;
        }
      }

      if (view != null) {
        final ViewParent parent = view.getParent();
        view = parent instanceof View ? (View) parent : null;
      }
    } while (view != null);
    return fallback;
  }
}
