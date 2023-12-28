import { useDispatch, useSelector } from "react-redux";
import { ResultFilter, ResultFilterType } from "../models/filters.model";
import { filtersSlice, selectFilters } from "../store/filters.store";

export const useFilters = () => {
  const dispatch = useDispatch();
  const actions = filtersSlice.actions;

  const { setFilters } = actions;

  const resultFilter = new ResultFilter(useSelector(selectFilters) || {});

  const setResultFilter = (resultFilter: ResultFilterType) =>
    dispatch(setFilters(resultFilter));

  return {
    resultFilter,
    setResultFilter,
  };
};
