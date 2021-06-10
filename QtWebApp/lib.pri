include(outdir.pri)

TARGET = $$QTWEBAPP_LIBNAME
DESTDIR = $$QTWEBAPP_BIN

TEMPLATE = lib
QT -= gui
CONFIG += c++11

VER_MAJ = 1
VER_MIN = 8
VER_PAT = 3
VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}ac1
 
DEFINES += \
	QTWEBAPP_MAJOR=$$VER_MAJ \
	QTWEBAPP_MINOR=$$VER_MIN \
	QTWEBAPP_PATCH=$$VER_PAT \
	"QTWEBAPP_VERSION_STR=\"\\\"$$VERSION\\\"\""

DISTFILES += doc/* mainpage.dox Doxyfile
OTHER_FILES += ../readme.txt

INCLUDEPATH += $PWD

HEADERS += qtwebappglobal.h
SOURCES += qtwebappglobal.cpp

include(qtservice/qtservice.pri)
include(logging/logging.pri)
include(httpserver/httpserver.pri)
include(templateengine/templateengine.pri)
