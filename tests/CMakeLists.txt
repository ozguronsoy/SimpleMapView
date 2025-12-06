cmake_minimum_required(VERSION 3.16)
project(SimpleMapViewTests LANGUAGES CXX)

if(SIMPLE_MAP_VIEW_BUILD_QML)

    find_package(Qt6 REQUIRED COMPONENTS QuickTest)
    add_executable(
        SimpleMapViewTests
        ../Resources.qrc
        qml/SimpleMapViewTest.cpp
    )
    target_compile_definitions(
        SimpleMapViewTests PRIVATE 
        QUICK_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}" 
        QML2_IMPORT_PATH="${CMAKE_BINARY_DIR}"
    )
    target_link_libraries(
        SimpleMapViewTests PRIVATE 
        SimpleMapViewplugin 
        Qt::QuickTest
    )

else()

    find_package(Qt6 REQUIRED COMPONENTS Test)
    add_executable(
        SimpleMapViewTests
        ../Resources.qrc
        widgets/SimpleMapViewTest.cpp
    )
    target_link_libraries(
        SimpleMapViewTests PRIVATE 
        SimpleMapView
        Qt::Test
    )

endif()

include(CTest)
add_test(NAME SimpleMapViewTests COMMAND SimpleMapViewTests)