import {
  Accordion,
  AccordionButton,
  AccordionItem,
  Box,
  Flex,
  HStack,
  Icon,
  Text,
  useAccordionItemState,
} from "@chakra-ui/react";

import { Link, useLocation } from "react-router-dom";
import React, { useState } from "react";
import sidebarLinks from "../../../constants/sidebarLinks";
import theme from "../../../styles/theme";

import { ReactComponent as MLCommonsLogo } from "../../../assets/MLCommonsLogo.svg";
import MainContent from "./MainContent";
import ModuleTypes from "../../../constants/moduleTypes";

const Main = () => {
  const { pathname: currPath } = useLocation();
  const currModule = currPath.split("/")[1];
  const getBgColor = (moduleType: string) => {
    return currModule === moduleType ? hoverColor : "transparent";
  };

  const hoverColor = "#E3EBFC";

  return (
    <Box display="flex" flexDir="column" h="100%" minH="100vh">
      <Box display="flex" h="100%" minH="100vh">
        <Flex
          flexDir="column"
          minW={{ base: "15%", lg: "220px" }}
          display={{ base: "none", xl: "flex" }}
          fontWeight="bold"
          overflowY="hidden"
        >
          <Flex justify="left" pl="10%">
            <Icon as={MLCommonsLogo} fontSize="120px" />
          </Flex>

          <Accordion allowToggle>
            <AccordionItem border="none">
              <AccordionButton
                _hover={{
                  bg: hoverColor,
                }}
                bg={getBgColor(ModuleTypes.benchmarks)}
                as={Link}
                to={sidebarLinks.BENCHMARKS.path}
                minH="55px"
                pl="10%"
              >
                <HStack>
                  {/*<Icon as={BenchmarksIcon} fontSize="20px" />*/}
                  <Text
                    color="#1033A5"
                    fontWeight={700}
                    fontFamily={theme.fonts.heading}
                    fontSize="14px"
                  >
                    Benchmarks
                  </Text>
                </HStack>
              </AccordionButton>
            </AccordionItem>
          </Accordion>
          <Flex
            fontSize="14px"
            cursor="pointer"
            _hover={{
              bg: hoverColor,
            }}
            alignItems="center"
            minH="30px"
            pl="10%"
          >
            <Text
              color="#1033A5"
              fontWeight={700}
              fontFamily={theme.fonts.heading}
              fontSize="14px"
            >
              Logout
            </Text>
          </Flex>
          <Flex pl="5px" position="absolute" bottom="0" width="100px" mt={3}>
            <Text color="#7E91C6" fontSize="12px" fontWeight={400}>
              Version: {1.0}
            </Text>
          </Flex>
        </Flex>
        <MainContent />
      </Box>
    </Box>
  );
};

export default Main;
