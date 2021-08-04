/* Copyright 2019 The MLPerf Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/
package org.mlperf.inference;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import androidx.recyclerview.widget.RecyclerView;
import java.util.List;

/** Adapter for each result row */
public class ResultsAdapter extends RecyclerView.Adapter<ResultsAdapter.ViewHolder> {
  private final List<ResultHolder> data;
  private final LayoutInflater inflater;

  // data is passed into the constructor
  ResultsAdapter(Context context, List<ResultHolder> data) {
    this.inflater = LayoutInflater.from(context);
    this.data = data;
  }

  // inflates the row layout from xml when needed
  @Override
  public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
    View view = inflater.inflate(R.layout.results_row, parent, false);
    return new ViewHolder(view);
  }

  // binds the data to the TextView in each row
  @Override
  public void onBindViewHolder(ViewHolder holder, int position) {
    String modelStr = MLPerfTasks.getModelConfig(data.get(position).getId()).getName();
    String runtimeStr = data.get(position).getRuntime();
    String infTimeStr = String.format("%.02f", data.get(position).getScore());
    String accStr = data.get(position).getAccuracy();

    holder.modelTextView.setText(modelStr);
    holder.runtimeTextView.setText(runtimeStr);
    holder.infTimeTextView.setText(infTimeStr);
    holder.accTextView.setText(accStr);
  }

  // total number of rows
  @Override
  public int getItemCount() {
    return data.size();
  }

  /** Stores and recycles views as they are scrolled off screen */
  public static class ViewHolder extends RecyclerView.ViewHolder {
    final TextView modelTextView;
    final TextView runtimeTextView;
    final TextView infTimeTextView;
    final TextView accTextView;

    ViewHolder(View itemView) {
      super(itemView);
      modelTextView = itemView.findViewById(R.id.model);
      runtimeTextView = itemView.findViewById(R.id.runtime);
      infTimeTextView = itemView.findViewById(R.id.infTime);
      accTextView = itemView.findViewById(R.id.accuracy);
    }
  }

  // convenience method for getting data at click position
  ResultHolder getItem(int id) {
    return data.get(id);
  }

  // allows clicks events to be caught
  @SuppressWarnings("EmptyMethod")
  void setClickListener(ItemClickListener itemClickListener) {}

  /** parent activity will implement this method to respond to click events */
  public interface ItemClickListener {
    void onItemClick(View view, int position);
  }
}
