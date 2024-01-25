import { Flex, useDisclosure } from "@chakra-ui/react";
import { Outlet } from "react-router-dom";
import React from "react";
import ResponsiveMenuHeader from "./ResponsiveMenuHeader";

const MainContentWithHeader = () => {
  const { onToggle, isOpen } = useDisclosure();

  return (
    <Flex flexDir="column" flexGrow={1} minWidth={0}>
      <ResponsiveMenuHeader onToggle={onToggle} isOpen={isOpen} />
      <Outlet />
    </Flex>
  );
};

export default MainContentWithHeader;
