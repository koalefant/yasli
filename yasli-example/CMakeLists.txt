cmake_minimum_required(VERSION 2.8)
project("yasli-example")

include_directories(.)

set(SOURCES Example.cpp)
source_group("" FILES ${SOURCES})
add_executable(yasli-example-exe ${SOURCES})
set_target_properties(yasli-example-exe PROPERTIES DEBUG_POSTFIX "-debug")
set_target_properties(yasli-example-exe PROPERTIES RELWITHDEBINFO_POSTFIX "-relwithdebinfo")
set_target_properties(yasli-example-exe PROPERTIES MINSIZEREL_POSTFIX "-minsizerel")
target_link_libraries(yasli-example-exe yasli)
