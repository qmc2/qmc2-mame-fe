#include "windows_tools.h"

#if defined(QMC2_OS_WIN)

#include <psapi.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>

// To ensure correct resolution of symbols, add psapi.lib to LIBS
// and compile with -DPSAPI_VERSION=1
// (see http://msdn.microsoft.com/en-us/library/ms682623.aspx)

QString winSearchText;
QMap<HWND, QString> winWindowMap;
HWND winFoundHandle;

HANDLE winFindProcessHandle(QString procName)
{
	HANDLE processHandle = 0;
	DWORD procs[QMC2_WIN_MAX_PROCS], bytesNeeded;
	if ( EnumProcesses(procs, sizeof(procs), &bytesNeeded) ) {
		DWORD numProcesses = bytesNeeded / sizeof(DWORD);
		for (uint i = 0; i < numProcesses; i++) {
			HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, procs[i]);
			if ( hProcess != 0 ) {
				HMODULE hMod;
				DWORD bN;
				if ( EnumProcessModules(hProcess, &hMod, sizeof(hMod), &bN) ) {
					TCHAR processName[MAX_PATH];
					GetModuleBaseName(hProcess, hMod, processName, sizeof(processName)/sizeof(TCHAR));
#ifdef UNICODE
					QString pN = QString::fromUtf16((ushort*)processName);
#else
					QString pN = QString::fromLocal8Bit(processName);
#endif
					if ( pN == procName ) {
						processHandle = hProcess;
						CloseHandle(hProcess);
						break;
					}
				}
				CloseHandle(hProcess);
			}
		}
	}

	return processHandle;
}

BOOL CALLBACK winFindWindowHandleCallbackProc(HWND hwnd, LPARAM lParam)
{
	WCHAR winTitle[QMC2_WIN_MAX_NAMELEN];
	if ( !GetWindow(hwnd, GW_OWNER) ) {
		GetWindowText(hwnd, winTitle, QMC2_WIN_MAX_NAMELEN - 1);
		QString windowTitle = QString::fromWCharArray(winTitle);
		if ( windowTitle == winSearchText )
			winFoundHandle = hwnd;
		winWindowMap[hwnd] = windowTitle;
	}
	return true;
}

HWND winFindWindowHandle(QString windowTitle)
{
	winFoundHandle = 0;
	winSearchText = windowTitle;
	winWindowMap.clear();
	EnumWindows((WNDENUMPROC)winFindWindowHandleCallbackProc, 0);
	return winFoundHandle;
}

void winRefreshWindowMap()
{
	winFindWindowHandle("DUMMY");
}

HWND winFindWindowHandleOfProcess(Q_PID processInfo, QString subString)
{
	bool handleFound = false;
	HWND windowHandle = GetTopWindow(0);
	DWORD pid;
	while ( windowHandle && !handleFound ) {
		GetWindowThreadProcessId(windowHandle, &pid);
		if ( pid == processInfo->dwProcessId ) {
			if ( !subString.isEmpty() ) {
				WCHAR winTitle[QMC2_WIN_MAX_NAMELEN];
				GetWindowText(windowHandle, winTitle, QMC2_WIN_MAX_NAMELEN - 1);
				QString windowTitle = QString::fromWCharArray(winTitle);
				if ( windowTitle.contains(subString) )
					handleFound = true;
			} else
				handleFound = true;
		}
		if ( !handleFound )
         		windowHandle = GetNextWindow(windowHandle, GW_HWNDNEXT);
	}
	return handleFound ? windowHandle : 0;
}

void winAllocConsole(bool parentOnly)
{
        bool parentConsoleAttached = true;
	if ( !AttachConsole(ATTACH_PARENT_PROCESS) ) {
		if ( !parentOnly ) {
			AllocConsole();
			parentConsoleAttached = false;
		} else
			return;
	}

	HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
	int hCrt = _open_osfhandle((intptr_t) handle_out, _O_TEXT);
	FILE *hf_out = _fdopen(hCrt, "w");
	setvbuf(hf_out, 0, _IONBF, 1);
	*stdout = *hf_out;

	HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
	hCrt = _open_osfhandle((intptr_t) handle_in, _O_TEXT);
	FILE *hf_in = _fdopen(hCrt, "r");
	setvbuf(hf_in, 0, _IONBF, 128);
	*stdin = *hf_in;

	if ( parentConsoleAttached ) {
		printf("\n");
		fflush(stdout);
	}
}

void winFreeConsole()
{
	FreeConsole();
}

void winGetMemoryInfo(quint64 *totalSize, quint64 *totalUsed, quint64 *totalFree)
{
	MEMORYSTATUSEX memStatus;
	memStatus.dwLength = sizeof(memStatus);
	if ( GlobalMemoryStatusEx(&memStatus) ) {
		*totalSize = memStatus.ullTotalPhys / QMC2_1M;
		*totalFree = memStatus.ullAvailPhys / QMC2_1M;
		*totalUsed = *totalSize - *totalFree;
	}
}

#endif
