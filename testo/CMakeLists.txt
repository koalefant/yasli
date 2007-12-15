project("Testo")

set(SOURCES

    Testo.h
    src/Testo.cpp
    src/StdAfx.h
    src/StdAfx.cpp
    )

add_definitions(-DTESTO_EXPORTED)
add_library("testo" ${SOURCES})
set_target_properties("testo" PROPERTIES DEBUG_POSTFIX "-debug")
