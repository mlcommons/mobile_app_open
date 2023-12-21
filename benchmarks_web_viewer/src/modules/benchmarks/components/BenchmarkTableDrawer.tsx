import React from "react";

import { Flex, type FlexProps } from "@chakra-ui/react";
import { motion } from "framer-motion";
import BenchmarkDrawerContent from "./BenchmarkDrawerContent";

const MotionFlex = motion<FlexProps>(Flex);

type Props = {
  isOpen: boolean;
  onToggle: () => void;
  onClose: () => void;
  benchmarkId: string;
};

const BenchmarkTableDrawer = ({ isOpen, onClose, benchmarkId }: Props) => {
  const drawerRef = React.useRef<HTMLElement | null>(null);

  const getDrawerContents = () => {
    return (
      <BenchmarkDrawerContent
        onClose={onClose}
        isOpen={isOpen}
        benchmarkId={benchmarkId}
      />
    );
  };

  return (
    <MotionFlex
      ref={drawerRef}
      bg="#fff"
      pos="absolute"
      top={0}
      right={-600}
      h={{ base: "auto", lg: "100%" }}
      minH={{ base: "100%", lg: "none" }}
      maxH={{ base: "calc(100vh - 245px)", lg: "none" }}
      width={{ base: "100%", lg: "535px" }}
      boxShadow="-10px 0px 20px rgba(0, 0, 0, 0.05)"
      borderRadius="0px 5px 6px 0px"
      animate={{
        display: isOpen ? "flex" : "none",
        opacity: isOpen ? 1 : 0,
        right: isOpen ? 0 : -600,
      }}
      flexDir="column"
      px={{ base: "20px", md: "40px" }}
      py="40px"
      overflowY="scroll"
    >
      {getDrawerContents()}
    </MotionFlex>
  );
};

export default BenchmarkTableDrawer;
