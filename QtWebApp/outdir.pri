QTWEBAPP_LIBNAME = QtWebApp
QTWEBAPP_BIN = $$PWD/bin

win32 {
    QTWEBAPP_BIN = $$QTWEBAPP_BIN-win32
    msvc:QTWEBAPP_BIN = $$QTWEBAPP_BIN-msvc$$QMAKE_MSC_VER
}
else:macx:QTWEBAPP_BIN = $$QTWEBAPP_BIN-osx
else:linux:QTWEBAPP_BIN = $$QTWEBAPP_BIN-linux


ARCH = $$QT_ARCH
macx {
    isEqual(QMAKE_APPLE_DEVICE_ARCHS, "x86_64") {
        ARCH = $$QMAKE_APPLE_DEVICE_ARCHS
    } else:isEqual(QMAKE_APPLE_DEVICE_ARCHS, "arm64") {
        ARCH = $$QMAKE_APPLE_DEVICE_ARCHS
    } else:contains(QMAKE_APPLE_DEVICE_ARCHS, "x86_64"):contains(QMAKE_APPLE_DEVICE_ARCHS, "arm64") {
        ARCH = "universal"
    }
}

isEmpty(QTWEBAPP_BIN) {
    message("Only mac/win32/linux supported")
    QTWEBAPP_BIN = "FAIL"
} else {
    clang:QTWEBAPP_BIN = $$QTWEBAPP_BIN-clang
    else:gcc:QTWEBAPP_BIN = $$QTWEBAPP_BIN-gcc

    QTWEBAPP_BIN = $$QTWEBAPP_BIN-$$ARCH
}

!isEmpty(QTWEBAPP_STATIC) {
    QTWEBAPP_BIN = $$QTWEBAPP_BIN-static
    DEFINES += QTWEBAPPLIB_STATIC
}

CONFIG(debug, debug|release) {
    QTWEBAPP_BIN = $$QTWEBAPP_BIN-debug
}
