QT *= core
QT *= gui
QT *= widgets
QT *= network
QT *= positioning

INCLUDEPATH += $$PWD/src

HEADERS += $$files($$PWD/src/*.h) $$files($$PWD/src/SimpleMapView/*.h)
SOURCES += $$files($$PWD/src/*.cpp) $$files($$PWD/src/SimpleMapView/*.cpp)

!isEmpty(SIMPLE_MAP_VIEW_ENABLE_RESOURCES) {
    RESOURCES += $$PWD/Resources.qrc
}

