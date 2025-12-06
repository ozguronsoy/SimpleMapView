import pytest
from PySimpleMapView import SimpleMapView, TileServers, MapText
from PySide6.QtCore import Qt, QPoint, QPointF
from PySide6.QtGui import QImage, QWheelEvent
from PySide6.QtWidgets import QApplication
from PySide6.QtPositioning import QGeoCoordinate
from PySide6.QtTest import QSignalSpy
from PySimpleMapView import SimpleMapView

default_marker_path = ":/SimpleMapView/marker.svg"
alt_marker_path = ":/SimpleMapView/marker_alt.svg"

def test_instantiation(qtbot):
    map_view = SimpleMapView()
    qtbot.addWidget(map_view)

    assert map_view.tileServer() == TileServers.OSM, "Default `tileServer` should be OSM."
    assert map_view.tileServerSource() == SimpleMapView.TileServerSource.Remote, "Tile server source for OSM should be `Remote`"
    
    assert not map_view.isZoomLocked(), "Default `isZoomLocked` should be false"
    assert not map_view.isGeolocationLocked(), "Default `isGeolocationLocked` should be false"
    assert not map_view.isMouseWheelZoomDisabled(), "Default `isMouseWheelZoomDisabled` should be false"
    assert not map_view.isMouseMoveMapDisabled(), "Default `isMouseMoveMapDisabled` should be false"

    expectedDefaultMarkerIcon = QImage()

    assert expectedDefaultMarkerIcon.load(default_marker_path), f"Failed to load expected image from resource path: {default_marker_path}"
    assert map_view.markerIcon() == expectedDefaultMarkerIcon, f"Default marker icon should be `{default_marker_path}`."

def test_zoom_level(qtbot):
    MIN_ZOOM_LEVEL = 5
    MAX_ZOOM_LEVEL = 15
    WHEEL_TEST_ZOOM_LEVEL = 10
    WHEEL_TEST_COUNT = 3

    map_view = SimpleMapView()
    qtbot.addWidget(map_view)
    map_view.resize(1024, 768)

    spy = QSignalSpy(map_view.zoomLevelChanged)
    assert spy.isValid(), "Signal spy is invalid"

    map_view.setZoomLevel(10)
    assert map_view.zoomLevel() == 10, "Failed to set the zoom level."
    assert spy.count() == 1

    map_view.setMinZoomLevel(MIN_ZOOM_LEVEL)
    assert map_view.minZoomLevel() == MIN_ZOOM_LEVEL, "Failed to set the minimum zoom level."

    map_view.setMaxZoomLevel(MAX_ZOOM_LEVEL)
    assert map_view.maxZoomLevel() == MAX_ZOOM_LEVEL, "Failed to set the maximum zoom level."

    map_view.setZoomLevel(MIN_ZOOM_LEVEL - 1)
    assert map_view.zoomLevel() == MIN_ZOOM_LEVEL, "Failed to clamp the zoom level to minimum."

    map_view.setZoomLevel(MAX_ZOOM_LEVEL + 1)
    assert map_view.zoomLevel() == MAX_ZOOM_LEVEL, "Failed to clamp the zoom level to maximum."

    def send_wheel_event(angle_delta_y):
        event = QWheelEvent(
            QPointF(map_view.rect().center()),
            QPointF(map_view.mapToGlobal(map_view.rect().center())),
            QPoint(),
            QPoint(0, angle_delta_y),
            Qt.MouseButton.NoButton,
            Qt.KeyboardModifier.NoModifier,
            Qt.ScrollPhase.NoScrollPhase,
            False
        )
        QApplication.sendEvent(map_view, event)

    map_view.setZoomLevel(WHEEL_TEST_ZOOM_LEVEL)
    for i in range(1, WHEEL_TEST_COUNT + 1):
        send_wheel_event(-120)
        expected_zoom = WHEEL_TEST_ZOOM_LEVEL - i
        assert map_view.zoomLevel() == expected_zoom, "Failed to zoom-out using mouse wheel."

    map_view.setZoomLevel(WHEEL_TEST_ZOOM_LEVEL)
    spy = QSignalSpy(map_view.zoomLevelChanged)
    for i in range(1, WHEEL_TEST_COUNT + 1):
        send_wheel_event(120)
        expected_zoom = WHEEL_TEST_ZOOM_LEVEL + i
        assert map_view.zoomLevel() == expected_zoom, "Failed to zoom-in using mouse wheel."
        assert spy.count() == i

    map_view.setZoomLevel(5)
    spy = QSignalSpy(map_view.zoomLevelChanged)
    map_view.lockZoom()
    map_view.setZoomLevel(12)
    assert map_view.zoomLevel() == 5, "Failed to lock zoom."
    assert spy.count() == 0
    map_view.unlockZoom()
    map_view.setZoomLevel(12)
    assert spy.count() == 1
    assert map_view.zoomLevel() == 12, "Failed to unlock zoom."

    map_view.disableMouseWheelZoom()
    assert map_view.isMouseWheelZoomDisabled(), "Failed to disable mouse wheel zoom."
    map_view.setZoomLevel(WHEEL_TEST_ZOOM_LEVEL)
    spy = QSignalSpy(map_view.zoomLevelChanged)
    for i in range(1, WHEEL_TEST_COUNT + 1):
        send_wheel_event(120)
        assert map_view.zoomLevel() == WHEEL_TEST_ZOOM_LEVEL, "Zoom changed even though wheel is disabled."
    map_view.enableMouseWheelZoom()
    assert not map_view.isMouseWheelZoomDisabled(), "Failed to enable mouse wheel zoom."
    assert spy.count() == 0

def test_positioning(qtbot):
    DELTA = 5
    MOUSE_MOVE_COUNT = 60
    EXPECTED_COORDINATE = QGeoCoordinate(39.749656173120805, 30.476754483329955)

    map_view = SimpleMapView()
    qtbot.addWidget(map_view)
    map_view.resize(1024, 768)

    spy = QSignalSpy(map_view.centerChanged)
    assert spy.isValid()

    # --- Basic Setters ---
    map_view.setLatitude(EXPECTED_COORDINATE.latitude())
    assert map_view.latitude() == EXPECTED_COORDINATE.latitude(), "Failed to set latitude."
    assert spy.count() == 1

    map_view.setLongitude(EXPECTED_COORDINATE.longitude())
    assert map_view.longitude() == EXPECTED_COORDINATE.longitude(), "Failed to set longitude."
    assert spy.count() == 2

    map_view.setCenter(10, 20)
    assert map_view.latitude() == 10, "Failed to set latitude."
    assert map_view.longitude() == 20, "Failed to set longitude."
    assert spy.count() == 3

    map_view.setCenter(EXPECTED_COORDINATE)
    assert map_view.center() == EXPECTED_COORDINATE, "Failed to set center."

    map_screen_center = map_view.rect().center()
    qtbot.mousePress(map_view, Qt.MouseButton.LeftButton, pos=map_screen_center)
    last_long = map_view.longitude()
    last_lat = map_view.latitude()
    spy = QSignalSpy(map_view.centerChanged)
    for i in range(1, MOUSE_MOVE_COUNT + 1):
        target_pos = map_screen_center - QPoint(DELTA * i, 0)
        qtbot.mouseMove(map_view, target_pos)
        assert map_view.longitude() > last_long, "Failed to move map via mouse."
        assert map_view.latitude() == last_lat, "Invalid map movement via mouse."
        assert spy.count() == i
        last_long = map_view.longitude()
        last_lat = map_view.latitude()
    qtbot.mouseRelease(map_view, Qt.MouseButton.LeftButton)

    qtbot.mousePress(map_view, Qt.MouseButton.LeftButton, pos=map_screen_center)
    last_long = map_view.longitude()
    last_lat = map_view.latitude()
    for i in range(1, MOUSE_MOVE_COUNT + 1):
        target_pos = map_screen_center + QPoint(0, DELTA * i)
        qtbot.mouseMove(map_view, target_pos)
        assert map_view.longitude() == last_long, "Failed to move map via mouse."
        assert map_view.latitude() > last_lat, "Invalid map movement via mouse."
        last_long = map_view.longitude()
        last_lat = map_view.latitude()
    qtbot.mouseRelease(map_view, Qt.MouseButton.LeftButton)

    map_view.setLatitude(10.0)
    spy = QSignalSpy(map_view.centerChanged)
    map_view.lockGeolocation()
    map_view.setLatitude(20.0)
    assert map_view.latitude() == 10.0, "Failed to lock geolocation."
    assert spy.count() == 0
    map_view.unlockGeolocation()
    map_view.setLatitude(20.0)
    assert spy.count() == 1
    assert map_view.latitude() == 20.0, "Failed to unlock geolocation."

    map_view.setLongitude(10.0)
    spy = QSignalSpy(map_view.centerChanged)
    map_view.lockGeolocation()
    map_view.setLongitude(20.0)
    assert map_view.longitude() == 10.0, "Failed to lock geolocation."
    assert spy.count() == 0
    map_view.unlockGeolocation()
    map_view.setLongitude(20.0)
    assert spy.count() == 1
    assert map_view.longitude() == 20.0, "Failed to unlock geolocation."

    map_view.setCenter(10, 20)
    spy = QSignalSpy(map_view.centerChanged)
    map_view.lockGeolocation()
    map_view.setCenter(30, 40)
    assert map_view.center() == QGeoCoordinate(10, 20), "Failed to lock geolocation."
    assert spy.count() == 0
    map_view.unlockGeolocation()
    map_view.setCenter(30, 40)
    assert spy.count() == 1
    assert map_view.center() == QGeoCoordinate(30, 40), "Failed to unlock geolocation."

    map_screen_center = map_view.rect().center()
    qtbot.mousePress(map_view, Qt.MouseButton.LeftButton, pos=map_screen_center)
    map_view.disableMouseMoveMap()
    assert map_view.isMouseMoveMapDisabled(), "Failed to disable moving map via mouse."
    last_center = map_view.center()
    spy = QSignalSpy(map_view.centerChanged)
    for i in range(1, MOUSE_MOVE_COUNT + 1):
        target_pos = map_screen_center - QPoint(DELTA * i, 0)
        qtbot.mouseMove(map_view, target_pos)
        assert map_view.center() == last_center, "Failed to disable moving map via mouse."
        last_center = map_view.center()
        assert spy.count() == 0
    map_view.enableMouseMoveMap()
    assert not map_view.isMouseMoveMapDisabled(), "Failed to enable moving map via mouse."
    qtbot.mouseRelease(map_view, Qt.MouseButton.LeftButton)

def test_tile_servers(qtbot):
    INVALID_TILE_MAP_URL = "https://asfdfdsa"

    map_view = SimpleMapView()
    qtbot.addWidget(map_view)
    map_view.resize(1024, 768)

    spy = QSignalSpy(map_view.tileServerChanged)
    assert spy.isValid(), "Signal spy is invalid"

    assert map_view.tileServer() == TileServers.OSM, "Invalid tile server."

    map_view.setTileServer(INVALID_TILE_MAP_URL)
    assert map_view.tileServer() == TileServers.OSM, "Invalid tile server."
    
    qtbot.wait(5000)
    assert spy.count() == 1

    map_view.setTileServer(TileServers.GOOGLE_MAP)
    assert map_view.tileServer() == TileServers.GOOGLE_MAP, "Failed to change the tile server."
    assert spy.count() == 2

    map_view.setTileServer([INVALID_TILE_MAP_URL, TileServers.INVALID, TileServers.OSM])
    assert map_view.tileServer() == TileServers.OSM, "Failed to change the tile server."
    assert spy.count() == 3

    map_view.setTileServer([TileServers.GOOGLE_MAP, TileServers.OSM])
    assert map_view.tileServer() == TileServers.GOOGLE_MAP, "Failed to change the tile server."
    assert spy.count() == 4

def test_marker(qtbot):
    map_view = SimpleMapView()
    qtbot.addWidget(map_view)
    map_view.resize(1024, 768)

    marker = map_view.addMarker(map_view.latitude(), map_view.longitude())
    assert marker is not None, "Failed to add marker."

    expected_marker_icon = QImage()
    assert expected_marker_icon.load(default_marker_path), f"Failed to load expected image from resource path: {default_marker_path}"
    assert marker.image() == expected_marker_icon, f"Default marker icon should be `{default_marker_path}`."
    assert map_view.markerIcon() == expected_marker_icon, f"Default marker icon should be `{default_marker_path}`."

    map_view = SimpleMapView()
    qtbot.addWidget(map_view)
    map_view.resize(1024, 768)

    map_view.setMarkerIcon(alt_marker_path)
    marker = map_view.addMarker(map_view.center())
    assert marker is not None, "Failed to add marker."

    expected_alt_icon = QImage()
    assert expected_alt_icon.load(alt_marker_path), f"Failed to load expected image from resource path: {alt_marker_path}"
    assert marker.image() == expected_alt_icon, "Failed to change marker icon."
    assert map_view.markerIcon() == expected_alt_icon, "Failed to change marker icon."

    map_view = SimpleMapView()
    qtbot.addWidget(map_view)
    map_view.resize(1024, 768)

    marker = map_view.addMarker(map_view.center())
    assert marker is not None, "Failed to add marker."
    assert marker.findChild(MapText) is not None, "Failed to add marker text."