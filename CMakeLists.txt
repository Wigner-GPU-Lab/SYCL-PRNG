cmake_minimum_required (VERSION 3.2.2)

project (SYCL-PRNG LANGUAGES CXX)

option (BUILD_EXAMPLES "Build example applications" ON)

set (SYCL-PRNG_ROOT ${PROJECT_SOURCE_DIR})

if (BUILD_EXAMPLES)
  add_subdirectory (examples)
endif (BUILD_EXAMPLES)