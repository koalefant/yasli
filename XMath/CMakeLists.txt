project("xmath")

# file(GLOB SOURCES *.h *.cpp)
set(SOURCES XMath.cpp Recti.cpp Rectf.cpp Range.cpp RangedWrapper.cpp)
source_group("" FILES ${SOURCES})

if(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif(WIN32)

add_library("xmath" ${SOURCES})
set_target_properties("xmath" PROPERTIES DEBUG_POSTFIX "-debug")

#include("Testo")
#TESTO("yasli" "Test.cpp")
