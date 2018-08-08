###############################################################################
#
# Top level CMakeLists.txt file for the SYCL-RNG repo
# Hacker: Mate Ferenc Nagy-Egri
# Last modified: 2018.08.01. (yyyy.mm.dd.)
#
###############################################################################

#########################
#                       #
#         CMake         #
#                       #
#########################

# The minimum version of 'cmake' necessary to build this project
cmake_minimum_required (VERSION 3.2.2)

# The project name and version, release numbers
project (SYCL-PRNG LANGUAGES CXX
                   VERSION 0.0.1)

# Behavioural options for the project
option (BUILD_EXAMPLES "Build example applications" ON)
option (USE_SYCL "Turn on SYCL API support" ON)

# Find dependencies
if (USE_SYCL)
  set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)
  find_package(ComputeCpp)
endif (USE_SYCL)

# Recurse into target directories
if (BUILD_EXAMPLES)
  add_subdirectory (examples)
endif (BUILD_EXAMPLES)