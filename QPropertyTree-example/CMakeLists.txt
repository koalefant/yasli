# makefile
cmake_minimum_required(VERSION 2.8)
set(CMAKE_AUTOMOC true)

find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

project("QPropertyTreeTest")

if (MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif (MSVC)

if (CMAKE_COMPILER_IS_GNUCC)
  #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif (CMAKE_COMPILER_IS_GNUCC)

#file(GLOB SOURCES *.h *.cpp)
set(SOURCES 
	main.cpp
	mainwindow.cpp mainwindow.h
	LogicEditor.cpp LogicEditor.h
  )
source_group("" FILES ${SOURCES})

add_executable(qpropertytree-example ${SOURCES})

target_link_libraries(qpropertytree-example yasli xmath qpropertytree)
set_target_properties(qpropertytree-example PROPERTIES DEBUG_POSTFIX "-debug")
set_target_properties(qpropertytree-example PROPERTIES RELWITHDEBINFO_POSTFIX "-relwithdebinfo")

if (MSVC_IDE)
    set_target_properties(qpropertytree-example PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR})
    set_target_properties(qpropertytree-example PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR})
    set_target_properties(qpropertytree-example PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${PROJECT_SOURCE_DIR})
    set_target_properties(qpropertytree-example PROPERTIES RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${PROJECT_SOURCE_DIR})
endif (MSVC_IDE)
