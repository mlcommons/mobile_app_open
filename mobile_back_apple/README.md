# mobile_back_apple

All Core ML models (the `*.mlmodel` or `*.mlpackage` files) must have
the [MLMultiArray](<https://developer.apple.com/documentation/coreml/mlmultiarray>)
type for input and output. Shape information must be set for both input and
output, since it will be read by the backend codes.
We currently use indexing for accessing the input and output tensor to match
with the API, therefore all input's and output's names will be sorted
alphabetically and then accessed by its index. It's recommended to add a prefix
to the input's and output's names to make sure that the names are sorted as
desired.

The scripts to convert the TensorFlow models to Core ML models used in the app
are located in the [models](models) directory.
