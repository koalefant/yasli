cmake_minimum_required(VERSION 2.8)
# makefile
project("YasliExample")

# ala build-configuration
set(CMAKE_BUILD_TYPE "Debug") # MinSizeRel

set(PROJECT_SOURCE_ROOT ${PROJECT_SOURCE_DIR})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_ROOT})
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_ROOT})

include_directories(.)
add_subdirectory(yasli)

add_executable("example" "Example.cpp")
set_target_properties("example" PROPERTIES DEBUG_POSTFIX "-debug")
target_link_libraries("example" "yasli")

add_subdirectory(UnitTest++)
include_directories(UnitTest++/src)

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

## CMake tests
# enable_testing()
# add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} -V)
# add_test(NAME Yasli-Test COMMAND yasli-test)
# add_dependencies(check yasli-test)
