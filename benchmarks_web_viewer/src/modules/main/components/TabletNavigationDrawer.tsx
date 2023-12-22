import React from "react";
import {
  Drawer,
  DrawerBody,
  DrawerFooter,
  DrawerOverlay,
  DrawerContent,
} from "@chakra-ui/react";

import {
  useDisclosure,
  Button,
  Text,
  Flex,
  Icon,
  IconButton,
  HStack,
  Accordion,
  AccordionItem,
  AccordionButton,
} from "@chakra-ui/react";

import { Link, useLocation } from "react-router-dom";
import { GiHamburgerMenu } from "react-icons/gi";
import { ReactComponent as MLCommonsLogo } from "../../../assets/MLCommonsLogo.svg";
import sidebarLinks from "../../../constants/sidebarLinks";
import theme from "../../../styles/theme";

const hoverColor = "#E3EBFC";

const TabletNavigationDrawer = () => {
  const { isOpen, onOpen, onClose } = useDisclosure();

  const { pathname: currPath } = useLocation();

  const currModule = currPath.split("/")[1];

  return (
    <>
      <IconButton
        bg="#fff"
        onClick={onOpen}
        // colorScheme="blue"
        aria-label="menu"
        icon={<Icon color="#1033A5" as={GiHamburgerMenu} fontSize="24px" />}
      />
      <Drawer isOpen={isOpen} placement="left" onClose={onClose} size="xs">
        <DrawerOverlay />
        <DrawerContent>
          <DrawerBody p={0}>
            <Flex
              flexDir="column"
              w="100%"
              display="flex"
              bg="#fff"
              pb="20px"
              fontSize="30px"
              fontWeight="bold"
              overflowY="hidden"
            >
              <Flex
                justifyContent="left"
                alignItems="left"
                minH="60px"
                pl="10%"
              >
                <Flex justify="left" align="left">
                  {" "}
                  <Icon as={MLCommonsLogo} fontSize="100px" />
                </Flex>
              </Flex>
              <Accordion allowToggle>
                <AccordionItem border="none">
                  <AccordionButton
                    _hover={{
                      bg: hoverColor,
                    }}
                    as={Link}
                    to={sidebarLinks.BENCHMARKS.path}
                    minH="55px"
                    pl="10%"
                  >
                    <HStack>
                      {/*<Icon as={DashboardIcon} fontSize="20px" />*/}
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
              <Flex
                pl="5px"
                position="absolute"
                bottom="0"
                width="100px"
                mt={3}
              >
                <Text color="#7E91C6" fontSize="12px" fontWeight={400}>
                  Version: {1.0}
                </Text>
              </Flex>
            </Flex>
          </DrawerBody>

          <DrawerFooter>
            <Button
              borderRadius="40px"
              bg="#fff"
              mr={3}
              onClick={onClose}
              border="1px solid #1033A5"
            >
              Close
            </Button>
            {/* <Button colorScheme="blue">Save</Button> */}
          </DrawerFooter>
        </DrawerContent>
      </Drawer>
    </>
  );
};

export default TabletNavigationDrawer;
