import React from "react";
import { Flex, Text, SimpleGrid, Input } from "@chakra-ui/react";
import { Controller, Control, UseFormRegister } from "react-hook-form";
import { ResultFilterType } from "../models/filters.model";

type Props = {
  control: Control<ResultFilterType, any>;
  register: UseFormRegister<ResultFilterType>;
  renderErr: any;
};

const SoCFilter = ({ control, register, renderErr }: Props) => {
  return (
    <SimpleGrid mt={2} columns={[1, 2]} spacingX={2}>
      <Flex alignItems={"center"}>
        <Text>SoC:</Text>
      </Flex>
      <Flex flexDir="column">
        <Controller
          name="soc"
          control={control}
          render={({ field }) => {
            return (
              <Input
                border={"none"}
                borderBottom="0.5px solid"
                rounded="none"
                borderColor="gray.300"
                id="soc"
                {...register("soc")}
              />
            );
          }}
        />
      </Flex>
    </SimpleGrid>
  );
};

export default SoCFilter;
