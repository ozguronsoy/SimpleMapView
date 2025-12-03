cmake_minimum_required(VERSION 3.16)
project(PySimpleMapView)

find_package(Python 3.8 COMPONENTS Interpreter Development REQUIRED)

execute_process(
    COMMAND "${Python_EXECUTABLE}" -c 
    "import os, shiboken6_generator; print(os.path.dirname(shiboken6_generator.__file__), end='')"
    OUTPUT_VARIABLE SHIBOKEN6_GENERATOR_DIR
    RESULT_VARIABLE SHIBOKEN6_GENERATOR_FOUND
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(NOT "${SHIBOKEN6_GENERATOR_FOUND}" EQUAL "0")
    message(FATAL_ERROR "Could not find 'shiboken6_generator'. Run: pip install shiboken6_generator")
endif()

execute_process(
    COMMAND "${Python_EXECUTABLE}" -c 
    "import os, PySide6; print(os.path.dirname(PySide6.__file__), end='')"
    OUTPUT_VARIABLE PYSIDE6_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

file(TO_CMAKE_PATH "${SHIBOKEN6_GENERATOR_DIR}" SHIBOKEN6_GENERATOR_DIR)
file(TO_CMAKE_PATH "${PYSIDE6_DIR}" PYSIDE6_DIR)
list(APPEND CMAKE_PREFIX_PATH "${SHIBOKEN6_GENERATOR_DIR}" "${PYSIDE6_DIR}")

find_package(Shiboken6Tools REQUIRED)

set(
    generated_sources
    "${CMAKE_CURRENT_BINARY_DIR}/PySimpleMapView/pysimplemapview_module_wrapper.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/PySimpleMapView/simplemapview_wrapper.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/PySimpleMapView/mapitem_wrapper.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/PySimpleMapView/mapellipse_wrapper.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/PySimpleMapView/maprect_wrapper.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/PySimpleMapView/mapimage_wrapper.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/PySimpleMapView/maptext_wrapper.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/PySimpleMapView/maplines_wrapper.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/PySimpleMapView/mappolygon_wrapper.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/PySimpleMapView/mappoint_wrapper.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/PySimpleMapView/mapsize_wrapper.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/PySimpleMapView/tileservers_wrapper.cpp"
)

shiboken_generator_create_binding(
    EXTENSION_TARGET PySimpleMapView
    GENERATED_SOURCES ${generated_sources}
    HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/bindings.h"
    TYPESYSTEM_FILE "${CMAKE_CURRENT_SOURCE_DIR}/simple-map-view.typesystem"
    LIBRARY_TARGET SimpleMapView
    QT_MODULES ${SIMPLE_MAP_VIEW_QT_COMPONENTS}
    SHIBOKEN_EXTRA_OPTIONS
        "-I${CMAKE_CURRENT_SOURCE_DIR}"
        "-I${CMAKE_CURRENT_SOURCE_DIR}/../include"
)

target_sources(PySimpleMapView PRIVATE ${generated_sources})
install(TARGETS PySimpleMapView DESTINATION PySimpleMapView)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/PySimpleMapView/__init__.pyi" DESTINATION PySimpleMapView)