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
- [Markers](#markers)
    - [Add Marker](#add-marker)
    - [Change Default Marker Icon](#change-default-marker-icon)

## Setup

after including the ``.h`` and ``.cpp`` files, add the ``Positioning`` and ``Network`` modules to the build system.
Then add the marker icons to one of your ``.qrc`` files.

cmake:
```cmake
find_package(Qt6 REQUIRED COMPONENTS Network Positioning)
target_link_libraries(mytarget PRIVATE Qt6::Network Qt6::Positioning)

set(PROJECT_SOURCES
    # your files

    SimpleMapView.h
    SimpleMapView.cpp
    MapItem.h
    MapItem.cpp
    MapEllipse.h
    MapEllipse.cpp
    MapRect.h
    MapRect.cpp
    MapText.h
    MapText.cpp
    MapImage.h
    MapImage.cpp
)

```

qmake
```
QT += network
QT += positioning
```

Resources.qrc
```xml
<RCC>
  <qresource>
    <file alias="map_marker.svg">path/to/map_marker.svg</file>
    <file alias="map_marker_alt.svg">path/to/map_marker_alt.svg</file>
  </qresource>
</RCC>
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
mapView->setTileServer(SimpleMapView::TileServers::GOOGLE_MAP);
mapView->setTileServer(SimpleMapView::TileServers::GOOGLE_SAT);
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

``MapRect`` derives from ``MapEllipse`` and supports all its features.

```c++
MapRect* rect = new MapRect(mapView);

rect->setPosition(mapView->center());
rect->setSize(QSizeF(200, 150));

rect->setBorderRadius(8);
rect->setBorderRadius(8, 20, 0, 40);
```

### Text

``MapText`` derives from ``MapRect`` and supports all its features.

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

``MapImage`` derives from ``MapRect`` and supports all its features.

```c++
MapImage* img = new MapImage(mapView);

img->setPosition(mapView->center());
img->setAlignmentFlags(Qt::AlignCenter);
img->setBorderRadius(8);

img->setAspectRatioMode(Qt::IgnoreAspectRatio);
img->setImage(QImage("image.png").scaledToWidth(200, Qt::SmoothTransformation));
//img->setImage("image.png");
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
