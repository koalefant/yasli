cmake_minimum_required(VERSION 2.8)
project("yasli-example")

include_directories(.)

add_executable("example" "Example.cpp")
set_target_properties("example" PROPERTIES DEBUG_POSTFIX "-debug")
target_link_libraries("example" "yasli")

set(TEST_SOURCES
  "Test.cpp"
  "ComplexClass.h"
  "TestInPlaceArchive.cpp"
  "TestTextArchive.cpp"
  )
source_group("" FILES ${TEST_SOURCES})

add_executable(yasli-test ${TEST_SOURCES})
target_link_libraries(yasli-test yasli UnitTestPP)
add_custom_target(check ALL COMMAND yasli-test)
