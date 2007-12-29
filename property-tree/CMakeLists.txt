project("YasliPropertyTree")
set(LIBRARY_NAME "property-tree")

set(SOURCES

    PropertyItemFactory.h
    PropertyItem.h PropertyItem.cpp
    PropertyItemContainer.h
    PropertyItemBasic.h PropertyItemBasic.cpp
    PropertyOArchive.h PropertyOArchive.cpp
    PropertyIArchive.h PropertyIArchive.cpp

    PropertyTree.h PropertyTree.cpp
    PropertyDialog.h PropertyDialog.cpp

    PropertyItemFileSelector.cpp
    PropertyItemLibrarySelector.cpp

    res/property_item_combo_button.xpm
    res/property_item_file_selector_button.xpm
    res/property_item_library_selector_button.xpm
    res/property_tree_minus.xpm
    res/property_tree_plus.xpm
    
    )

add_library(${LIBRARY_NAME} ${SOURCES})
add_definitions(-DGENERIC_EDITOR_EXPORTED)

set_target_properties(${LIBRARY_NAME} PROPERTIES DEBUG_POSTFIX "-debug")
target_link_libraries(${LIBRARY_NAME} "utils" "yasli")

#install(TARGETS ${LIBRARY_NAME} DESTINATION ..)
