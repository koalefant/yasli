project("yasli")

file(GLOB SOURCES *.h *.cpp)
source_group("" FILES ${SOURCES})

if(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif(WIN32)

add_library("yasli" ${SOURCES})
set_target_properties("yasli" PROPERTIES DEBUG_POSTFIX "-debug")

#include("Testo")
#TESTO("yasli" "Test.cpp")
