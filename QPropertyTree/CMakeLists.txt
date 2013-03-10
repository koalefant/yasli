project(QPropertyTree)
set(CMAKE_AUTOMOC TRUE)

find_package(Qt4 REQUIRED)
#file(GLOB SOURCES *.h *.cpp)
set(SOURCES
	Color.cpp
	ConstStringList.cpp
	Icon.cpp
	PropertyDrawContext.cpp
	PropertyIArchive.cpp
	PropertyOArchive.cpp
	PropertyRow.cpp
	PropertyRowBool.cpp
	PropertyRowContainer.cpp
	PropertyRowField.cpp
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
	Icon.h
	MathUtils.h
	PropertyDrawContext.h
	PropertyIArchive.h
	PropertyOArchive.h
	PropertyRow.h
	PropertyRowBool.h
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
add_library(qpropertytree ${SOURCES})
source_group("" FILES ${SOURCES})
include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})
target_link_libraries(qpropertytree yasli ${QT_LIBRARIES})
set_target_properties(qpropertytree PROPERTIES DEBUG_POSTFIX "-debug")
