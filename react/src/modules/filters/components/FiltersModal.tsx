import {
  Modal,
  ModalOverlay,
  ModalContent,
  ModalHeader,
  ModalBody,
  ModalCloseButton,
} from "@chakra-ui/react";

import FiltersForm from "./FiltersForm";

type Props = {
  disclosureProps: any;
};

const FiltersModal = ({ disclosureProps }: Props) => {
  const { onClose, isOpen } = disclosureProps;

  return (
    <Modal isOpen={isOpen} onClose={onClose} size="2xl">
      <ModalOverlay />
      <ModalContent>
        <ModalHeader
          bg="#a6b5e2"
          borderRadius="5px 5px 0px 0px"
          color="#fff"
          fontSize="16px"
          lineHeight="20px"
        >
          Benchmark Filters
        </ModalHeader>
        <ModalCloseButton />
        <ModalBody p={0} borderRadius="8px" mx="auto" w="100%">
          <FiltersForm onClose={onClose} />
        </ModalBody>
      </ModalContent>
    </Modal>
  );
};

export default FiltersModal;
