cmake_minimum_required(VERSION 3.16)

if (CMAKE_CXX_STANDARD LESS 17)
	message(FATAL_ERROR "C++17 or higher is required")
endif()

find_package(Qt NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR}
    COMPONENTS
    Core
    Gui
    Widgets
    Positioning 
    Network
)

file(GLOB SIMPLE_MAP_VIEW_SOURCES

	${CMAKE_CURRENT_LIST_DIR}/src/*.h
    ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/SimpleMapView/*.h
    ${CMAKE_CURRENT_LIST_DIR}/src/SimpleMapView/*.cpp
)

if(DEFINED SIMPLE_MAP_VIEW_ENABLE_RESOURCES)
	
	set(CMAKE_AUTORCC ON CACHE BOOL "SimpleMapView Compile Qrc")
	file(GLOB SIMPLE_MAP_VIEW_SOURCES 
		${SIMPLE_MAP_VIEW_SOURCES} 
		${CMAKE_CURRENT_LIST_DIR}/Resources.qrc
	)

endif()

set(SIMPLE_MAP_VIEW_SOURCES ${SIMPLE_MAP_VIEW_SOURCES} CACHE STRING "source files of SimpleMapView")

set(SIMPLE_MAP_VIEW_LIBRARIES 
	Qt::Core
	Qt::Gui
	Qt::Widgets
	Qt::Positioning
	Qt::Network
	CACHE STRING "SimpleMapView required libraries"
)