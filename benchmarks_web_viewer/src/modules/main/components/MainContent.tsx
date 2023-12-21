import { Flex, Box } from "@chakra-ui/react";
import { Outlet } from "react-router-dom";

const MainContentWithHeader = () => {
  return (
    <Flex flexDir="column" flexGrow={1} minWidth={0}>
      <Box
        pos="relative"
        height={"100%"}
        backgroundColor={"#E3EBFC"}
        px={{
          base: 1,
          sm: "28px",
        }}
        pb={"28px"}
      >
        <Box>
          <Outlet />
        </Box>
      </Box>
    </Flex>
  );
};

export default MainContentWithHeader;
