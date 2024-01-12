import React from "react";
import { Select, Flex, Text, SimpleGrid } from "@chakra-ui/react";
import { Controller, Control, UseFormRegister } from "react-hook-form";
import { Platform, ResultFilterType } from "../models/filters.model";

type Props = {
  control: Control<ResultFilterType, any>;
  register: UseFormRegister<ResultFilterType>;
  renderErr: any;
};

const PlatformFilter = ({ control, register, renderErr }: Props) => {
  return (
    <SimpleGrid mt={2} columns={[1, 2]} spacingX={2}>
      <Flex alignItems={"center"}>
        <Text>Platform:</Text>
      </Flex>
      <Flex flexDir="column">
        <Controller
          name="platform"
          control={control}
          render={({ field }) => {
            return (
              <Select
                variant="flushed"
                placeholder="Select an option"
                {...register("platform")}
              >
                {Object.values(Platform).map((platform) => (
                  <option key={platform} value={platform}>
                    {platform}
                  </option>
                ))}
              </Select>
            );
          }}
        />
        {renderErr("localStartDate")}
      </Flex>
    </SimpleGrid>
  );
};

export default PlatformFilter;
