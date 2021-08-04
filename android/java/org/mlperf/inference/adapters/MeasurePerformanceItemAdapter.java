package org.mlperf.inference.adapters;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;
import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;
import java.util.ArrayList;
import org.mlperf.inference.R;
import org.mlperf.inference.models.ItemStatus;

public class MeasurePerformanceItemAdapter
    extends RecyclerView.Adapter<MeasurePerformanceItemAdapter.ItemViewHolder> {

  public final ArrayList<ItemStatus> myDataset;
  final View.OnClickListener onItemInfoClicked;

  @SuppressWarnings({"unused", "RedundantSuppression"})
  public MeasurePerformanceItemAdapter(
      ArrayList<ItemStatus> myDataset, View.OnClickListener onItemInfoClicked) {
    this.myDataset = myDataset;
    this.onItemInfoClicked = onItemInfoClicked;
  }

  // Provide a reference to the views for each data item
  // Complex data items may need more than one view per item, and
  // you provide access to all the views for a data item in a view holder.
  // Each data item is just a string in this case that is shown in a TextView.
  static class ItemViewHolder extends RecyclerView.ViewHolder {
    public final ViewGroup view;

    public ItemViewHolder(ViewGroup view) {
      super(view);
      this.view = view;
    }

    public void setData(ItemStatus status, View.OnClickListener onItemInfoClicked) {
      TextView tv = view.findViewById(R.id.calculatingStatusTitle);
      if (tv != null) tv.setText(status.title);

      boolean checked = status.value;

      ImageView iv = view.findViewById(R.id.calculatingStatusIcon);
      if (iv != null) {
        iv.setImageResource(checked ? R.drawable.circle : R.drawable.circle_outline);
        iv.setOnClickListener(onItemInfoClicked);
      }
    }
  }

  @SuppressWarnings({"unused", "RedundantSuppression"})
  @NonNull
  @Override
  public ItemViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
    View view =
        LayoutInflater.from(parent.getContext())
            .inflate(R.layout.calculating_results_content_item, parent, false);
    return new ItemViewHolder((ViewGroup) view);
  }

  @SuppressWarnings({"unused", "RedundantSuppression"})
  @Override
  public void onBindViewHolder(@NonNull ItemViewHolder holder, int position) {
    ItemStatus score = myDataset.get(position);
    holder.setData(score, onItemInfoClicked);
  }

  @SuppressWarnings({"unused", "RedundantSuppression"})
  @Override
  public int getItemCount() {
    return myDataset.size();
  }
}
