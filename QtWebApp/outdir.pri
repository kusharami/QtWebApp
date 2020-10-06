QTWEBAPP_LIBNAME = QtWebApp
QTWEBAPP_BIN = $$PWD/bin

win32 {
    QTWEBAPP_BIN = $$QTWEBAPP_BIN-win32
    msvc:QTWEBAPP_BIN = $$QTWEBAPP_BIN-msvc$$QMAKE_MSC_VER
}
else:macx:QTWEBAPP_BIN = $$QTWEBAPP_BIN-osx
else:linux:QTWEBAPP_BIN = $$QTWEBAPP_BIN-linux

isEmpty(QTWEBAPP_BIN) {
    message("Only mac/win32/linux supported")
    QTWEBAPP_BIN = "FAIL"
} else {
    clang:QTWEBAPP_BIN = $$QTWEBAPP_BIN-clang
    else:gcc:QTWEBAPP_BIN = $$QTWEBAPP_BIN-gcc

    QTWEBAPP_BIN = $$QTWEBAPP_BIN-$$QT_ARCH
}

!isEmpty(QTWEBAPP_STATIC) {
    QTWEBAPP_BIN = $$QTWEBAPP_BIN-static
    DEFINES += QTWEBAPPLIB_STATIC
}

CONFIG(debug, debug|release) {
    QTWEBAPP_BIN = $$QTWEBAPP_BIN-debug
}
