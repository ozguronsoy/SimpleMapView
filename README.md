# SimpleMapView

A Qt widget for rendering tile maps.

- [Setup](#setup)
- [Create Widget](#create-widget)
- [Change Tile Server](#change-tile-server)
- [Limit Zoom](#limit-zoom)
- [Lock Zoom and Geolocation]()
- [Disable Mouse Events](#disable-mouse-events)

### Setup

after including the ``.h`` and ``.cpp`` files, add the ``Positioning`` and ``Network`` modules to the build system.

cmake:
```
find_package(Qt6 REQUIRED COMPONENTS Network)
target_link_libraries(mytarget PRIVATE Qt6::Network)

find_package(Qt6 REQUIRED COMPONENTS Positioning)
target_link_libraries(mytarget PRIVATE Qt6::Positioning)
```

qmake
```
QT += network
QT += positioning
```

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

You can use any tile server that contains ``{x}``, ``{y}``, and ``{z}`` coordinates in the URL.

```c++
// google maps
mapView->setTileServer("https://mt0.google.com/vt/lyrs=m&hl=en&x={x}&y={y}&z={z}&s=Ga");

// google satellite
mapView->setTileServer("https://mt0.google.com/vt/lyrs=s&hl=en&x={x}&y={y}&z={z}&s=Ga");
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