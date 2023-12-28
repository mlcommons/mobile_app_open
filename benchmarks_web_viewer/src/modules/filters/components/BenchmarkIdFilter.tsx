import React from "react";
import { Select, Flex, Text, SimpleGrid } from "@chakra-ui/react";
import { Controller, Control, UseFormRegister } from "react-hook-form";
import { ResultFilterType } from "../models/filters.model";
import { BenchmarkId } from "../../../constants/constants";

type Props = {
  control: Control<ResultFilterType, any>;
  register: UseFormRegister<ResultFilterType>;
  renderErr: any;
};

const BenchmarkIdFilter = ({ control, register }: Props) => {
  return (
    <SimpleGrid mt={2} columns={[1, 2]} spacingX={2}>
      <Flex alignItems={"center"}>
        <Text>Benchmark:</Text>
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
                {...register("benchmarkId")}
              >
                {Object.values(BenchmarkId.allIds).map((platform) => (
                  <option key={platform} value={platform}>
                    {platform}
                  </option>
                ))}
              </Select>
            );
          }}
        />
      </Flex>
    </SimpleGrid>
  );
};

export default BenchmarkIdFilter;
