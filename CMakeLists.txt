cmake_minimum_required(VERSION 2.8)
project("yasli")

if(UNIX)
# This is needed only for Vim to be able to figure out what path errors are erelative to
message("make: Entering Directory '${CMAKE_CURRENT_SOURCE_DIR}'")
endif()

if (MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
endif()

if (CMAKE_CXX_COMPILER MATCHES "clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations -Wno-unused-value -fsanitize=address")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
endif()

if (UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

mark_as_advanced(CMAKE_INSTALL_PREFIX)
mark_as_advanced(QT_QMAKE_EXECUTABLE)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

set(PROJECT_SOURCE_ROOT ${PROJECT_SOURCE_DIR})
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_ROOT})
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_ROOT})

add_subdirectory(UnitTest++)
include_directories(UnitTest++/src)

include_directories(.)
add_subdirectory(yasli)
add_subdirectory(yasli-example)
add_subdirectory(yasli-test)

add_subdirectory(XMath)

if(NOT YASLI_NO_QT)
	add_subdirectory(QPropertyTree)
	add_subdirectory(QPropertyTree-example)
endif()

if (MSVC)
   add_subdirectory(ww)
   add_subdirectory(ww-example)
endif (MSVC)

if(UNIX)
message("make: Entering Directory '${CMAKE_BINARY_DIR}'")
endif()
