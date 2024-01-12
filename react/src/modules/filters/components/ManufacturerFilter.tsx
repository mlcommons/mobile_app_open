import React from "react";
import { Flex, Text, SimpleGrid, Input } from "@chakra-ui/react";
import { Controller, Control, UseFormRegister } from "react-hook-form";
import { ResultFilterType } from "../models/filters.model";

type Props = {
  control: Control<ResultFilterType, any>;
  register: UseFormRegister<ResultFilterType>;
  renderErr: any;
};

const ManufacturerFilter = ({ control, register, renderErr }: Props) => {
  return (
    <SimpleGrid mt={2} columns={[1, 2]} spacingX={2}>
      <Flex alignItems={"center"}>
        <Text>Manufacturer:</Text>
      </Flex>
      <Flex flexDir="column">
        <Controller
          name="manufacturer"
          control={control}
          render={({ field }) => {
            return (
              <Input
                border={"none"}
                borderBottom="0.5px solid"
                rounded="none"
                borderColor="gray.300"
                id="manufacturer"
                {...register("manufacturer")}
              />
            );
          }}
        />
      </Flex>
    </SimpleGrid>
  );
};

export default ManufacturerFilter;
