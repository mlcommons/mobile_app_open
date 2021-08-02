package org.mlperf.inference.adapters;

import android.content.Context;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;
import org.mlperf.inference.R;
import org.mlperf.inference.models.ItemStatus;

public class MeasureItemUtil {
  public static View addItemToParent(
      Context context,
      ViewGroup parentLayout,
      ItemStatus itemStatus,
      View.OnClickListener onItemInfoClicked) {
    View view = View.inflate(context, R.layout.calculating_results_content_item, null);
    parentLayout.addView(view);

    TextView tv = view.findViewById(R.id.calculatingStatusTitle);
    tv.setText(itemStatus.title);

    boolean checked = itemStatus.value;

    ImageView iv = view.findViewById(R.id.calculatingStatusIcon);
    if (itemStatus.resIdSource != -1) {
      try {
        iv.setImageResource(itemStatus.resIdSource);
      } catch (Exception e) {
        // resource not found.
      }
    } else {
      iv.setImageResource(checked ? R.drawable.circle : R.drawable.circle_outline);
    }

    view.setTag(itemStatus.tag);
    view.setOnClickListener(onItemInfoClicked);
    return view;
  }
}
