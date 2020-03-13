INCLUDEPATH += $$PWD/..
INCLUDEPATH += $$PWD/qtservice

include(outdir.pri)

LIBS += -L$$QTWEBAPP_BIN
LIBS += -l$$QTWEBAPP_LIBNAME