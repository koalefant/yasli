# makefile
project("YasliExample")

# ala build-configuration
set(CMAKE_BUILD_TYPE "Debug") # MinSizeRel

# we need this to find our Testo.cmake
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/Testo)

set(PROJECT_SOURCE_ROOT ${PROJECT_SOURCE_DIR})

set(wxWidgets_USE_LIBS base core)
find_package(wxWidgets)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_ROOT})
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_ROOT})

include_directories(.)
add_subdirectory(yasli)
add_subdirectory(testo)

add_executable("example" "Example.cpp")
set_target_properties("example" PROPERTIES DEBUG_POSTFIX "-debug")
target_link_libraries("example" "testo" "yasli")

# libraries
if(wxWidgets_FOUND)
    add_definitions(${wxWidgets_DEFINITIONS})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${wxWidgets_CXX_FLAGS}")
    include_directories(${wxWidgets_INCLUDE_DIRS})

    add_subdirectory(property-tree)

    set(SOURCES "PropertyExample.cpp" "TestData.h" "PropertyEdit.cpp" "PropertyEdit.h")
    add_executable("property-example" ${SOURCES})
    set_target_properties("property-example" PROPERTIES DEBUG_POSTFIX "-debug")
    target_link_libraries("property-example" "property-tree" ${wxWidgets_LIBRARIES})
endif(wxWidgets_FOUND)
