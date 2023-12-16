import { createSlice, PayloadAction } from "@reduxjs/toolkit";
import { useDispatch, useSelector } from "react-redux";
import { ResultFilter } from "../models/filters.model";

interface FiltersState {
  resultFilter: ResultFilter | null;
}

const initialState: FiltersState = {
  resultFilter: null,
};

export const filtersSlice = createSlice({
  name: "filtersState",
  initialState,
  reducers: {
    setFilters: (state, { payload }: PayloadAction<ResultFilter>) => {
      state.resultFilter = payload;
    },
  },
});

export const filtersReducer = filtersSlice.reducer;

export const selectFilters = (state: any): ResultFilter =>
  state.filtersState.resultFilter;
