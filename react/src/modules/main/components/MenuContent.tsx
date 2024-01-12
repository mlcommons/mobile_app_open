import {
  Accordion,
  AccordionButton,
  AccordionItem,
  Flex,
  HStack,
  Icon,
  Text,
} from "@chakra-ui/react";
import ModuleTypes from "../../../constants/moduleTypes";
import { Link, useLocation } from "react-router-dom";
import sidebarLinks from "../../../constants/sidebarLinks";
import theme from "../../../styles/theme";
import React from "react";

import { ReactComponent as MLCommonsLogo } from "../../../assets/MLCommonsLogo.svg";
import { useLogoutMutation } from "../../auth/hooks/useLogoutMutation";

type Props = {
  flexStyle: any;
};

const MenuContent = ({ flexStyle }: Props) => {
  const { pathname: currPath } = useLocation();
  const currModule = currPath.split("/")[1];
  const { mutate: logout } = useLogoutMutation();

  const onLogout = () => {
    console.log("onLogout");
    logout();
  };

  const getBgColor = (moduleType: string) => {
    return currModule === moduleType ? hoverColor : "transparent";
  };
  const hoverColor = "#E3EBFC";

  return (
    <Flex {...flexStyle}>
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
        alignItems="center"
        minH="30px"
        pl="10%"
        onClick={onLogout}
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
  );
};

export default MenuContent;
