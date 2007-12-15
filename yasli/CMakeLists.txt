project("yasli")

set(SOURCES

    API.h
    Archive.h Archive.cpp
    Serializer.h
    Helpers.h

    TypeID.h

    EnumDescription.h EnumDescription.cpp

    TypesFactory.h TypesFactory.cpp

    LibraryReference.h
    Library.h Library.cpp

    TextOArchive.h TextIArchive.h

    STL.h STLImpl.h
    Pointers.h PointersImpl.h
    StringList.h StringList.cpp

    FileSelector.h FileSelector.cpp

    StdAfx.h StdAfx.cpp
    TextIArchive.h TextIArchive.cpp
    TextOArchive.h TextOArchive.cpp

    )

# interesting only for SHARED library version
#add_definitions(-DSERIALIZATION_EXPORTED)

add_library("yasli" ${SOURCES})
set_target_properties("yasli" PROPERTIES DEBUG_POSTFIX "-debug")

include("Testo")
TESTO("yasli" "Test.cpp")

target_link_libraries("yasli" "utils")
