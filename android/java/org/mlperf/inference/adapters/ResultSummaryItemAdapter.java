package org.mlperf.inference.adapters;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;
import java.text.DecimalFormat;
import java.util.ArrayList;
import org.mlperf.inference.AppConstants;
import org.mlperf.inference.R;
import org.mlperf.inference.ResultHolder;
import org.mlperf.inference.activities.CalculatingActivity;

public class ResultSummaryItemAdapter
    extends RecyclerView.Adapter<ResultSummaryItemAdapter.ItemViewHolder> {

  public final ArrayList<ResultHolder> myDataset;

  public ResultSummaryItemAdapter(ArrayList<ResultHolder> myDataset) {
    this.myDataset = myDataset;
  }

  public ResultSummaryItemAdapter(ArrayList<ResultHolder> myDataset, int type) {
    this.myDataset = myDataset;
    this._type = type;
  }

  private int _type = AppConstants.TYPE_PERFORMANCE;

  @SuppressWarnings({"unused", "RedundantSuppression"})
  public int getType() {
    return _type;
  }

  public void setType(int value) {
    _type = value;
    notifyDataSetChanged();
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

    public void setData(ResultHolder score, int type) {
      TextView titleTV = view.findViewById(R.id.summaryResultItemTitle);
      titleTV.setText(
          CalculatingActivity.getReadableBenchmarkTitle(view.getContext(), score.getId()));

      DecimalFormat df = new DecimalFormat("0.00");

      TextView scoreTV = view.findViewById(R.id.summaryResultItemScore);
      scoreTV.setText(
          type == AppConstants.TYPE_PERFORMANCE
              ? df.format(score.getScore())
              : score.getAccuracy());
    }
  }

  @SuppressWarnings({"unused", "RedundantSuppression"})
  @NonNull
  @Override
  public ItemViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
    View view =
        LayoutInflater.from(parent.getContext())
            .inflate(R.layout.summary_result_content_item, parent, false);
    return new ItemViewHolder((ViewGroup) view);
  }

  @SuppressWarnings({"unused", "RedundantSuppression"})
  @Override
  public void onBindViewHolder(@NonNull ItemViewHolder holder, int position) {
    ResultHolder score = myDataset.get(position);
    holder.setData(score, _type);
  }

  @SuppressWarnings({"unused", "RedundantSuppression"})
  @Override
  public int getItemCount() {
    return myDataset.size();
  }
}
