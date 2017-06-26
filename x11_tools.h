#ifndef X11_TOOLS_H
#define X11_TOOLS_H

#include "macros.h"

#if defined(QMC2_OS_UNIX)

#include <QtGui>

#define QMC2_X11_MAX_PROCS	4096
#define QMC2_X11_MAX_NAMELEN	1024

WId x11FindWindowId(QString titlePattern, QString classHintPattern);

#endif

#endif
