#if defined(Q_WS_WIN)

#ifndef _WINDOWS_TOOL_H_
#define _WINDOWS_TOOL_H_

#include <Qt>
#include <windows.h>

#define QMC2_WIN_MAX_PROCS	4096
#define QMC2_WIN_MAX_NAMELEN	1024

HANDLE winFindProcessHandle(QString);
HWND winFindWindowHandle(QString);

#endif

#endif
