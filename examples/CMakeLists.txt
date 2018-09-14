# C++11 examples
#foreach (Example IN ITEMS RandomSeed)
#
#  add_executable (${Example} ${Example}.cpp)
#
#  target_include_directories (${Example} PRIVATE ${PROJECT_SOURCE_DIR}/include)
#
#  set_target_properties (${Example} PROPERTIES CXX_STANDARD 11
#                                               CXX_STANDARD_REQUIRED ON
#											   CXX_EXTENSIONS OFF)
#endforeach (Example)

# SYCL C++14 examples
foreach (Example IN ITEMS RandomSeed
                          SYCL-RandomSeed)

  add_executable (${Example} ${Example}.cpp)

  target_include_directories (${Example} PRIVATE ${PROJECT_SOURCE_DIR}/include)

  set_target_properties (${Example} PROPERTIES CXX_STANDARD 14
                                               CXX_STANDARD_REQUIRED ON
											   CXX_EXTENSIONS OFF)

  add_sycl_to_target(TARGET ${Example}
                     SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/${Example}.cpp)

endforeach (Example)