import { Button, Flex, Text } from "@chakra-ui/react";
import React from "react";
import { useForm } from "react-hook-form";
import { yupResolver } from "@hookform/resolvers/yup";
import * as yup from "yup";
import { partialRight } from "ramda";
import { ResultFilter, ResultFilterType } from "../models/filters.model";
import { useFilters } from "../hooks/useFilters";
import PlatformFilter from "./PlatformFilter";
import DateFilter from "./DateFilter";
import BenchmarkIdFilter from "./BenchmarkIdFilter";
import BackendFilter from "./BackendFilter";
import DeviceModelFilter from "./DeviceModelFilter";
import ManufacturerFilter from "./ManufacturerFilter";
import SoCFilter from "./SoCFilter";

const schema = yup.object().shape({
  fromCreationDate: yup.date().nullable(),
  toCreationDate: yup
    .date()
    .nullable()
    .min(yup.ref("fromCreationDate"), "To date must be after From date"),
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
  const { resultFilter, setResultFilter } = useFilters();
  const {
    handleSubmit,
    control,
    formState: { errors },
    register,
    reset,
  } = useForm<ResultFilterType>({
    mode: "all",
    resolver: yupResolver(schema),
    defaultValues: {
      fromCreationDate: resultFilter.fromCreationDate,
      toCreationDate: resultFilter.toCreationDate,
      platform: resultFilter.platform,
      deviceModel: resultFilter.deviceModel,
      backend: resultFilter.backend,
      manufacturer: resultFilter.manufacturer,
      soc: resultFilter.soc,
      benchmarkId: resultFilter.benchmarkId,
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

  const renderErr = partialRight(renderInputErr, [errors]);

  const onFormReset = () => {
    const formValues = {
      fromCreationDate: null,
      toCreationDate: null,
      platform: null,
      deviceModel: null,
      backend: null,
      manufacturer: null,
      soc: null,
      benchmarkId: null,
    };
    reset(formValues);
    setResultFilter(new ResultFilter(formValues));
  };
  const onSubmit = (formValues: ResultFilterType) => {
    setResultFilter(new ResultFilter(formValues));
    onClose();
  };

  return (
    <Flex p={10} flexDir="column" w="100%">
      <form onSubmit={handleSubmit(onSubmit)}>
        <DateFilter control={control} renderErr={renderErr} />
        <PlatformFilter
          control={control}
          register={register}
          renderErr={renderErr}
        />
        <BenchmarkIdFilter
          control={control}
          register={register}
          renderErr={renderErr}
        />
        <BackendFilter
          control={control}
          register={register}
          renderErr={renderErr}
        />
        <DeviceModelFilter
          control={control}
          register={register}
          renderErr={renderErr}
        />
        <ManufacturerFilter
          control={control}
          register={register}
          renderErr={renderErr}
        />
        <SoCFilter
          control={control}
          register={register}
          renderErr={renderErr}
        />
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
