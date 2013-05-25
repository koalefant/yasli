cmake_minimum_required(VERSION 2.8)
project("yasli-test")

include_directories(.)

set(TEST_SOURCES
  "Test.cpp"
  "ComplexClass.h"
  "TestInPlaceArchive.cpp"
  "TestTextArchive.cpp"
  "TestJSONArchive.cpp"
  )
source_group("" FILES ${TEST_SOURCES})

add_executable(yasli-test-exe ${TEST_SOURCES})
target_link_libraries(yasli-test-exe yasli UnitTestPP)
add_custom_target(check ALL COMMAND yasli-test-exe)
