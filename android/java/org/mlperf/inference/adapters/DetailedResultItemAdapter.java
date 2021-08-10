package org.mlperf.inference.adapters;

import static org.mlperf.inference.activities.CalculatingActivity.middleInterface;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;
import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;
import java.text.DecimalFormat;
import java.util.ArrayList;
import org.mlperf.inference.AppConstants;
import org.mlperf.inference.Benchmark;
import org.mlperf.inference.R;
import org.mlperf.inference.ResultHolder;
import org.mlperf.inference.activities.CalculatingActivity;
import org.mlperf.inference.ui.DetailedResultBar;
import org.mlperf.proto.BackendSetting;
import org.mlperf.proto.BenchmarkSetting;

public class DetailedResultItemAdapter
    extends RecyclerView.Adapter<DetailedResultItemAdapter.ItemViewHolder> {

  public final ArrayList<ResultHolder> myDataset;
  public final View.OnClickListener onItemInfoClicked;

  public DetailedResultItemAdapter(
      ArrayList<ResultHolder> myDataset, View.OnClickListener onItemInfoClicked) {
    this.myDataset = myDataset;
    this.onItemInfoClicked = onItemInfoClicked;
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
    final ViewGroup view;

    public ItemViewHolder(ViewGroup view) {
      super(view);
      this.view = view;
    }

    public void setData(ResultHolder result, int type, View.OnClickListener onItemInfoClicked) {
      TextView titleTV = view.findViewById(R.id.detailedResultItemTitle);
      titleTV.setText(CalculatingActivity.getReadableBenchmarkTitle(result.getId()));

      DecimalFormat df = new DecimalFormat("0.00");
      TextView scoreTV = view.findViewById(R.id.detailedResultItemScore);
      scoreTV.setText(
          type == AppConstants.TYPE_PERFORMANCE
              ? df.format(result.getScore())
              : result.getAccuracy() + "");

      DetailedResultBar bar = view.findViewById(R.id.detailedResultsBar);
      float progress = 0;
      float maxScore = 0;
      try {
        ArrayList<Benchmark> benchmarks = middleInterface.getBenchmarks();
        for (Benchmark bm : benchmarks) {
          if (bm.getId().equals(result.getId())) {
            maxScore = bm.getMaxScore();
            break;
          }
        }
        progress =
            type == AppConstants.TYPE_PERFORMANCE
                ? result.getScore() / maxScore
                : // TODO: MLCommons - this should divide by the max.
                // TODO: MLCommons team to set max value in progress bar for accuracy display
                Float.parseFloat(result.getAccuracy()) / 100f;
      } catch (NumberFormatException e) {
        e.printStackTrace();
      }
      bar.setProgress(progress);

      ImageView icon = view.findViewById(R.id.detailedResultItemIcon);

      String subtitle = "";
      BackendSetting bs = middleInterface.getSettings();
      for (BenchmarkSetting bms : bs.getBenchmarkSettingList()) {
        if (bms.getBenchmarkId().equals(result.getId())) {
          subtitle = bms.getConfiguration();
        }
      }

      ((TextView) view.findViewById(R.id.detailedResultItemSubtitle)).setText(subtitle);

      for (Benchmark bm : CalculatingActivity.middleInterface.getBenchmarks()) {
        if (bm.getId().equals(result.getId())) {
          icon.setImageResource(bm.getIcon());
          break;
        }
      }

      ImageView b = view.findViewById(R.id.detailedResultItemMoreInfoButton);
      b.setTag(result.getId());
      b.setOnClickListener(onItemInfoClicked);
    }
  }

  @SuppressWarnings({"unused", "RedundantSuppression"})
  @NonNull
  @Override
  public ItemViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
    View view =
        LayoutInflater.from(parent.getContext())
            .inflate(R.layout.detailed_result_content_item, parent, false);
    return new ItemViewHolder((ViewGroup) view);
  }

  @SuppressWarnings({"unused", "RedundantSuppression"})
  @Override
  public void onBindViewHolder(@NonNull ItemViewHolder holder, int position) {
    ResultHolder result = myDataset.get(position);
    holder.setData(result, _type, onItemInfoClicked);
  }

  @SuppressWarnings({"unused", "RedundantSuppression"})
  @Override
  public int getItemCount() {
    return myDataset.size();
  }
}
