#include "qtwebappglobal.h"

#ifndef QTWEBAPP_MAJOR
#error Please define QTWEBAPP_MAJOR
#endif
#ifndef QTWEBAPP_MINOR
#error Please define QTWEBAPP_MAJOR
#endif
#ifndef QTWEBAPP_PATCH
#error Please define QTWEBAPP_PATCH
#endif
#ifndef QTWEBAPP_VERSION_STR
#error Please define QTWEBAPP_VERSION_STR
#endif

#define QTWEBAPP_VERSION \
	((QTWEBAPP_MAJOR << 16) | (QTWEBAPP_MINOR << 8) | QTWEBAPP_PATCH)

const char *qtwebapp::getQtWebAppLibVersion()
{
	return QTWEBAPP_VERSION_STR;
}
int qtwebapp::getQtWebAppLibVersionMajor()
{
	return QTWEBAPP_MAJOR;
}
int qtwebapp::getQtWebAppLibVersionMinor()
{
	return QTWEBAPP_MINOR;
}
int qtwebapp::getQtWebAppLibVersionPatch()
{
	return QTWEBAPP_PATCH;
}
int qtwebapp::getQtWebAppLibVersionNumber()
{
	return QTWEBAPP_VERSION;
}

int qtwebapp::parseNum(const QVariant &v, int base)
{
	QString str = v.toString();
	int mul = 1;
	if (str.endsWith('K'))
		mul *= base;
	else if (str.endsWith('M'))
		mul *= base * base;
	else if (str.endsWith('G'))
		mul *= base * base * base;
	if (mul != 1)
		str = str.mid(str.length() - 1);
	return str.toInt() * mul;
}
