import { logoutUser } from "../services/authService";
import { useNavigate } from "react-router-dom";
import { useMutation, useQueryClient } from "@tanstack/react-query";
import { QUERY_KEY } from "../../../constants/constants";

const logOut = async () => {
  return await logoutUser();
};

export const useLogoutMutation = () => {
  const navigate = useNavigate();
  const queryClient = useQueryClient();

  return useMutation({
    mutationFn: logOut,
    onSuccess(response) {
      console.log("Successfully logout");
      queryClient.setQueryData([QUERY_KEY.USER], null);
      navigate("/auth/sign-in");
    },
  });
};
