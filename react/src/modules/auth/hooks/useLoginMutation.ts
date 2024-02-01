import { loginUser } from "../services/authService";
import { useNavigate } from "react-router-dom";
import { useMutation, useQueryClient } from "@tanstack/react-query";
import { QUERY_KEY } from "../../../constants/constants";
import { useToast } from "@chakra-ui/react";

export type FormValTypes = {
  email: string;
  password: string;
};

const signIn = async (formData: FormValTypes) => {
  const body = {
    ...formData,
  };
  return await loginUser(body.email, body.password);
};

export const useLoginMutation = () => {
  const navigate = useNavigate();
  const queryClient = useQueryClient();
  const toast = useToast();

  return useMutation({
    mutationFn: signIn,
    onSuccess(response) {
      const user = response;
      if (user) {
        queryClient.setQueryData([QUERY_KEY.USER], user);
        navigate("/benchmarks");
      }
    },
    onError(error) {
      toast({
        title: "Login Error",
        description: "Login Failed.",
        status: "error",
        duration: 4000,
        isClosable: true,
      });
    },
  });
};
