project("YasliPropertyTree")
set(LIBRARY_NAME "property-tree")

file(GLOB SOURCES *.cpp *.h)
source_group("" FILES ${SOURCES})

set(RESOURCES
    res/property_item_combo_button.xpm
    res/property_item_file_selector_button.xpm
    res/property_item_library_selector_button.xpm
    res/property_tree_minus.xpm
    res/property_tree_plus.xpm    
    )
source_group(res FILES ${RESOURCES})
set(SOURCES	${SOURCES} ${RESOURCES})

add_library(${LIBRARY_NAME} ${SOURCES})
add_definitions(-DGENERIC_EDITOR_EXPORTED)

set_target_properties(${LIBRARY_NAME} PROPERTIES DEBUG_POSTFIX "-debug")
target_link_libraries(${LIBRARY_NAME} "utils" "yasli")

#install(TARGETS ${LIBRARY_NAME} DESTINATION ..)
