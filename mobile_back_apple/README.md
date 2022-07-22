# mobile_back_apple (WORK IN PROGRESS)

All Core ML models (`*.mlmodel`) must have MLMultiArray type for input and output.
Shape information must be set for both input and output since it will be read by the code.
Since we currently use index for accessing the input and output tensor,
all input and output names will be sorted alphabetically and then accessed by index.
