import {
  Box,
  Flex,
  Text,
  Input,
  Button,
  useToast,
  InputGroup,
  InputRightElement,
  chakra, Icon,
} from "@chakra-ui/react";
import * as yup from "yup";
import { useForm } from "react-hook-form";
import { yupResolver } from "@hookform/resolvers/yup";

import { useNavigate } from "react-router-dom";
import { useEffect, useMemo, useState } from "react";
import { FiEye, FiEyeOff } from "react-icons/fi";
import { useLoginMutation } from "../hooks/useLoginMutation";
import { useUser } from "../hooks/useUser";
import { ReactComponent as MLCommonsLogo } from "../../../assets/MLCommonsLogo.svg";

export type FormValTypes = {
  email: string;
  password: string;
};

const initialValues = {
  email: "",
  password: "",
};

const schema = yup.object().shape({
  email: yup.string().required("Email is a required field"),
  password: yup.string().required("Password is a required field"),
});

const LoginPage = () => {
  const { mutate } = useLoginMutation();
  const navigate = useNavigate();
  const toast = useToast();
  const [show, setShow] = useState(false);

  const user = useUser();

  console.log("login user", user);
  const handleClick = () => setShow(!show);

  const isExpiredSession = window.location.search.includes("expired=true");
  const showError = useMemo(
    () => Boolean(isExpiredSession),
    [isExpiredSession],
  );

  useEffect(() => {
    if (showError) {
      toast({
        title: "Session Expired",
        description: "Please login again",
        status: "error",
        duration: 5000,
        isClosable: true,
      });
      navigate(window.location.pathname);
    }
  }, [showError, navigate, toast]);

  const hookFormVals = useForm<FormValTypes>({
    resolver: yupResolver(schema),
    defaultValues: initialValues,
  });
  const { register, handleSubmit, formState } = hookFormVals;
  const { errors } = formState as any;

  const onSubmit = async (formData: FormValTypes) => {
    mutate(formData);
  };

  const onForgotPassword = () => {
    // navigate("/forgot-password");
  };

  const renderErr = (errKey: string) => {
    const message = errors[errKey]?.message;

    return (
      <Text mt={2} style={{ color: "red" }}>
        {message}
      </Text>
    );
  };

  return (
    <Box
      display="flex"
      h="100vh"
      justifyContent="center"
      alignItems="center"
      flexDir="column"
    >
      <Flex
        bg="#fff"
        justify="center"
        w={{ base: "100%", sm: "452px" }}
        h="500px"
        boxShadow={{
          base: "none",
          sm: "0px 0px 20px rgba(0, 0, 0, 0.15)",
        }}
      >
        <Box
          display="flex"
          flexDir="column"
          alignItems="center"
          minW={{ base: "100%", sm: "354px" }}
        >
          <Flex justify="center">
            <Icon as={MLCommonsLogo} fontSize="150px" />
          </Flex>
          <Flex flexDir="column" w={{ base: "88%", sm: "auto" }}>
            <chakra.form onSubmit={handleSubmit(onSubmit)}>
              <Flex flexDir="column" w="100%" mx="auto">
                <Input
                  bg="#E4ECFC"
                  borderRadius="20px"
                  placeholder="email"
                  h="44px"
                  mt={10}
                  isInvalid={!!errors?.email}
                  errorBorderColor="red.300"
                  {...register("email", {
                    setValueAs: (v) => v.trim(),
                  })}
                />
                {errors?.email?.message && renderErr("email")}
              </Flex>
              {/* ====== */}
              <InputGroup
                size="md"
                as={Flex}
                flexDir="column"
                w="100%"
                mx="auto"
              >
                <Input
                  minW={{ base: "80%", sm: "354px" }}
                  bg="#E4ECFC"
                  borderRadius="20px"
                  type={show ? "text" : "password"}
                  placeholder="Password"
                  h="44px"
                  mt={4}
                  isInvalid={!!errors?.password}
                  errorBorderColor="red.300"
                  {...register("password")}
                />
                <InputRightElement mt={4}>
                  <Button
                    variant={"unstyled"}
                    size="sm"
                    display={"flex"}
                    onClick={handleClick}
                  >
                    {show ? <FiEye /> : <FiEyeOff />}
                  </Button>
                </InputRightElement>
                {errors?.password?.message && renderErr("password")}
              </InputGroup>

              <Flex justifyContent="flex-end">
                <Button mt={2.5} variant="link" onClick={onForgotPassword}>
                  Forgot password?
                </Button>
              </Flex>

              <Button
                variant={"purple"}
                mt={10}
                fontSize={16}
                type="submit"
                w={"100%"}
              >
                Login
              </Button>
            </chakra.form>
          </Flex>
        </Box>
      </Flex>
    </Box>
  );
};

export default LoginPage;
