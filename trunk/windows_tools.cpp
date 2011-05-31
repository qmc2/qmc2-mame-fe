#if defined(Q_WS_WIN)

#include "windows_tools.h"
#include <psapi.h>

// To ensure correct resolution of symbols, add psapi.lib to LIBS
// and compile with -DPSAPI_VERSION=1
// (see http://msdn.microsoft.com/en-us/library/ms682623.aspx)

HANDLE winFindProcessHandle(QString procName)
{
	HANDLE processHandle = NULL;
	DWORD procs[QMC2_WIN_MAX_PROCS], bytesNeeded;

	if ( EnumProcesses(procs, sizeof(procs), &bytesNeeded) ) {
		DWORD numProcesses = bytesNeeded / sizeof(DWORD);
		for (int i = 0; i < numProcesses; i++) {
			HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, procs[i]);
			if ( hProcess != NULL ) {
				HMODULE hMod;
				DWORD bN;
				if ( EnumProcessModules(hProcess, &hMod, sizeof(hMod), &bN) ) {
					TCHAR processName[MAX_PATH];
					GetModuleBaseName(hProcess, hMod, processName, sizeof(processName)/sizeof(TCHAR));
					QString pN((const char *)processName);
					if ( pN == procName ) {
						// debug
						printf("processHandle found!\n"); fflush(stdout);
						processHandle = hProcess;
						break;
					}
				}
				CloseHandle(hProcess);
			}
		}
	}

	return processHandle;
}

HANDLE winGetProcessHandle(Q_PID pid)
{
	HANDLE processHandle = NULL;

	// FIXME

	return processHandle;
}

#endif
