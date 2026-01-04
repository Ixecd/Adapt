cmake_minimum_required(version 3.28)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Turn off the fking test!")
add_subdirectory(benchmark)
target_link_libraries(main PUBLIC benchmark)