#pragma once

#include <sys/types.h>

#include <QtGlobal>
#include <QVariant>

#if defined(QTWEBAPPLIB_STATIC)
#  define QTWEBAPP_EXPORT
#  define QTWEBAPP_PRIVATE
#else
#if defined _WIN32 || defined __CYGWIN__
#  if defined(CMAKE_QTWEBAPP_SO) || defined(QTWEBAPPLIB_EXPORT)
#    ifdef __GNUC__
#      define QTWEBAPP_EXPORT __attribute__((dllexport))
#    else // __GNUC__
#      define QTWEBAPP_EXPORT __declspec(dllexport)
#    endif // __GNUC__
#  else // CMAKE_QTWEBAPP_SO
#    ifdef __GNUC__
#      define QTWEBAPP_EXPORT __attribute__((dllimport))
#    else // __GNUC__
#      define QTWEBAPP_EXPORT __declspec(dllimport)
#    endif // __GNUC__
#  endif // CMAKE_QSL_SO
#  define QTWEBAPP_PRIVATE
#elif defined __GNUC__ && __GNUC__ >= 4
#  define QTWEBAPP_EXPORT __attribute__((visibility("default")))
#  define QTWEBAPP_PRIVATE __attribute__((visibility("hidden")))
#else
#  define QTWEBAPP_EXPORT
#  define QTWEBAPP_PRIVATE
#endif
#endif

namespace qtwebapp {

/// The version of QtWebApp.
QTWEBAPP_EXPORT const char* getQtWebAppLibVersion();
QTWEBAPP_EXPORT int getQtWebAppLibVersionMajor();
QTWEBAPP_EXPORT int getQtWebAppLibVersionMinor();
QTWEBAPP_EXPORT int getQtWebAppLibVersionPatch();
QTWEBAPP_EXPORT int getQtWebAppLibVersionNumber();

/// Parses the given number by respecting its suffix.
QTWEBAPP_EXPORT int parseNum(const QVariant &v, int base = 1e3);

}
