project(QPropertyTree)
set(CMAKE_AUTOMOC TRUE)

find_package(Qt4 REQUIRED)
file(GLOB SOURCES *.h *.cpp)
add_library(qpropertytree ${SOURCES})
source_group("" FILES ${SOURCES})
include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})
target_link_libraries(qpropertytree yasli ${QT_LIBRARIES})
set_target_properties(qpropertytree PROPERTIES DEBUG_POSTFIX "-debug")