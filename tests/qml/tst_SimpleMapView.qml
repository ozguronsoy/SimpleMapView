import QtQuick
import QtTest
import com.github.ozguronsoy.SimpleMapView

TestCase {
    name: "SimpleMapViewTests"
    width: 1024
    height: 768
    visible: true
    when: windowShown

    SimpleMapView {
        id: map
        anchors.fill: parent
        focus: true
    }

    SignalSpy { id: zoomSpy; target: map; signalName: "zoomLevelChanged" }
    SignalSpy { id: centerSpy; target: map; signalName: "centerChanged" }
    SignalSpy { id: tileServerSpy; target: map; signalName: "tileServerChanged" }

    function test_instantiation() {
        verify(map !== null, "SimpleMapView widget failed to instantiate.")
        
        compare(map.tileServer, TileServers.OSM, "Default `tileServer` should be OSM.")
        compare(map.tileServerSource(), SimpleMapView.Remote, "Default source should be Remote.")

        verify(!map.isZoomLocked(), "Default `isZoomLocked` should be false")
        verify(!map.isGeolocationLocked(), "Default `isGeolocationLocked` should be false")
        verify(!map.isMouseWheelZoomDisabled(), "Default `isMouseWheelZoomDisabled` should be false")
        verify(!map.isMouseMoveMapDisabled(), "Default `isMouseMoveMapDisabled` should be false")
    }

    function test_zoomLevel() {
        var minZoom = 5
        var maxZoom = 15
        var wheelTestLevel = 10
        
        zoomSpy.clear()

        map.zoomLevel = 10
        compare(map.zoomLevel, 10, "Failed to set zoom level.")
        compare(zoomSpy.count, 1)

        map.minZoomLevel = minZoom
        map.maxZoomLevel = maxZoom
        
        map.zoomLevel = minZoom - 1
        compare(map.zoomLevel, minZoom, "Failed to clamp to minimum.")
        
        map.zoomLevel = maxZoom + 1
        compare(map.zoomLevel, maxZoom, "Failed to clamp to maximum.")

        map.zoomLevel = wheelTestLevel
        
        mouseWheel(map, map.width/2, map.height/2, -120, -120)
        verify(map.zoomLevel < wheelTestLevel, "Failed to zoom-out using mouse wheel.")

        var preZoom = map.zoomLevel
        mouseWheel(map, map.width/2, map.height/2, 120, 120)
        verify(map.zoomLevel > preZoom, "Failed to zoom-in using mouse wheel.")

        map.zoomLevel = 5
        zoomSpy.clear()
        
        map.lockZoom = true
        map.zoomLevel = 12
        compare(map.zoomLevel, 5, "Failed to lock zoom.")
        compare(zoomSpy.count, 0)
        
        map.lockZoom = false
        map.zoomLevel = 12
        compare(map.zoomLevel, 12, "Failed to unlock zoom.")
        compare(zoomSpy.count, 1)
        
        map.disableMouseWheelZoom = true
        verify(map.isMouseWheelZoomDisabled(), "Failed to disable mouse wheel zoom.")
        
        preZoom = map.zoomLevel
        mouseWheel(map, map.width/2, map.height/2, 120, 120)
        compare(map.zoomLevel, preZoom, "Mouse wheel should be ignored when disabled.")
        
        map.disableMouseWheelZoom = false
        verify(!map.isMouseWheelZoomDisabled(), "Failed to enable mouse wheel zoom.")
    }

    function test_positioning() {
        var expectedLat = 39.749656
        var expectedLon = 30.476754
        
        centerSpy.clear()

        map.latitude = expectedLat
        compare(map.latitude, expectedLat, "Failed to set latitude.")
        compare(centerSpy.count, 1)

        map.longitude = expectedLon
        compare(map.longitude, expectedLon, "Failed to set longitude.")
        compare(centerSpy.count, 2)

        map.setCenter(10, 20)
        compare(map.latitude, 10, "Failed to set center lat.")
        compare(map.longitude, 20, "Failed to set center lon.")
        
        var startX = map.width / 2
        var startY = map.height / 2
        var initialLon = map.longitude
        var initialLat = map.latitude
        
        mousePress(map, startX, startY, Qt.LeftButton)
        mouseMove(map, startX - 100, startY, 0, Qt.LeftButton)
        mouseRelease(map, startX - 100, startY, Qt.LeftButton)

        verify(map.longitude > initialLon, "Failed to move map (Longitude) via mouse.")
        verify(map.latitude === initialLat, "Latitude should remain constant on horizontal drag.")

        map.longitude = initialLon
        map.latitude = initialLat

        mousePress(map, startX, startY, Qt.LeftButton)
        mouseMove(map, startX, startY + 100, 0, Qt.LeftButton)
        mouseRelease(map, startX, startY + 100, Qt.LeftButton)

        verify(map.longitude === initialLon, "Failed to move map (Longitude) via mouse.")
        verify(map.latitude > initialLat, "Latitude should remain constant on horizontal drag.")

        map.latitude = 10.0
        centerSpy.clear()
        
        map.lockGeolocation = true
        map.latitude = 20.0
        compare(map.latitude, 10.0, "Failed to lock geolocation.")
        compare(centerSpy.count, 0)
        
        map.lockGeolocation = false
        map.latitude = 20.0
        compare(map.latitude, 20.0, "Failed to unlock geolocation.")
        
        map.disableMouseMoveMap = true
        verify(map.isMouseMoveMapDisabled(), "Failed to flag mouse move disabled.")
        
        var frozenCenter = map.center
        mousePress(map, startX, startY, Qt.LeftButton)
        mouseMove(map, startX - 100, startY + 100, 0, Qt.LeftButton)
        mouseRelease(map, startX - 100, startY + 100, Qt.LeftButton)
        compare(map.center, frozenCenter, "Map should not move when mouse move is disabled.")
        
        map.disableMouseMoveMap = false
        verify(!map.isMouseMoveMapDisabled(), "Failed to flag mouse move enabled.")
    }

    function test_tileServers() {
        var invalidUrl = "https://asfdfdsa"
        
        tileServerSpy.clear()
        
        map.setTileServer(invalidUrl)
        wait(5000)
        
        compare(map.tileServer, TileServers.OSM, "Should default to OSM on invalid server.")

        map.setTileServer(TileServers.GOOGLE_MAP)
        compare(map.tileServer, TileServers.GOOGLE_MAP, "Failed to change tile server.")
    }

    function test_markers() {
        var marker = map.addMarker(map.latitude, map.longitude)
        verify(marker !== null, "Failed to add marker.")
    }
}