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
  Icon,
  IconButton,
} from "@chakra-ui/react";

import { GiHamburgerMenu } from "react-icons/gi";
import MenuContent from "./MenuContent";

const TabletNavigationDrawer = () => {
  const { isOpen, onOpen, onClose } = useDisclosure();

  const menuFlexStyle = {
    flexDir: "column",
    w: "100%",
    display: "flex",
    bg: "#fff",
    pb: "20px",
    fontSize: "30px",
    fontWeight: "bold",
    overflowY: "hidden",
  };

  return (
    <>
      <IconButton
        bg="#fff"
        onClick={onOpen}
        aria-label="menu"
        icon={<Icon color="#1033A5" as={GiHamburgerMenu} fontSize="24px" />}
      />
      <Drawer isOpen={isOpen} placement="left" onClose={onClose} size="xs">
        <DrawerOverlay />
        <DrawerContent>
          <DrawerBody p={0}>
            <MenuContent flexStyle={menuFlexStyle} />
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
          </DrawerFooter>
        </DrawerContent>
      </Drawer>
    </>
  );
};

export default TabletNavigationDrawer;
