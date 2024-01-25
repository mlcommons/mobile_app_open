import { Flex, CircularProgress } from "@chakra-ui/react";

type Props = {
  color?: string;
  [key: string]: unknown;
};

const CenterSpinner = ({ color, ...rest }: Props) => (
  <Flex justify="center" align="center" h="50vh" {...rest}>
    <CircularProgress
      isIndeterminate
      color={color ?? `${"#000"}`}
      size="24px"
    />
  </Flex>
);

export default CenterSpinner;
