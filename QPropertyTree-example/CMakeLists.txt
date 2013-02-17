# makefile
cmake_minimum_required(VERSION 2.8.6)
set(CMAKE_AUTOMOC true)

find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

project("QPropertyTreeTest")

if (MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif (MSVC)

#file(GLOB SOURCES *.h *.cpp)
set(SOURCES main.cpp
  mainwindow.cpp mainwindow.h
  )
source_group("" FILES ${SOURCES})

add_executable(qpropertytree-example WIN32 ${SOURCES})

target_link_libraries(qpropertytree-example yasli xmath qpropertytree)
set_target_properties(qpropertytree-example PROPERTIES DEBUG_POSTFIX "-debug")

if (MSVC_IDE)
    set_target_properties(may PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR})
    set_target_properties(may PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR})
    set_target_properties(may PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${PROJECT_SOURCE_DIR})
    set_target_properties(may PROPERTIES RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${PROJECT_SOURCE_DIR})
endif (MSVC_IDE)
