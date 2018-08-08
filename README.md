# SYCL-RNG Readme !! PRE-ALPHA !!

## Introduction

This is the README document for SYCL-RNG, a header-only library implementing various standard layout pseudo random number generators to be used with SYCL. The generators satisfy the concepts defined by the STL and integrate with `#include <chrono>` as well.

The library provides the following PRNGs under the [![License](https://img.shields.io/badge/License-BSD%203--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause):

- Tiny Mersenne Twister, both the 32/64-bit word derivates. Code is largely based on the original code which can be found in [this repository](https://github.com/MersenneTwister-Lab/TinyMT).

## Contents

* bench/
    - Some benchmarks, used to track the performance of SYCL-RNG.
* cmake/
    - Contains helper files and functions for downloading dependencies and performing common tasks (like adding tests, libraries etc.).
* CMakeLists.txt
    - The root CMakeLists.txt file, refer to this when building SYCL-RNG.
* CONTRIBUTING.md
    - Information about how to contribute bug reports, code, or other works to this project.
* include/
    - The directory under which the public interface is stored.
* LICENSE
    - The license is still to be decided upon.
* README.md
    - This readme file.
* examples/
    - Example programs to get inspiration from how to use the library.
* test/
    - All tests are bundled in this folder and are the mark of correctness when doing pre-merge tests.

## Requirements

* SYCL-RNG is tested against most recent releases of ComputeCpp and triSYCL. It is likely however, that it will be compatible with multiple versions concurrently as the SYCL interface is fixed.

* OpenCL 1.2-capable hardware and drivers with SPIR 1.2/SPIR-V/PTX support

* A C++-11 compiler and STL implementation

* CMake version 3.2.2

## Setup

SYCL-RNG uses CMake as its build system and unit test framework. SYCL-RNG has no dependencies, other than a SYCL SDK, which the library tries to find on it's own via designated CMake features. By default, the tests and library will be built, but not the benchmarks.

Please note, that if your development environment is not set up to find your SYCL SDK without manual intervention, then using:

- FindComputeCpp.cmake requires the variable `COMPUTECPP_PACKAGE_ROOT_DIR` to be set when configuring. It should point to the folder where bin/, lib/ etc. are. This should be the only argument that is mandatory, everything else should be optional. The default build type is Release, though this can be overridden.

## Troubleshooting

The master branch of SYCL-RNG should always compile and tests should always pass on our supported platforms. Ideally we should be writing portable, standards-compliant SYCL code, and as such it should pass tests on all compatible OpenCL hardware. See CONTRIBUTING.md for details about creating bug reports for this project.

## Maintainers

This project is written and maintained by the
[Wigner GPU-Laboratory](http://gpu.wigner.mta.hu/en).
Please get in touch if you have any issues or questions - you can reach us at
[gpu@wigner.mta.hu](mailto:gpu@wigner.mta.hu).

## Contributions

Please see the file CONTRIBUTIONS.md for further details if you would like to contribute code, build systems, bug fixes or similar.