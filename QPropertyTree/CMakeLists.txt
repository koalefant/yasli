project(QPropertyTree)
set(CMAKE_AUTOMOC TRUE)

if (UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC")
endif (UNIX)

if (MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4996")
endif(MSVC)

if (NOT QT4_FOUND AND NOT Qt5_INCLUDE_DIRS)
	find_package(Qt5Widgets)
	include_directories(${Qt5Widgets_INCLUDE_DIRS})
	add_definitions(${Qt5Widgets_DEFINITIONS})
endif()

set(SOURCES
	Color.cpp
	ConstStringList.cpp
	PropertyDrawContext.cpp
	PropertyIArchive.cpp
	PropertyOArchive.cpp
	PropertyRow.cpp
	PropertyRowBool.cpp
	PropertyRowButton.cpp
	PropertyRowContainer.cpp
	PropertyRowColor.cpp
	PropertyRowField.cpp
	PropertyRowFileOpen.cpp
	PropertyRowFileOpen.h
	PropertyRowFileSave.cpp
	PropertyRowIconXPM.cpp
	PropertyRowNumber.cpp
	PropertyRowNumberField.cpp
	PropertyRowObject.cpp
	PropertyRowPointer.cpp
	PropertyRowString.cpp
	PropertyRowStringListValue.cpp
	PropertyTreeModel.cpp
	PropertyTreeOperator.cpp
	QPropertyTree.cpp
	Unicode.cpp
	Color.h
	ConstStringList.h
	Factory.h
	MathUtils.h
	PropertyDrawContext.h
	PropertyIArchive.h
	PropertyOArchive.h
	PropertyRow.h
	PropertyRowBool.h
	PropertyRowColor.h
	PropertyRowContainer.h
	PropertyRowField.h
	PropertyRowImpl.h
	PropertyRowNumber.h
	PropertyRowNumberField.h
	PropertyRowObject.h
	PropertyRowPointer.h
	PropertyRowString.h
	PropertyRowStringListValue.h
	PropertyTreeMenuHandler.h
	PropertyTreeModel.h
	PropertyTreeOperator.h
	QPropertyTree.h
	Serialization.h
	Unicode.h
	)
include_directories(..)
add_library(qpropertytree ${SOURCES})
source_group("" FILES ${SOURCES})
target_link_libraries(qpropertytree yasli ${Qt5Widgets_LIBRARIES})
set_target_properties(qpropertytree PROPERTIES DEBUG_POSTFIX "-debug")
