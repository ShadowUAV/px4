#-------------------------------------------------
#
# Project created by QtCreator 2016-04-09T10:39:32
#
#-------------------------------------------------

QT       += core gui xml widgets

TARGET = GeofenceSelect
TEMPLATE = app
INCLUDEPATH += ../Geofence/

SOURCES += main.cpp \
		../Geofence/Geofence.cpp \
	    ../Geofence/Element.cpp \
        SelectDialog.cpp

HEADERS  += SelectDialog.h\
		../Geofence/Geofence.h\
		../Geofence/Element.h

FORMS    += Dialog.ui
