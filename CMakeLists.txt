cmake_minimum_required(VERSION 3.16)

if (CMAKE_CXX_STANDARD LESS 17)
	message(FATAL_ERROR "C++17 or higher is required")
endif()

set(SIMPLE_MAP_VIEW_VERSION_MAJOR 1)
set(SIMPLE_MAP_VIEW_VERSION_MINOR 0)
set(SIMPLE_MAP_VIEW_VERSION_PATCH 0)
set(SIMPLE_MAP_VIEW_VERSION "${SIMPLE_MAP_VIEW_VERSION_MAJOR}.${SIMPLE_MAP_VIEW_VERSION_MINOR}.${SIMPLE_MAP_VIEW_VERSION_PATCH}")
project(SimpleMapView VERSION ${SIMPLE_MAP_VIEW_VERSION} LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Positioning Network)

file(GLOB SIMPLE_MAP_VIEW_SOURCES
	include/*.h
    src/*.cpp

	include/SimpleMapView/*.h
    src/SimpleMapView/*.cpp
)

add_library(SimpleMapView STATIC ${SIMPLE_MAP_VIEW_SOURCES})
target_include_directories(
    SimpleMapView
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
)
target_link_libraries(SimpleMapView
    PUBLIC
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::Positioning
        Qt::Network
)