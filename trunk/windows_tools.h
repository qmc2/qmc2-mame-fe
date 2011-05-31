#if defined(Q_WS_WIN)

#ifndef _WINDOWS_TOOL_H_
#define _WINDOWS_TOOL_H_

#include <Qt>
#include <windows.h>

#define QMC2_WIN_MAX_PROCS	4096

HANDLE winFindProcessHandle(QString);
HANDLE winGetProcessHandle(Q_PID pid);

#endif

#endif
