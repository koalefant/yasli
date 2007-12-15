project("yasli")

set(SOURCES

    API.h
    Archive.h Archive.cpp
    Serializer.h
    Helpers.h

    TypeID.h

    EnumDescription.h EnumDescription.cpp

    TypesFactory.h TypesFactory.cpp

    LibrarySelector.h
    LibraryReference.h
    Library.h Library.cpp

    TextOArchive.h TextIArchive.h

    STL.h STLImpl.h
    Pointers.h PointersImpl.h
    StringList.h StringList.cpp

    Math.cpp uMath.cpp
    Colors.cpp Colors.h
    FileSelector.h FileSelector.cpp

    StdAfx.h StdAfx.cpp
    TextIArchive.h TextIArchive.cpp
    TextOArchive.h TextOArchive.cpp

    )

add_definitions(-DSERIALIZATION_EXPORTED)
add_library("yasli" ${SOURCES})
set_target_properties("yasli" PROPERTIES DEBUG_POSTFIX "-debug")

include("Testo")
TESTO("yasli" "Test.cpp")

target_link_libraries("yasli" "utils")
