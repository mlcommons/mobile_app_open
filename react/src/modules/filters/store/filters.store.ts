import { createSlice, PayloadAction } from "@reduxjs/toolkit";
import { ResultFilterType } from "../models/filters.model";

interface FiltersState {
  resultFilter: ResultFilterType | null;
}

const initialState: FiltersState = {
  resultFilter: null,
};

export const filtersSlice = createSlice({
  name: "filtersState",
  initialState,
  reducers: {
    setFilters: (state, { payload }: PayloadAction<ResultFilterType>) => {
      state.resultFilter = payload;
    },
  },
});

export const filtersReducer = filtersSlice.reducer;

export const selectFilters = (state: any): ResultFilterType =>
  state.filtersState.resultFilter;
