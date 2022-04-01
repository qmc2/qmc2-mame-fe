#ifndef WINDOWS_TOOLS_H
#define WINDOWS_TOOLS_H

#include "macros.h"

#if defined(QMC2_OS_WIN)

#include <QtGui>
#include <windows.h>

#define QMC2_WIN_MAX_PROCS	4096
#define QMC2_WIN_MAX_NAMELEN	1024

HANDLE winFindProcessHandle(QString);
HWND winFindWindowHandle(QString);
HWND winFindWindowHandleOfProcess(Q_PID, QString subString = QString());
void winRefreshWindowMap();
void winAllocConsole(bool parentOnly = true);
void winFreeConsole();
void winGetMemoryInfo(quint64 *, quint64 *, quint64 *);

#endif

#endif
