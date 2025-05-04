QT *= core
QT *= gui
QT *= widgets
QT *= network
QT *= positioning

INCLUDEPATH += $$PWD/src

HEADERS += $$files($$PWD/src/*.h)
SOURCES += $$files($$PWD/src/*.cpp)

!isEmpty(SIMPLE_MAP_VIEW_ENABLE_RESOURCES) {
    RESOURCES += $$PWD/Resources.qrc
}

