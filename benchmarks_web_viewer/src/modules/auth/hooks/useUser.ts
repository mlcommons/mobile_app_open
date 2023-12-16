import { useQueryClient } from "@tanstack/react-query";
import { QUERY_KEY } from "../../../constants/queryKeys";
import { UserType } from "../models/user.model";

export const useUser = (): UserType | undefined => {
  const queryClient = useQueryClient();
  return queryClient.getQueryData([QUERY_KEY.USER]);
};
