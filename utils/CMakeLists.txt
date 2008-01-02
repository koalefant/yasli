project("Utilities")

set(SOURCES

    Context.h
    Content.h Content.cpp
    Errors.h
    Factory.h
    Files.h Files.cpp
    MemoryReader.h MemoryReader.cpp
    MemoryWriter.h MemoryWriter.cpp
    Pointers.h
    SafeCast.h
    SharedLibApi.h
    StdAfx.h StdAfx.cpp
    StringList.h StringList.cpp
    StringUtil.h StringUtil.cpp
    Tokenizer.h Tokenizer.cpp
    UtilsAPI.h

    )

add_library("utils" ${SOURCES})
add_definitions(-DUTILS_EXPORTED)
set_target_properties("utils" PROPERTIES DEBUG_POSTFIX "-debug")

target_link_libraries("utils" "testo")

include("Testo")
TESTO("utils" "Test.cpp")
