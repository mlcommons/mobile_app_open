import React from "react";
import { Select, Flex, Text, SimpleGrid } from "@chakra-ui/react";
import { Controller, Control, UseFormRegister } from "react-hook-form";
import { ResultFilterType } from "../models/filters.model";
import { BackendId } from "../../../constants/constants";

type Props = {
  control: Control<ResultFilterType, any>;
  register: UseFormRegister<ResultFilterType>;
  renderErr: any;
};

const BackendFilter = ({ control, register, renderErr }: Props) => {
  return (
    <SimpleGrid mt={2} columns={[1, 2]} spacingX={2}>
      <Flex alignItems={"center"}>
        <Text>Backend:</Text>
      </Flex>
      <Flex flexDir="column">
        <Controller
          name="backend"
          control={control}
          render={({ field }) => {
            return (
              <Select
                variant="flushed"
                placeholder="Select an option"
                {...register("backend")}
              >
                {Object.values(BackendId.allIds).map((backendId) => (
                  <option key={backendId} value={backendId}>
                    {backendId}
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

export default BackendFilter;
