import { Control, Controller } from "react-hook-form";
import { ResultFilterType } from "../models/filters.model";
import { Flex, SimpleGrid, Text } from "@chakra-ui/react";
import DatePicker from "react-date-picker";
import React from "react";
import { css } from "@emotion/react";

type Props = {
  control: Control<ResultFilterType, any>;
  renderErr: any;
};

const DateFilter = ({ control, renderErr }: Props) => {
  return (
    <SimpleGrid mt={2} columns={[1, 2]} spacingX={2}>
      <Flex alignItems={"center"}>
        <Text>From Creation Date:</Text>
      </Flex>
      <Flex css={datePickerWrapperStyles} flexDir="column">
        <Controller
          name="fromCreationDate"
          control={control}
          render={({ field }) => {
            const { value, onChange } = field;
            return (
              <DatePicker
                calendarAriaLabel="Date"
                clearIcon={null}
                calendarIcon={null}
                value={value}
                onChange={(newDate: any) => onChange(newDate)}
              />
            );
          }}
        />
        {renderErr("fromCreationDate")}
      </Flex>
      <Flex alignItems={"center"}>
        <Text>To Creation Date:</Text>
      </Flex>
      <Flex css={datePickerWrapperStyles} flexDir="column">
        <Controller
          name="toCreationDate"
          control={control}
          render={({ field }) => {
            const { value, onChange } = field;
            return (
              <DatePicker
                calendarAriaLabel="Date"
                clearIcon={null}
                calendarIcon={null}
                value={value}
                onChange={(newDate: any) => onChange(newDate)}
              />
            );
          }}
        />
        {renderErr("toCreationDate")}
      </Flex>
    </SimpleGrid>
  );
};

export default DateFilter;

const datePickerWrapperStyles = css`
  .react-date-picker__wrapper {
    /* border: 0px !important; */
    /* border-top: 0px; */
    border: 0px !important;
    border-bottom: 1px solid #cbd8f1 !important;
    height: 40px;
    text-indent: 10px;
  }
  .date-filter {
    background: #e4ecfc;
    border: 1px solid #cbd8f1;
    box-sizing: border-box;
    border-radius: 40px;
  }
`;
