# makefile
project("YasliExample")

# переменная EXE (имя exe-шника)
set(EXE "example")

# ala build-configuration
set(CMAKE_BUILD_TYPE "Debug") # MinSizeRel

# we need this to find our Testo.cmake
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/Testo)

set(PROJECT_SOURCE_ROOT ${PROJECT_SOURCE_DIR})

# set(wxWidgets_USE_LIBS base core)
# find_package(wxWidgets)

set(SOURCES
    Example.cpp
    )

if(CMAKE_COMPILER_IS_GNUCC)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fmessage-length=80 ${WARNINGS}")
endif(CMAKE_COMPILER_IS_GNUCC)

if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmessage-length=80 ${WARNINGS}")
  set(WARNINGS "-Wall -Wformat -Winit-self -Wimplicit -Wparentheses -Wmissing-braces -Wsequence-point -Wreturn-type -Wswitch -Wunused-function -Wunused-label -Wunknown-pragmas -Wstrict-aliasing -Wundef -Wunsafe-loop-optimizations -Wpointer-arith -Wconversion -Wsign-compare -Wmissing-field-initializers -Wmissing-field-initializers -Winline -Winvalid-pch ")
endif(CMAKE_COMPILER_IS_GNUCXX)

add_executable(${EXE} ${SOURCES})

# place binaries in project root
# install(TARGETS ${EXE}
#         DESTINATION "${PROJECT_SOURCE_ROOT}")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_ROOT})
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_ROOT})

# append "-debug"
set_target_properties(${EXE} PROPERTIES DEBUG_POSTFIX "-debug")

# includes
include_directories(.)

# libraries
# if(wxWidgets_FOUND)
#     add_definitions(-DENABLE_EDITOR -DwxUSE_GLCANVAS=1 ${wxWidgets_DEFINITIONS})
#     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${wxWidgets_CXX_FLAGS}")
# 
#     include_directories(${wxWidgets_INCLUDE_DIRS})
# 
#     add_subdirectory(editor)
#     add_subdirectory(libs/generic-editor)
# 
#     target_link_libraries(${EXE} ${wxWidgets_LIBRARIES})
# endif(wxWidgets_FOUND)

add_subdirectory(testo)
target_link_libraries(${EXE} "testo")

add_subdirectory(utils)
target_link_libraries(${EXE} "utils")

add_subdirectory(yasli)
target_link_libraries(${EXE} "yasli")
