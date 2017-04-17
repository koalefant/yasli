# makefile
cmake_minimum_required(VERSION 2.8)

project("ww-example")

if (MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif (MSVC)

set(SOURCES 
	Example.cpp
	ExampleCustomRows.cpp
	ExampleDigest.cpp
	ExampleObjects.cpp
	ExamplePolymorphicTree.cpp
	ExampleTable.cpp
	ExampleTwoTrees.cpp
	TestData.cpp
	Resource.h
  )
source_group("" FILES ${SOURCES})

add_executable(ww-example WIN32 ${SOURCES})

target_link_libraries(ww-example yasli wWidgets xmath)
set_target_properties(ww-example PROPERTIES DEBUG_POSTFIX "-debug")
set_target_properties(ww-example PROPERTIES RELWITHDEBINFO_POSTFIX "-relwithdebinfo")
set_target_properties(ww-example PROPERTIES RELEASE_POSTFIX "-release")
set_target_properties(ww-example PROPERTIES MINSIZEREL_POSTFIX "-minsizerel")

if (MSVC_IDE)
    set_target_properties(ww-example PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR})
    set_target_properties(ww-example PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR})
    set_target_properties(ww-example PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${PROJECT_SOURCE_DIR})
    set_target_properties(ww-example PROPERTIES RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${PROJECT_SOURCE_DIR})
endif (MSVC_IDE)
