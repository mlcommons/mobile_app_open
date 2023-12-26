import { useState, useEffect } from "react";
import { Flex, useDisclosure, useMediaQuery } from "@chakra-ui/react";
import ChakraBreakpoints from "../../../constants/breakpoints";
import { useLocation } from "react-router-dom";
import { css } from "@emotion/react";
import sidebarLinks from "../../../constants/sidebarLinks";
import TabletNavigationDrawer from "./TabletNavigationDrawer";
import { Box, IconButton } from "@chakra-ui/react";
import { FaFilter } from "react-icons/fa";
import FiltersModal from "../../filters/components/FiltersModal";

type Props = {
  onToggle: () => void;
  isOpen: boolean;
};

const getOptions = (currPath: string) => {
  const sideBarLinksVals = Object.values(sidebarLinks);
  const pathString = currPath.slice(1);
  const currentLink = sideBarLinksVals.find((item) => {
    const { path } = item;
    if (path === pathString) return true;
    return null;
  });
  const currentModule = currentLink?.module;
  return sideBarLinksVals.reduce(
    (acc: { value: string; label: string }[], curr: any) => {
      const { label, path, module } = curr;
      if (currentModule === module) {
        acc = [...acc, { value: path, label }];
      }
      return acc;
    },
    [],
  );
};

const ResponsiveMenuHeader = ({ onToggle, isOpen }: Props) => {
  const { pathname: currPath } = useLocation();

  const options = getOptions(currPath);

  const matchedPath = options.find(
    (item) => item?.value === currPath?.slice(1),
  );
  const initVal = matchedPath?.value || options[0]?.value;

  const [currSelectVal, setCurrSelectVal] = useState(initVal);

  useEffect(() => {
    if (currSelectVal !== initVal) {
      setCurrSelectVal(initVal);
    }
  }, [currPath, currSelectVal, initVal]);

  const [isBelowXl] = useMediaQuery(`(max-width: ${ChakraBreakpoints.xl})`);

  const isDrawerVisible = isBelowXl;

  const disclosureProps = useDisclosure();
  const { onOpen } = disclosureProps;

  type FilterIconProps = {
    onClick: any;
  };

  const FilterIcon = ({ onClick }: FilterIconProps) => {
    return (
      <Box
        position="fixed"
        top="1rem"
        right="2rem"
        display="flex"
        justifyContent="flex-end"
        alignItems="flex-start"
      >
        <IconButton
          icon={<FaFilter />}
          onClick={onOpen}
          aria-label="Filter"
          variant="ghost"
        />
      </Box>
    );
  };

  const renderFullHeader = () => (
    <Flex
      px="20px"
      py="15px"
      w="100%"
      maxW="100vw"
      minH="60px"
      alignItems={"center"}
      // ml="auto"
      css={css`
        div[role="menu"] {
          position: relative;
          box-shadow: 0 0 10px rgb(0 0 0 / 15%);
        }
      `}
      zIndex={5}
    >
      <FilterIcon onClick={onOpen} />
      <FiltersModal disclosureProps={disclosureProps} />
      {isDrawerVisible ? (
        <Flex mr={2}>
          {" "}
          <TabletNavigationDrawer />
        </Flex>
      ) : null}
    </Flex>
  );

  return renderFullHeader();
};

export default ResponsiveMenuHeader;
