# SimpleMapView

A Qt widget for rendering tile maps.

- [Setup](#setup)
- [Map Widget](#map-widget)
    - [Create Widget](#create-widget)
    - [Change Tile Server](#change-tile-server)
    - [Limit Zoom](#limit-zoom)
    - [Lock Zoom and Geolocation](#lock-zoom-and-geolocation)
    - [Disable Mouse Events](#disable-mouse-events)
- [Map Items](#map-items)
    - [Ellipse](#ellipse)
    - [Rect](#rect)
    - [Text](#text)
    - [Image](#image)
    - [Lines](#lines)
    - [Polygon](#polygon)
- [Markers](#markers)
    - [Add Marker](#add-marker)
    - [Change Default Marker Icon](#change-default-marker-icon)
- [QML](#qml)
- [Using Offline Maps](#using-offline-maps)

## Setup

1. create a folder in your project (e.g., ``dependencies/SimpleMapView``) and copy the repo files to the folder.
2. include ``SimpleMapView`` in your CMake or qmake file. 


CMake:
```cmake
find_package(Qt6 REQUIRED COMPONENTS Core)
qt_standard_project_setup()

set(SIMPLE_MAP_VIEW_USE_QML ON) # if QML app
add_subdirectory(dependencies/SimpleMapView)

set(CMAKE_AUTORCC ON)
set(PROJECT_SOURCES
    # your files
    dependencies/SimpleMapView/Resources.qrc # optional
)

target_link_libraries(mytarget PUBLIC
    # your libraries
    SimpleMapView
)
```

qmake:
```
SIMPLE_MAP_VIEW_ENABLE_RESOURCES = 1 # optional
SIMPLE_MAP_VIEW_USE_QML = 1 # if QML app
include(dependencies/SimpleMapView/SimpleMapView.pro)
```

## Map Widget

### Create Widget

create the widget inside the main window's constructor, then set the zoom level and the center coordinates to the place you want to display.

```c++
SimpleMapView* mapView = new SimpleMapView(this);
mapView->move(0, 0);
mapView->resize(this->width(), this->height()); // full screen

mapView->setCenter(41.010172, 28.957912); // İstanbul, Türkiye
mapView->setZoomLevel(9);
```

![default_map](readme_images/map.png)

### Change Tile Server

you can use any tile server that contains ``{x}``, ``{y}``, and ``{z}`` coordinates in the URL.

```c++
mapView->setTileServer(TileServers::GOOGLE_MAP);
mapView->setTileServer(TileServers::GOOGLE_SAT);
mapView->setTileServer("https://a.tile.maptiler.com/{z}/{x}/{y}.png?key=YOUR_API_KEY");
``` 
![satellite_map](readme_images/map_satellite.png)

### Limit Zoom

you can set limit (min/max) to zoom level.
```c++
mapView->setMinZoomLevel(10);
mapView->setMaxZoomLevel(17);
```

### Lock Zoom and Geolocation

```c++
mapView->lockZoom();
mapView->unlockZoom();

mapView->lockGeolocation();
mapView->unlockGeolocation();
```

### Disable Mouse Events

by default, you can move the map by holding the left mouse button down and moving the mouse. And you can zoom in/out via the mouse wheel.

```c++
mapView->disableMouseWheelZoom();
mapView->enableMouseWheelZoom();

mapView->disableMouseMoveMap();
mapView->enableMouseMoveMap();
```
## Map Items

map items are used for drawing on the map.
All map items are derived from the ``MapItem`` class.

### Ellipse

```c++
MapEllipse* ellipse = new MapEllipse(mapView);

//ellipse->setPosition(QPointF(100, 100));
ellipse->setPosition(mapView->center());
ellipse->setAlignmentFlags(Qt::AlignCenter);
ellipse->setSize(QSizeF(200, 150));
//ellipse->setSize(QGeoCoordinate(1e-3, 2e-3));

ellipse->setBackgroundColor(QColor::fromRgba(0xAF0000FF));
ellipse->setPen(QPen(Qt::black, 3));
```

### Rect

```c++
MapRect* rect = new MapRect(mapView);

rect->setPosition(mapView->center());
rect->setSize(QSizeF(200, 150));

rect->setBorderRadius(8);
rect->setBorderRadius(8, 20, 0, 40);
```

### Text

```c++
MapText* text = new MapText(mapView);
text->setPosition(mapView->center());

// if size is not set
// text size will be used.
text->setText("Lorem ipsum dolor sit amet.");

text->setFont(QFont("Arial", 14));
text->setTextColor(Qt::white);
text->setTextFlags(Qt::TextSingleLine);
text->setTextPadding(10, 10, 10, 10);
```


### Image

```c++
MapImage* img = new MapImage(mapView);

img->setPosition(mapView->center());
img->setAlignmentFlags(Qt::AlignCenter);
img->setBorderRadius(8);

img->setAspectRatioMode(Qt::IgnoreAspectRatio);
img->setImage(QImage("image.png").scaledToWidth(200, Qt::SmoothTransformation));
//img->setImage("image.png");
```

### Lines

```c++
MapLines* lines = new MapLines(mapView);
lines->setPen(QPen(Qt::blue, 5));

lines->points().push_back(QPointF(0, 0));
lines->points().push_back(mapView->center());
```

### Polygon

```c++
MapPolygon* polygon = new MapPolygon(mapView);
polygon->setPen(QPen(Qt::blue, 5));
polygon->setBackgroundColor(QColor(255, 0, 0, 50));

polygon->points() = {
	QPointF(-100, 0),
	mapView->center(),
	QPointF(-100, mapView->height() * 2)
};
```

## Markers

### Add Marker

```c++
MapImage* markerIcon = mapView->addMarker(mapView->center());
markerIcon->findChild<MapText*>()->setText("Marker Text");
```

### Change Default Marker Icon

```c++
mapView->setMarkerIcon(":/map_marker_alt.svg");

QImage newIcon(":/map_marker_alt.svg");
// newIcon.doStuff();
mapView->setMarkerIcon(newIcon);
```

## QML

``SimpleMapView`` also provides a QML component based on ``QQuickItem`` instead of ``QWidget``. Since ``QQuickItem`` uses GPU-accelerated rendering, it offers better performance.

To use the QML component, you need to enable it in your build system first.

CMake:
```cmake
set(SIMPLE_MAP_VIEW_USE_QML ON)
```

qmake:
```
SIMPLE_MAP_VIEW_USE_QML = 1
```

Then in your ``main.cpp`` file, you need to register the QML components after creating the ``app`` instance.

Sample main code:
```c++
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "SimpleMapView.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    SimpleMapView::registerQmlTypes();

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/qt/qml/yourapp/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
```

After that you can import and use the QML components in your ``.qml`` files.

```qml
import QtQuick
import QtQuick.Window
import QtPositioning
import com.github.ozguronsoy.SimpleMapView

Window {
    visible: true
    width: 960
    height: 540
    title: "QtQuickApplication1"

    SimpleMapView {
        id: map
        anchors.fill: parent
        tileServer: TileServers.GOOGLE_MAP
        latitude: 37.78310363232004 // Denizli, Türkiye
        longitude: 29.095721285868844
        zoomLevel: 14

        MapEllipse {
            backgroundColor: "red"
            penColor: "blue"
            penWidth: 4
            position: SimpleMapViewQmlHelpers.createMapPoint(Qt.point(50, 50))
            size: SimpleMapViewQmlHelpers.createMapSize(Qt.size(150, 100))
        }

        MapRect {
            backgroundColor: "blue"
            penColor: "red"
            penWidth: 4
            position: SimpleMapViewQmlHelpers.createMapPoint(QtPositioning.coordinate(37.77610363232004, 29.065721285868844))
            size: SimpleMapViewQmlHelpers.createMapSize(QtPositioning.coordinate(0.006, 0.01))
        }

        Component.onCompleted: {
            map.addMarker(map.center)
        }
    }
}

```

## Using Offline Maps

Create a widgets app and download the tiles. This is a one time thing.
```c++
SimpleMapView* mapView = new SimpleMapView(this);

// Eskişehir, Türkiye
const QGeoCoordinate topLeft(39.86073417201014, 30.292027040985936);
const QGeoCoordinate bottomRight(39.68314525072665, 30.719807145290957);
const int zoomLevel1 = 10;
const int zoomLevel2 = 15;

mapView->downloadTiles("path/to/offline-tiles", topLeft, bottomRight, zoomLevel1, zoomLevel2);
```

Use downloaded tiles
```c++
SimpleMapView* mapView = new SimpleMapView(this);
mapView->setTileServer("path/to/offline-tiles");
// mapView->setTileServer(":/SimpleMapView/Tiles");
```

``downloadTiles`` method also generates a ``qrc`` file so one can use the resource system. However, this is not recommended for large maps, as it increases compile time and can significantly bloat the executable.