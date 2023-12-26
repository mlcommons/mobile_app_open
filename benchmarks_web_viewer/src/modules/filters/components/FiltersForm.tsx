import { Button, Flex, SimpleGrid, Text } from "@chakra-ui/react";
import React from "react";
import { useForm, Controller } from "react-hook-form";
import { yupResolver } from "@hookform/resolvers/yup";
import * as yup from "yup";
import DatePicker from "react-date-picker";
import { partialRight } from "ramda";
import { ResultFilterType } from "../models/filters.model";
import { css } from "@emotion/react";

const schema = yup.object().shape({
  fromCreationDate: yup.date().nullable(),
  toCreationDate: yup.date().nullable(),
  platform: yup.string().nullable(),
  deviceModel: yup.string().nullable(),
  backend: yup.string().nullable(),
  manufacturer: yup.string().nullable(),
  soc: yup.string().nullable(),
  benchmarkId: yup.string().nullable(),
});

type Props = {
  onClose: () => void;
};

const FiltersForm = ({ onClose }: Props) => {
  const { handleSubmit, control, formState, register, reset, watch } =
    useForm<ResultFilterType>({
      resolver: yupResolver(schema),
      defaultValues: {
        fromCreationDate: null,
        toCreationDate: null,
        platform: null,
        deviceModel: null,
        backend: null,
        manufacturer: null,
        soc: null,
        benchmarkId: null,
      },
    });

  const renderInputErr = (
    errKey: string,
    errors: any,
    styles?: Record<string, any>,
  ) => {
    const message = errors[errKey]?.message;
    return (
      message && (
        <Text style={{ color: "red" }} fontSize="14px" {...styles}>
          {message}
        </Text>
      )
    );
  };

  const { errors } = formState;
  const renderErr = partialRight(renderInputErr, [errors]);

  const onFormReset = () => {
    reset({
      fromCreationDate: null,
      toCreationDate: null,
      platform: null,
      deviceModel: null,
      backend: null,
      manufacturer: null,
      soc: null,
      benchmarkId: null,
    });
  };
  const onSubmit = (formValues: ResultFilterType) => {
    onClose();
  };

  return (
    <Flex p={10} flexDir="column" w="100%">
      <form onSubmit={handleSubmit(onSubmit)}>
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
            {renderErr("localStartDate")}
          </Flex>
        </SimpleGrid>

        <Flex mt={8} justifyContent="flex-end">
          <Button
            onClick={onFormReset}
            _hover={{
              bg: "#a6b5e2",
            }}
            mx="3"
            variant="ghost"
          >
            Reset Filters
          </Button>
          <Button variant="purpleVariant" type="submit">
            Apply Filters
          </Button>
        </Flex>
      </form>
    </Flex>
  );
};

export default FiltersForm;

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
