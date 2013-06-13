/***************************************************************************
    NWN Extender - NWN Server related functions
    Copyright (C) 2003 Ingmar Stieger (Papillon)
    email: papillon@blackdagger.com

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ***************************************************************************/

#include "stdafx.h"
#include "shellapi.h"
#include <conio.h>
#include <windows.h>
#include "nwnserver.h"
#include <tchar.h>

STARTUPINFOW si;
PROCESS_INFORMATION pi;
char logDir[8] = {0};

// returns log directory the server will use (logs.0, logs.1, ...)
char* GetLogDir()
{
	if (strlen(logDir) == 0)
	{
		int iDirectory = 0;
		char tmpNo[3] = {0};		
		char tmpFileName[26] = {0};
		HANDLE outFile;
		
		while (iDirectory < 100)
		{
			strcpy(tmpFileName, "logs.");
			_itoa(iDirectory, tmpNo, 10);
			strcat(tmpFileName, tmpNo);
			strcat(tmpFileName, "\\nwserverlog1.txt");

			outFile = CreateFileA(
			tmpFileName,						// pointer to name of the file
			GENERIC_READ | GENERIC_WRITE,       // access (read-write) mode
			0,									// exclusive mode
			NULL,                               // pointer to security attributes
			OPEN_ALWAYS,                        // how to create
			0,                                  // file attributes
			NULL);                              // handle to file with attributes to copy

			if (outFile != INVALID_HANDLE_VALUE)
			{
				// file not in use
				strcpy(logDir, "logs.");
				strcat(logDir, tmpNo);
				CloseHandle(outFile);
				break;
			}
			CloseHandle(outFile);
			iDirectory++;
		}

	}
	return logDir;
}

void RotateLogs()
{
	char tmpNo[3] = {0};
	char oldDirNo;
	char baseDirName[256] = {0};
	char oldDirName[256] = {0};
	char newDirName[256] = {0};
	char tmpFileName[256] = {0};

	if (strlen(GetLogDir()) == 0)
		return;

	GetCurrentDirectoryA(256, baseDirName);
	strcat(baseDirName, "\\");
	strcat(baseDirName, GetLogDir());
	strcat(baseDirName, "\\");

	// Delete directory '9'
	strcpy(oldDirName, baseDirName);
	strcat(oldDirName, "9");

	SHFILEOPSTRUCTA fileOp;
	fileOp.hwnd = 0;
	fileOp.wFunc = FO_DELETE;
	fileOp.pFrom = oldDirName;
	fileOp.pTo = NULL;
	fileOp.fFlags = FOF_NOERRORUI + FOF_NOCONFIRMATION;
    fileOp.fAnyOperationsAborted = NULL;
    fileOp.hNameMappings = NULL;
    fileOp.lpszProgressTitle = NULL;

	SHFileOperationA(&fileOp);

	// Age directories 1-8
	for (oldDirNo = 99; oldDirNo > 0; oldDirNo--)
	{
		strcpy(oldDirName, baseDirName);
		_itoa(oldDirNo, tmpNo, 10);
		strcat(oldDirName, tmpNo);
		strcpy(newDirName, baseDirName);
		_itoa(oldDirNo + 1, tmpNo, 10);
		strcat(newDirName, tmpNo);
		MoveFileA(oldDirName, newDirName);
	}

	// Create youngest directory '1'
	CreateDirectoryA(oldDirName, NULL);

	// Move current log files to '1'
	strcpy(oldDirName, baseDirName);
	strcat(oldDirName, "nwnx.txt");
	strcpy(newDirName, baseDirName);
	strcat(newDirName, "1\\nwnx.txt");
	MoveFileA(oldDirName, newDirName);
	strcpy(oldDirName, baseDirName);
	strcat(oldDirName, "nwserverlog1.txt");
	strcpy(newDirName, baseDirName);
	strcat(newDirName, "1\\nwserverlog1.txt");
	MoveFileA(oldDirName, newDirName);
	strcpy(oldDirName, baseDirName);
	strcat(oldDirName, "nwserverError1.txt");
	strcpy(newDirName, baseDirName);
	strcat(newDirName, "1\\nwserverError1.txt");
	MoveFileA(oldDirName, newDirName);

	strcpy(oldDirName, baseDirName);
	strcat(oldDirName, "nwnx_odbc.txt");
	strcpy(newDirName, baseDirName);
	strcat(newDirName, "1\\nwnx_odbc.txt");
	MoveFileA(oldDirName, newDirName);
}

void StartServerProcess(char  *cl, STARTUPINFOW* si, PROCESS_INFORMATION* pi)
{
	void *address;
	WCHAR dllPath[MAX_PATH];
	WCHAR WDFixed[MAX_PATH];
	WCHAR wappendage[20];
	WCHAR server_cast[14];
	WCHAR cl_cast[300];
	WCHAR kernel_cast[14];
	char *appendage = "\\nwnx-module.dll";
	char *server_name = "nwserver.exe";
	char *kernel_name = "Kernel32.dll";
	unsigned long numBytes, exitCode;
	HANDLE thandle;

	char debuggy[256];

	GetCurrentDirectoryW(sizeof(WCHAR) * MAX_PATH, dllPath);
	GetCurrentDirectoryW(sizeof(WCHAR) * MAX_PATH, WDFixed);
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, appendage, -1, wappendage, strlen(appendage) + 1);
	wcscat(dllPath, wappendage );

	
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, server_name, -1, server_cast, strlen(server_name) + 1);
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, cl, -1, cl_cast, strlen(cl) + 1);
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, kernel_name, -1, kernel_cast, strlen(kernel_name) + 1);

	CreateProcessW(server_cast, cl_cast , NULL, NULL, FALSE, 
		            CREATE_SUSPENDED | CREATE_PRESERVE_CODE_AUTHZ_LEVEL, NULL, WDFixed, si, pi);

	// Get permission to write into the process's address space of a space about the length of the path/DLL name.
	address = VirtualAllocEx(pi->hProcess, NULL, sizeof(WCHAR)*wcsnlen(dllPath, MAX_PATH) + sizeof(WCHAR), 
						MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if(!address) {
		MessageBoxA(NULL, "Can't start server process; there was a memory access issue.  Does NWNX2 have the permissions it needs to run?", "Error", MB_TASKMODAL | MB_TOPMOST | MB_ICONERROR | MB_OK);
		return;
	}

	// Write the DLL name into the process's address space
	if(!WriteProcessMemory(pi->hProcess, address, (const void *) dllPath,  sizeof(WCHAR)*wcsnlen(dllPath, MAX_PATH) + sizeof(WCHAR), &numBytes)) {
		MessageBoxA(NULL, "Can't start server process; The write to memory failed.", "Error", MB_TASKMODAL | MB_TOPMOST | MB_ICONERROR | MB_OK);
		return;
	}

	// Force the process to load the DLL by creating a thread inside the process that calls LoadLibrary and exits.
	thandle = CreateRemoteThread(pi->hProcess, NULL, 0, (LPTHREAD_START_ROUTINE) GetProcAddress(LoadLibraryW(kernel_cast), "LoadLibraryW"), address, 0, NULL);

	// Let's check if the thread ran ok-
	if(!GetExitCodeThread(thandle, &exitCode) || thandle == NULL) {
		MessageBoxA(NULL, "Can't start server process; The loading thread in nwserver failed to run.", "Error", MB_TASKMODAL | MB_TOPMOST | MB_ICONERROR | MB_OK);
		return;		
	}

	// 259 is still running
	while(exitCode == 259) {
		GetExitCodeThread(thandle, &exitCode);
	}

	// Let's check how LoadLibrary() exited
	if(!exitCode) {
		exitCode = GetLastError();
	// don't do this normally...
		sprintf(debuggy, "%d", exitCode);
		MessageBoxA(NULL, debuggy, "Error", MB_TASKMODAL | MB_TOPMOST | MB_ICONERROR | MB_OK);
		//MessageBoxA(NULL, "Can't start server process; nwnx-module.dll failed to load into nwserver.exe", "Error", MB_TASKMODAL | MB_TOPMOST | MB_ICONERROR | MB_OK);
		return;		
	}



	// All good to go, let's kick the football-
	if(!ResumeThread(pi->hThread)) {
		MessageBoxA(NULL, "There was a threading problem, possibly a bad handle.", "Error", MB_TASKMODAL | MB_TOPMOST | MB_ICONERROR | MB_OK);
	}

}

void StartAndInject(char* cl)
{
	GetStartupInfoW(&si);
	RotateLogs();

	StartServerProcess(cl, &si, &pi);
}

BOOL CheckProcessActive()
{
	DWORD lpExitCode;
	if (GetExitCodeProcess(pi.hProcess, &lpExitCode))
	{
		if (lpExitCode == STILL_ACTIVE)
			return TRUE;
	}	
	return FALSE;
}

void KillServerProcess()
{
	TerminateProcess(pi.hProcess, -1);
}


DWORD WINAPI StartServerDummyThread(LPVOID lpParam) 
{
	// Start NWN server
	StartAndInject((char*)lpParam);
	return 0;
} 

