cmake_minimum_required(VERSION 2.8)
project("yasli-benchmark")

include_directories(.)

set(SOURCES Benchmark.cpp)
source_group("" FILES ${SOURCES})
add_executable(yasli-benchmark-exe ${SOURCES})
set_target_properties(yasli-benchmark-exe PROPERTIES DEBUG_POSTFIX "-debug")
set_target_properties(yasli-benchmark-exe PROPERTIES RELWITHDEBINFO_POSTFIX "-relwithdebinfo")
set_target_properties(yasli-benchmark-exe PROPERTIES MINSIZEREL_POSTFIX "-minsizerel")
target_link_libraries(yasli-benchmark-exe yasli)
