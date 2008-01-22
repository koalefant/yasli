project("Utilities")

file(GLOB SOURCES *.h *.cpp)
source_group("" FILES ${SOURCES})

add_library("utils" ${SOURCES})
add_definitions(-DUTILS_EXPORTED)
set_target_properties("utils" PROPERTIES DEBUG_POSTFIX "-debug")

target_link_libraries("utils" "testo")

include("Testo")
TESTO("utils" "Test.cpp")
