project("xmath")

# file(GLOB SOURCES *.h *.cpp)
set(SOURCES xmath.h XMath.cpp Recti.cpp Rectf.cpp Range.cpp Colors.cpp ExceptionStub.cpp)
source_group("" FILES ${SOURCES})

include_directories(..)
if(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif(WIN32)
if (CMAKE_CXX_COMPILER MATCHES "clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-invalid-source-encoding")
endif()

if (EMSCRIPTEN)
add_library("xmath" SHARED ${SOURCES})
else (EMSCRIPTEN)
add_library("xmath" ${SOURCES})
endif (EMSCRIPTEN)

set_target_properties("xmath" PROPERTIES DEBUG_POSTFIX "-debug")

#include("Testo")
#TESTO("yasli" "Test.cpp")
