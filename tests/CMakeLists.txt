cmake_minimum_required(VERSION 3.16)
project(SimpleMapViewTests LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Test)

add_executable(
    SimpleMapViewTests
    ../Resources.qrc
    SimpleMapViewTest.cpp
)

target_link_libraries(
    SimpleMapViewTests
    SimpleMapView
    Qt::Test
)

include(CTest)
add_test(NAME SimpleMapViewTests COMMAND SimpleMapViewTests)