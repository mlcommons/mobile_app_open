import { useDispatch, useSelector } from "react-redux";
import { ResultFilter } from "../models/filters.model";
import { filtersSlice, selectFilters } from "../store/filters.store";

export const useFilters = () => {
  const dispatch = useDispatch();
  const actions = filtersSlice.actions;

  const { setFilters } = actions;

  const resultFilter = useSelector(selectFilters);

  const setResultFilter = (resultFilter: ResultFilter) =>
    dispatch(setFilters(resultFilter));

  return {
    resultFilter,
    setResultFilter,
  };
};
