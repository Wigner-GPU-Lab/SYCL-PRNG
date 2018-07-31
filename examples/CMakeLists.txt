foreach (Example IN ITEMS RandomSeed)

  add_executable (${Example} ${Example}.cpp)

  target_include_directories (${Example} PRIVATE ${SYCL-PRNG_ROOT}/include)

  set_target_properties (${Exampel} PROPERTIES CXX_STANDARD 11
                                               CXX_STANDARD_REQUIRED ON
											   CXX_EXTENSIONS OFF)

endforeach (Example)