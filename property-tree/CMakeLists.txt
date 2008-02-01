project("YasliPropertyTree")

file(GLOB SOURCES *.cpp *.h)
source_group("" FILES ${SOURCES})

if(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif(WIN32)

set(RESOURCES
    res/property_item_combo_button.xpm
    res/property_item_file_selector_button.xpm
    res/property_item_library_selector_button.xpm
    res/property_tree_minus.xpm
    res/property_tree_plus.xpm    
    )
source_group(res FILES ${RESOURCES})
set(SOURCES	${SOURCES} ${RESOURCES})

add_library("property-tree" ${SOURCES})
add_definitions(-DGENERIC_EDITOR_EXPORTED)

set_target_properties("property-tree" PROPERTIES DEBUG_POSTFIX "-debug")
target_link_libraries("property-tree" "utils" "yasli")

#install(TARGETS "property-tree" DESTINATION ..)
