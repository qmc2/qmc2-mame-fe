#ifndef _WINDOWS_TOOL_H_
#define _WINDOWS_TOOL_H_

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
void winAllocConsole();
void winFreeConsole();

#endif

#endif
