project("yasli")

file(GLOB SOURCES *.h *.cpp)
source_group("" FILES ${SOURCES})

add_library("yasli" ${SOURCES})
set_target_properties("yasli" PROPERTIES DEBUG_POSTFIX "-debug")

include("Testo")
TESTO("yasli" "Test.cpp")

target_link_libraries("yasli" "utils")
