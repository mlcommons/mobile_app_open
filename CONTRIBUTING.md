# Contributing

The best way to contribute to the MLCommons is to get involved with one of our many project communities. You find more information about getting involved with MLCommons [here](https://mlcommons.org/en/get-involved/#getting-started). 

Generally we encourage people to become a MLCommons member if they wish to contribute to MLCommons projects, but outside pull requests are very welcome too.

To get started contributing code, you or your organization needs to sign the MLCommons CLA found at the [MLC policies page](https://mlcommons.org/en/policies/). Once you or your organization has signed the corporate CLA, please fill out this [CLA sign up form](https://forms.gle/Ew1KkBVpyeJDuRw67) form to get your specific GitHub handle authorized so that you can start contributing code under the proper license.

MLCommons project work is tracked with issue trackers and pull requests. Modify the project in your own fork and issue a pull request once you want other developers to take a look at what you have done and discuss the proposed changes. Ensure that cla-bot and other checks pass for your Pull requests.

## Contributing guidelines

If you have improvements to MLPerf, send us your pull requests! For those
just getting started, Github has a [howto](https://help.github.com/articles/using-pull-requests/).

### Pull Request Checklist

Before sending your pull requests, make sure you followed this list.

- Read [contributing guidelines](CONTRIBUTING.md)
- Ensure you have signed the [Contributor License Agreement (CLA)](https://cla.developers.google.com/).
- (Note: additional technical details TBD by community.)

## How to become a contributor and submit your own code

### Contributor License Agreements

We'd love to accept your patches! Before we can take them, we have to jump a couple of legal hurdles.

Please fill out either the individual or corporate Contributor License Agreement (CLA).

  * If you are an individual writing original source code and you're sure you own the intellectual property, then you'll need to sign an [individual CLA](https://code.google.com/legal/individual-cla-v1.0.html).
  * If you work for a company that wants to allow you to contribute your work, then you'll need to sign a [corporate CLA](https://code.google.com/legal/corporate-cla-v1.0.html).

Follow either of the two links above to access the appropriate CLA and instructions for how to sign and return it. Once we receive it, we'll be able to accept your pull requests.

***NOTE***: Only original source code from you and other people that have signed the CLA can be accepted into the main repository. (Note: we need to modify this to allow third party code under Apache2 or MIT license with additional review.)

### Contributing code

#### License

Include a license at the top of new files.

*   [C/C++ license example](https://github.com/mlperf/policies/blob/master/license_example.cpp)
*   [Java license example](https://github.com/mlperf/policies/blob/master/license_example.cpp)

#### Coding styles

The code was originally contributed by Google so it conforms to the
[Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) and
[Google Java Style Guide](https://google.github.io/styleguide/javaguide.html).

From the root directory you can run the command `make format` to format all files or `make format/<bazel|java|clang|dart>` to format only certain files in the directory.
See [format.mk](tools/formatter/format.mk) for more commands.

Running `make format` requires you have all the tools installed locally on your computer.
For your convenience you can use a Docker image to format code files by running `make docker/format`.
Note: if you have an error related to flutter when running `make docker/format`, 
try to run `cd flutter && flutter clean` to clear flutter cache, then try `make docker/format` again.
