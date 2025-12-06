#include <QtTest>
#include <QFile>
#include <memory>
#include "../include/SimpleMapView.h"

class SimpleMapViewTest : public QObject
{
    Q_OBJECT

private slots:
    void test_Instantiation()
    {
        auto map = std::make_unique<SimpleMapView>();

        QVERIFY2(map != nullptr, "SimpleMapView widget failed to instantiate.");

        QVERIFY2(map->tileServer() == TileServers::OSM, "Default `tileServer` should be OSM.");
        QVERIFY2(map->tileServerSource() == SimpleMapView::TileServerSource::Remote, "Tile server source for OSM should be `Remote`");

        QVERIFY2(!map->isZoomLocked(), "Default `isZoomLocked` should be false");
        QVERIFY2(!map->isGeolocationLocked(), "Default `isGeolocationLocked` should be false");
        QVERIFY2(!map->isMouseWheelZoomDisabled(), "Default `isMouseWheelZoomDisabled` should be false");
        QVERIFY2(!map->isMouseMoveMapDisabled(), "Default `isMouseMoveMapDisabled` should be false");

        QVERIFY2(!map->markerIcon().isNull(), "Default marker icon should not be null");
        QVERIFY2(QFile::exists(":/SimpleMapView/marker.svg"), "Marker resource not found.");
        QVERIFY2(QFile::exists(":/SimpleMapView/marker_alt.svg"), "Alternative marker resource not found.");

        QImage expectedDefaultMarkerIcon;
        QVERIFY2(
            expectedDefaultMarkerIcon.load(":/SimpleMapView/marker.svg"),
            QString("Failed to load expected image from resource path: %1").arg(":/SimpleMapView/marker.svg").toUtf8());
        QVERIFY2(
            map->markerIcon() == expectedDefaultMarkerIcon,
            "Default marker icon should be `:/SimpleMapView/marker.svg`.");
    }

    void test_ZoomLevel()
    {
        constexpr int minZoomLevel = 5;
        constexpr int maxZoomLevel = 15;
        constexpr int wheelTestZoomLevel = 10;
        constexpr int wheelTestCount = 3;

        SimpleMapView map;
        map.resize(1024, 768);

        QSignalSpy spy(&map, &SimpleMapView::zoomLevelChanged);
        QVERIFY(spy.isValid());

        map.setZoomLevel(10);
        QVERIFY2(map.zoomLevel() == 10, "Failed to set the zoom level.");
        QCOMPARE(spy.count(), 1);

        map.setMinZoomLevel(minZoomLevel);
        QVERIFY2(map.minZoomLevel() == minZoomLevel, "Failed to set the minimum zoom level.");

        map.setMaxZoomLevel(maxZoomLevel);
        QVERIFY2(map.maxZoomLevel() == maxZoomLevel, "Failed to set the maximum zoom level.");

        map.setZoomLevel(minZoomLevel - 1);
        QVERIFY2(map.zoomLevel() == minZoomLevel, "Failed to clamp the zoom level to minimum.");

        map.setZoomLevel(maxZoomLevel + 1);
        QVERIFY2(map.zoomLevel() == maxZoomLevel, "Failed to clamp the zoom level to maximum.");

        QWheelEvent wheelDownEvent(
            map.rect().center(),
            map.mapToGlobal(map.rect().center()),
            QPoint(),
            QPoint(0, -120),
            Qt::NoButton,
            Qt::NoModifier,
            Qt::NoScrollPhase,
            false
        );

        map.setZoomLevel(wheelTestZoomLevel);
        for (size_t i = 1; i <= wheelTestCount; ++i)
        {
            QApplication::sendEvent(&map, &wheelDownEvent);
            QVERIFY2(map.zoomLevel() == (wheelTestZoomLevel - i), "Failed to zoom-out using mouse wheel.");
        }


        QWheelEvent wheelUpEvent(
            map.rect().center(),
            map.mapToGlobal(map.rect().center()),
            QPoint(),
            QPoint(0, 120),
            Qt::NoButton,
            Qt::NoModifier,
            Qt::NoScrollPhase,
            false
        );

        map.setZoomLevel(wheelTestZoomLevel);
        spy.clear();
        for (size_t i = 1; i <= wheelTestCount; ++i)
        {
            QApplication::sendEvent(&map, &wheelUpEvent);
            QVERIFY2(map.zoomLevel() == (wheelTestZoomLevel + i), "Failed to zoom-in using mouse wheel.");
            QCOMPARE(spy.count(), i);
        }

        map.setZoomLevel(5);
        spy.clear();
        map.lockZoom();
        map.setZoomLevel(12);
        QVERIFY2(map.zoomLevel() == 5, "Failed to lock zoom.");
        QCOMPARE(spy.count(), 0);
        map.unlockZoom();
        map.setZoomLevel(12);
        QCOMPARE(spy.count(), 1);
        QVERIFY2(map.zoomLevel() == 12, "Failed to unlock zoom.");

        map.disableMouseWheelZoom();
        QVERIFY2(map.isMouseWheelZoomDisabled(), "Failed to disable mouse wheel zoom.");
        map.setZoomLevel(wheelTestZoomLevel);
        spy.clear();
        for (size_t i = 1; i <= wheelTestCount; ++i)
        {
            QApplication::sendEvent(&map, &wheelUpEvent);
            QVERIFY2(map.zoomLevel() == wheelTestZoomLevel, "Failed to disable mouse wheel zoom.");
        }
        map.enableMouseWheelZoom();
        QVERIFY2(!map.isMouseWheelZoomDisabled(), "Failed to enable mouse wheel zoom.");
        QCOMPARE(spy.count(), 0);
    }

    void test_Positioning()
    {
        constexpr int delta = 5;
        constexpr int mouseMoveCount = 60;
        const QGeoCoordinate expectedCoordinate(39.749656173120805, 30.476754483329955);

        SimpleMapView map;
        map.resize(1024, 768);

        QSignalSpy spy(&map, &SimpleMapView::centerChanged);
        QVERIFY(spy.isValid());

        map.setLatitude(expectedCoordinate.latitude());
        QVERIFY2(map.latitude() == expectedCoordinate.latitude(), "Failed to set latitude.");
        QCOMPARE(spy.count(), 1);

        map.setLongitude(expectedCoordinate.longitude());
        QVERIFY2(map.longitude() == expectedCoordinate.longitude(), "Failed to set longitude.");
        QCOMPARE(spy.count(), 2);

        map.setCenter(10, 20);
        QVERIFY2(map.latitude() == 10, "Failed to set latitude.");
        QVERIFY2(map.longitude() == 20, "Failed to set longitude.");
        QCOMPARE(spy.count(), 3);

        map.setCenter(expectedCoordinate);
        QVERIFY2(map.center() == expectedCoordinate, "Failed to set center.");

        auto mapScreenCenter = map.rect().center();
        QTest::mousePress(&map, Qt::LeftButton, Qt::KeyboardModifiers(), mapScreenCenter);
        qreal lastLong = map.longitude();
        qreal lastLat = map.latitude();
        spy.clear();
        for (size_t i = 1; i <= mouseMoveCount; ++i)
        {
            QTest::mouseMove(&map, mapScreenCenter - QPoint(delta * i, 0));
            QVERIFY2((map.longitude() > lastLong), "Failed to move map via mouse.");
            QVERIFY2(map.latitude() == lastLat, "Invalid map movement via mouse.");
            QCOMPARE(spy.count(), i);
            lastLong = map.longitude();
            lastLat = map.latitude();
        }
        QTest::mouseRelease(&map, Qt::LeftButton);

        QTest::mousePress(&map, Qt::LeftButton, Qt::KeyboardModifiers(), mapScreenCenter);
        lastLong = map.longitude();
        lastLat = map.latitude();
        for (size_t i = 1; i <= mouseMoveCount; ++i)
        {
            QTest::mouseMove(&map, mapScreenCenter + QPoint(0, delta * i));
            QVERIFY2(map.longitude() == lastLong, "Failed to move map via mouse.");
            QVERIFY2((map.latitude() > lastLat), "Invalid map movement via mouse.");
            lastLong = map.longitude();
            lastLat = map.latitude();
        }
        QTest::mouseRelease(&map, Qt::LeftButton);

        map.setLatitude(10.0);
        spy.clear();
        map.lockGeolocation();
        map.setLatitude(20.0);
        QVERIFY2(map.latitude() == 10.0, "Failed to lock geolocation.");
        QCOMPARE(spy.count(), 0);
        map.unlockGeolocation();
        map.setLatitude(20.0);
        QCOMPARE(spy.count(), 1);
        QVERIFY2(map.latitude() == 20.0, "Failed to unlock geolocation.");

        map.setLongitude(10.0);
        spy.clear();
        map.lockGeolocation();
        map.setLongitude(20.0);
        QVERIFY2(map.longitude() == 10.0, "Failed to lock geolocation.");
        QCOMPARE(spy.count(), 0);
        map.unlockGeolocation();
        map.setLongitude(20.0);
        QCOMPARE(spy.count(), 1);
        QVERIFY2(map.longitude() == 20.0, "Failed to unlock geolocation.");

        map.setCenter(10, 20);
        spy.clear();
        map.lockGeolocation();
        map.setCenter(30, 40);
        QVERIFY2(map.center() == QGeoCoordinate(10, 20), "Failed to lock geolocation.");
        QCOMPARE(spy.count(), 0);
        map.unlockGeolocation();
        map.setCenter(30, 40);
        QCOMPARE(spy.count(), 1);
        QVERIFY2(map.center() == QGeoCoordinate(30, 40), "Failed to unlock geolocation.");

        QTest::mousePress(&map, Qt::LeftButton, Qt::KeyboardModifiers(), mapScreenCenter);
        map.disableMouseMoveMap();
        QVERIFY2(map.isMouseMoveMapDisabled(), "Failed to disable moving map via mouse.");
        QGeoCoordinate lastCenter = map.center();
        spy.clear();
        for (size_t i = 1; i <= mouseMoveCount; ++i)
        {
            QTest::mouseMove(&map, mapScreenCenter - QPoint(delta * i, 0));
            QVERIFY2(map.center() == lastCenter, "Failed to disable moving map via mouse.");
            lastCenter = map.center();
            QCOMPARE(spy.count(), 0);
        }
        map.enableMouseMoveMap();
        QVERIFY2(!map.isMouseMoveMapDisabled(), "Failed to enable moving map via mouse.");
        QTest::mouseRelease(&map, Qt::LeftButton);
    }

    void test_TileServers()
    {
        constexpr const char* invalidTileMapUrl = "https://asfdfdsa";

        SimpleMapView map;
        map.resize(1024, 768);

        QSignalSpy spy(&map, &SimpleMapView::tileServerChanged);
        QVERIFY(spy.isValid());

        QVERIFY2(map.tileServer() == TileServers::OSM, "Invalid tile server.");

        map.setTileServer(invalidTileMapUrl);
        QVERIFY2(map.tileServer() == TileServers::OSM, "Invalid tile server.");

        QTest::qWait(5000);
        QCOMPARE(spy.count(), 1);

        map.setTileServer(TileServers::GOOGLE_MAP);
        QVERIFY2(map.tileServer() == TileServers::GOOGLE_MAP, "Failed to change the tile server.");
        QCOMPARE(spy.count(), 2);

        map.setTileServer({ invalidTileMapUrl, TileServers::INVALID, TileServers::OSM });
        QVERIFY2(map.tileServer() == TileServers::OSM, "Failed to change the tile server.");
        QCOMPARE(spy.count(), 3);

        map.setTileServer({ TileServers::GOOGLE_MAP, TileServers::OSM });
        QVERIFY2(map.tileServer() == TileServers::GOOGLE_MAP, "Failed to change the tile server.");
        QCOMPARE(spy.count(), 4);
    }

    void test_Marker()
    {
        {
            SimpleMapView map;
            map.resize(1024, 768);

            MapImage* marker = map.addMarker(map.latitude(), map.longitude());
            QVERIFY2(marker != nullptr, "Failed to add marker.");

            QImage expectedMarkerIcon;
            QVERIFY2(
                expectedMarkerIcon.load(":/SimpleMapView/marker.svg"),
                QString("Failed to load expected image from resource path: %1").arg(":/SimpleMapView/marker.svg").toUtf8());
            QVERIFY2(
                marker->image() == expectedMarkerIcon,
                "Default marker icon should be `:/SimpleMapView/marker.svg`.");
            QVERIFY2(
                map.markerIcon() == expectedMarkerIcon,
                "Default marker icon should be `:/SimpleMapView/marker.svg`.");
        }

        {
            SimpleMapView map;
            map.resize(1024, 768);

            map.setMarkerIcon(":/SimpleMapView/marker_alt.svg");
            MapImage* marker = map.addMarker(map.center());
            QVERIFY2(marker != nullptr, "Failed to add marker.");

            QImage expectedMarkerIcon;
            QVERIFY2(
                expectedMarkerIcon.load(":/SimpleMapView/marker_alt.svg"),
                QString("Failed to load expected image from resource path: %1").arg(":/SimpleMapView/marker_alt.svg").toUtf8());
            QVERIFY2(
                marker->image() == expectedMarkerIcon,
                "Failed to change marker icon.");
            QVERIFY2(
                map.markerIcon() == expectedMarkerIcon,
                "Failed to change marker icon.");
        }

        {
            SimpleMapView map;
            map.resize(1024, 768);

            MapImage* marker = map.addMarker(map.center());
            QVERIFY2(marker != nullptr, "Failed to add marker.");
            QVERIFY2(marker->findChild<MapText*>() != nullptr, "Failed to add marker text.");
        }
    }
};

QTEST_MAIN(SimpleMapViewTest)
#include "SimpleMapViewTest.moc"