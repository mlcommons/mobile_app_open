import { Box } from "@chakra-ui/react";

import React from "react";

import MainContent from "./MainContent";
import MenuContent from "./MenuContent";

const Main = () => {
  const menuFlexStyle = {
    flexDir: "column",
    minW: { base: "15%", lg: "220px" },
    display: { base: "none", xl: "flex" },
    fontWeight: "bold",
    overflowY: "hidden",
  };
  return (
    <Box display="flex" flexDir="column" h="100%" minH="100vh">
      <Box display="flex" h="100%" minH="100vh">
        <MenuContent flexStyle={menuFlexStyle} />
        <MainContent />
      </Box>
    </Box>
  );
};

export default Main;
