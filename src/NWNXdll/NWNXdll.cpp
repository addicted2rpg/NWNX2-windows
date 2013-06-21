/***************************************************************************
    NWN Extender - Library for injection
    Copyright (C) 2003 Ingmar Stieger (Papillon) and Jeroen Broekhuizen and David Strait (2013)
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
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <windows.h>
#include <Winsock2.h>
#include "string.h"

#include "NWNXBase.h"
#include "IniFile.h"
#include "HashTable.h"
#include "..\NWNX2\NWNXShared.h"
#include "hook_funcs.h"


#pragma comment(lib, "Ws2_32.lib")


// perms: ??
// buildfile: chasing down authors for plugins ;-)
// madChook: check; lets pray ODBC holds up good :)
// gspy list: check; preliminary game use looks good.

#define USE_HOST 0


typedef CNWNXBase* (WINAPI* GETCLASSOBJECT)();
GETCLASSOBJECT GetClassObject = NULL;

FILE *logFile;
char logDir[8] = {0};
char logFileName[18] = {0};
int debuglevel = 0;

bool ObjRet = 0;
unsigned long oRes;

CHashTable Libraries;

void PayLoad(char*, char**, char**);
void ObjectPayLoad(char*, char*);
void LoadLibraries ();
BOOL APIENTRY DllMain(HANDLE, DWORD, LPVOID);
char* GetLogDir();



/******** Core NWNX2 stuff **************/
//void (*SetLocalStringNextHook)();
//void (*GetLocalObjectNextHook)();
void *SetLocalStringNextHook = NULL;
void *GetLocalObjectNextHook = NULL;

void SetLocalStringHookProc();
void GetLocalObjectHookProc(const char **var_name);


/*********** WINAPI function overrides & vars for server listing *********************/

int WINAPI sendtoProc(SOCKET s, const char *buf, int len, int flags, const sockaddr *to, int tolen);
void *sendtoOriginal = NULL;

struct hostent * WINAPI gethostbynameProc(const char *name);
void *gethostbynameOriginal = gethostbyname;

char new_masterserver[256];
struct hostent *masterserver = NULL;
struct hostent *biowareserver = NULL;
unsigned long address_master;
unsigned long address_bioware;

/***************************************************/



// Things work a little differently now without MadCHook...  
void __declspec(naked) SetLocalStringHookProc()
{
	__asm {

		// Since PayLoad is called from naked, we need to do this.  The compiler won't put in 
		// __cdecl and __stdcall preparations
		push eax
		push ebx
		push ecx
		push edi
		push esi


		mov eax, [esp+0x8+0x14]
		push eax
		mov eax, [esp+0x8+0x14]
		push eax
		mov eax, [esp+0x24+0x14]
		push eax
		call PayLoad
		add esp, 0xC

		pop esi
		pop edi
		pop ecx
		pop ebx
		pop eax

		// This does not call the original, it goes to a custom bridge that eventually leads back 
		// to the original.  Matters of alignment and stack parameterization are addressed there.
		call SetLocalStringNextHook

	}
}


 /*

 // Ingmar Stieger's original that uses the MadCHook library:
void __declspec(naked) SetLocalStringHookProc()
{
	__asm {	
		
		push ecx	  // save register contents
		push edx
		push ebx
		push esi
		push edi
		
		push ebp	  // prolog 1
        mov ebp, esp  // prolog 2
		

		
		mov eax, dword ptr ss:[esp+0x20] // variable value (param 3)
		//mov eax, [eax] 
		push eax
		mov eax, dword ptr ss:[esp+0x20] // variable name (param 2)
		mov eax, [eax] 
		push eax
		mov eax, dword ptr ss:[esp+0x3C] // game object (param 1)
		push eax
		call PayLoad
		add esp, 0xC

		
		pop ebp		// restore register contents
		pop edi		
		pop esi
		pop ebx
		pop edx
		pop ecx





		mov eax, dword ptr ss:[esp+0x4]  
		push eax
		mov eax, dword ptr ss:[esp+0x4]  
		push eax

		call SetLocalStringNextHook // call original function
		
		
		// cleanup stack
		pop eax   // grabs return value from SetLocalStringNextHook
		// add esp, 8  // ends this function
		add esp, 0xC
		push eax  // puts it back on the stack


        retn
	}
}
*/

void __declspec(naked) GetLocalObjectHookProc(const char **var_name)
{

	__asm {

		push eax
		push ebx
		push ecx
		push edi
		push esi

		mov eax, dword ptr ss:[esp+0x18] // variable name (param 2)
		mov eax, [eax] 
		push eax
		mov eax, dword ptr ss:[esp+0x2C] // game object (param 1)
		push eax
		call ObjectPayLoad
		add esp, 0x8

		pop esi
		pop edi
		pop ecx
		pop ebx


		xor eax, eax
		mov al, byte ptr ObjRet //check if we need to bypass the original function and return our value
		test eax, eax
		mov ObjRet, 0
		mov eax, oRes  //return value


		jnz ext  //don't call the original function

//		mov eax, dword ptr ss:[esp+0x4] // arg 1
//		push eax
		pop eax
		call GetLocalObjectNextHook

ext:
		add esp, 0x4
        retn 4
	}


}


/*
void __declspec(naked) GetLocalObjectHookProc(const char **var_name)
{
	//Too much assembly here..
	__asm {

		push ecx	  // save register contents
		push edx
		push ebx
		push esi
		push edi


		push ebp	  // prolog 1
        mov ebp, esp  // prolog 2

		mov eax, dword ptr ss:[esp+0x1C] // variable name (param 2)
		mov eax, [eax] 
		push eax
		mov eax, dword ptr ss:[esp+0x30] // game object (param 1)
		push eax
		call ObjectPayLoad
		add esp, 0x8

		pop ebp		// restore register contents
		pop edi		
		pop esi
		pop ebx
		pop edx
		pop ecx

		xor eax, eax
		mov al, byte ptr ObjRet //check if we need to bypass the original function and return our value
		test eax, eax
		mov ObjRet, 0
		mov eax, oRes  //return value
		jnz ext  //don't call the original function
		
		mov eax, dword ptr ss:[esp+0x4] // arg 1
		push eax

		call GetLocalObjectNextHook // call original function

ext:
        retn 4
	}
}

*/


// WINAPI functions (the originals, not the hooks) are so predictable we don't need to bother with assembly.
// (they all begin with mov edi, edi + prolog)
int WINAPI sendtoProc(SOCKET s, const char *buf, int len, int flags, const sockaddr *to, int tolen) {
	struct sockaddr_in *caller;
	int ret;
	


	if(to != NULL && address_master != 0) {
		caller = (struct sockaddr_in *) to;		
		//if(caller->sin_port == htons(5121)) {
		if(caller->sin_addr.s_addr == address_bioware) {
			//fprintf(logFile, "Echoing data to valhalla\n");
			caller->sin_addr.s_addr = address_master;
			ret = ((int (WINAPI *)(SOCKET, const char *, int, int, const sockaddr *, int))sendtoOriginal)(s, buf, len, flags, to, tolen);
			caller->sin_addr.s_addr = address_bioware;
			// do not return here, allow original packet to be sent as usual.
		}
			
	}
	
	ret = ((int (WINAPI *)(SOCKET, const char *, int, int, const sockaddr *, int))sendtoOriginal)(s, buf, len, flags, to, tolen);
	return ret;
}


/// This solution works pretty well from basic testing (extensive testing not done yet).
/// The only real reason why I don't use it is the author of the listing, Skywing, recommended I 
/// duplicate traffic rather than re-direct it.  To enable the host-redirect, go to the top of this file 
/// and set USE_HOST to 1.  At present, I see no need to make this an option in NWNX.ini.
struct hostent * WINAPI gethostbynameProc(const char *name) {

	if(strcmp(name, "nwmaster.bioware.com") == 0 
		) {
		return (((struct hostent * (WINAPI *)(const char *))gethostbynameOriginal))(new_masterserver);
	}
	
	//if(strcmp(name, "nwn.master.gamespy.com") == 0) {
	//	return (((struct hostent * (WINAPI *)(const char *))gethostbynameOriginal))("api.mst.valhallalegends.com");
	//}
	

	return (((struct hostent * (WINAPI *)(const char *))gethostbynameOriginal))(name);
}



void PayLoad(char *gameObject, char **pname, char** ppvalue)
{
	int iValueLength;
	int iResultLength;
	char *name;

	name = *pname;
	
	if (!name || !ppvalue || !*ppvalue)
		return;

		
	char *value= (char*)*ppvalue;

	if(debuglevel>=3){
		fprintf(logFile, "name='%s'\n",name);
		fprintf(logFile, "value='%s'\n",value);
		if(gameObject == NULL)
			fprintf(logFile, "gameObject=NULL\n");
		fflush(logFile);
	}


	if (strncmp(name, "NWNX!", 5) != 0) 	// not for us
		return;

	char* library = &name[5];
	if (!library)
	{
		fprintf (logFile, "* Library not specified.");
		return;
	}

	char* function = strchr(library, '!');
	if (!function)
	{
		fprintf (logFile, "* Function not specified.");
		return;
	}

	// see if the library is already loaded
	*function = 0x0;
	CNWNXBase* pBase = (CNWNXBase*)Libraries.Lookup (library);
	*function = '!';
	if (pBase != NULL)
	{
		// library found, handle the request
		iValueLength = strlen(value);
		char* pRes = pBase->OnRequest(gameObject, function + 1, value);
		if (pRes != NULL)
		{
			if(strncmp(library,"LETO",4) != 0 &&
			   strncmp(library,"HASHSET",7) != 0)
			{
				// copy result into nwn variable value while respecting the maximum size
				// new plugins
				iResultLength = strlen(pRes);
				if (iValueLength < iResultLength)
				{
					free(value);
					*ppvalue = pRes;   // *ppvalue is where value was pointing
					*((unsigned long *)ppvalue+1) = strlen(pRes);
				}
				else
				{
					strncpy(value, pRes, iResultLength);
					*(value+iResultLength) = 0x0;
					free(pRes);
				}
			}
			else
			{
				// copy result into nwn variable value while respecting the maximum size
				// legacy plugins
				iResultLength = strlen(pRes);
				if (iValueLength < iResultLength)
				{
					strncpy(value, pRes, iValueLength);
					*(value+iValueLength) = 0x0;
				}
				else
				{
					strncpy(value, pRes, iResultLength);
					*(value+iResultLength) = 0x0;
				}

			}
		}
	}
	else
		fprintf(logFile, "* Library %s does not exist.\n", library);
}

void ObjectPayLoad(char *gameObject, char* name)
{
	if (!name)
		return;
	if(debuglevel>=3) {
		fprintf(logFile, "Object Request='%s'\n",name);
		fflush(logFile);
	}

	if (strncmp(name, "NWNX!", 5) != 0) 	// not for us
		return;

	char* library = &name[5];
	if (!library)
	{
		fprintf (logFile, "* Library not specified.");
		return;
	}

	char* function = strchr(library, '!');
	if (!function)
	{
		fprintf (logFile, "* Function not specified.");
		return;
	}

	// see if the library is already loaded
	*function = 0x0;
	CNWNXBase* pBase = (CNWNXBase*)Libraries.Lookup (library);
	*function = '!';
	if (pBase != NULL)
	{
		// library found, handle the request
		oRes = pBase->OnRequestObject(gameObject, function + 1);
		ObjRet = 1;
	}
	else
		fprintf(logFile, "* Library %s does not exist.", library);
}

void LoadLibraries()
{
	WIN32_FIND_DATA findData;
	memset(&findData, 0, sizeof(findData));
	char logDir[MAX_PATH];
	BOOL proceed = TRUE;
	char moduleName[MAX_PATH];

	

	strcpy (logDir, GetLogDir());

	// create hash table with default size
	if (!Libraries.Create())
	{
		fprintf (logFile, "* Critical error: creation of hash table failed!");
		return;
	}

	// search for all plugin DLL's
	HANDLE hSearch = FindFirstFile("nwnx_*.dll", &findData);
	if (hSearch == INVALID_HANDLE_VALUE)
	{
		fprintf(logFile, "* Error: No plugins found\n");
		return;
	}

	do
	{
		memset(moduleName, 0, MAX_PATH);
		char* pos = strchr(findData.cFileName, '_') + 1;
		char* end = strchr(pos, '.');
		strncpy (moduleName, pos, end - pos);


		// try to load the library
		HINSTANCE hDLL = LoadLibraryA(findData.cFileName);
		if (hDLL == NULL)
		{
			LPVOID lpMsgBuf;
			DWORD dw = GetLastError(); 
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM| FORMAT_MESSAGE_MAX_WIDTH_MASK ,
				NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR) &lpMsgBuf,	0, NULL);

			fprintf(logFile, "* An error occured while loading extension %s (%d: %s)\n", moduleName, dw, lpMsgBuf);
		    LocalFree(lpMsgBuf);
		}
		else
		{
			// create an instance of module
			GetClassObject = (GETCLASSOBJECT)GetProcAddress(hDLL, "GetClassObject");
			if (GetClassObject != NULL)
			{
				CNWNXBase* pBase = GetClassObject();
				if (pBase)
				{
					if (!pBase->OnCreate(logDir))
						fprintf(logFile, "* An error occured during OnCreate of %s\n", moduleName);
					else
						fprintf(logFile, "* Plugin %s is loaded.\n", moduleName);
					Libraries.Insert(_strupr(moduleName), pBase);
				}
				else
					fprintf(logFile, "* Could not create an instance of plugin %s", moduleName);
			}
			else
			{
				fprintf(logFile, "* Error: can not find creation function in %s\n", findData.cFileName);
			}
		}
		
		// find next module
		proceed = FindNextFile(hSearch, &findData);
	} while (proceed);

	FindClose(hSearch);
}

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

			outFile = CreateFile(
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

			iDirectory++;
		}
	}
	return logDir;
}


DWORD FindHook()
{
	char* ptr = (char*) 0x400000;
	while (ptr < (char*) 0x600000)
	{
		if ((ptr[0] == (char) 0x8b) &&
			(ptr[1] == (char) 0x44) &&
			(ptr[2] == (char) 0x24) &&
			(ptr[3] == (char) 0x04) &&
			(ptr[4] == (char) 0x6a) &&
			(ptr[5] == (char) 0x01) &&
			(ptr[6] == (char) 0x6a) &&
			(ptr[7] == (char) 0x03)
			)
			return (DWORD) ptr;
		else
			ptr++;

	}
	return NULL;
}

DWORD FindObjectHook()
{
	char* ptr = (char*) 0x400000;
	while (ptr < (char*) 0x600000)
	{
		if ((ptr[0] == (char) 0x8b) &&
			(ptr[1] == (char) 0x44) &&
			(ptr[2] == (char) 0x24) &&
			(ptr[3] == (char) 0x04) &&
			(ptr[4] == (char) 0x56) &&
			(ptr[11] == (char) 0x6a) &&
			(ptr[12] == (char) 0x00) &&
			(ptr[13] == (char) 0x6a) &&
			(ptr[14] == (char) 0x04)
			)
			return (DWORD) ptr;
		else
			ptr++;

	}
	return NULL;
}

void NewMasterServerInit() {
	clock_t c;
	float t;

	address_master = 0;

	// Try to get the master server, but give up after 10 seconds.
	c = clock();
	while(masterserver == NULL) {
		masterserver = gethostbyname(new_masterserver);
		t = ((float)c) / ((float)CLOCKS_PER_SEC);
		if(t > 10.0f) {
			break;
		}
	}

	// Repeat the same code block; let's get nwmaster.bioware.com now
	while(biowareserver == NULL) {
		biowareserver = gethostbyname("nwmaster.bioware.com");
		t = ((float)c) / ((float)CLOCKS_PER_SEC);
		if(t > 10.0f) {
			break;
		}
	}


	if(masterserver == NULL) {
		fprintf(logFile, "Could not resolve hostname: %s\n", new_masterserver);
	}
	else {
		address_master = *(unsigned long *)masterserver->h_addr_list[0];		
	}

	if(biowareserver == NULL) {
		fprintf(logFile, "Could not resolve hostname: nwmaster.bioware.com\n");
	}
	else {
		address_bioware = *(unsigned long *)biowareserver->h_addr_list[0];
	}

	if(address_master == 0) {
		fprintf(logFile, "Couldn't lookup new master server.  This is pretty bad.  Checking your DNS is advised.\n");
		fflush(logFile);
	}

	if(address_bioware == 0) {
		fprintf(logFile, "Couldn't lookup nwmaster.bioware.com.\n");
		fflush(logFile);
	}

}


DWORD WINAPI Init(LPVOID lpParam) 
{
	DWORD SLSHook = FindHook();
	DWORD GLOHook = FindObjectHook();
	BOOL listGame;

	// If you are debugging and wondering what to use for alignment, it's X - 5
	// So for example you are hooking 005C0380:
	//.text:005C0380                 push    0FFFFFFFFh
	//.text:005C0382                 push    offset loc_631448
	//.text:005C0387                 mov     eax, large fs:0
	// jump inst + address (5 bytes) would "hose" a portion of the "push offset loc_631448" because it extends to 
	// 005C0387 and NOT 005C0385.  The jump-back address would execute the partially overwitten op codes at 005C0385
	// and 005C0386.  So we increase the saved instructions and the jump-back address by +2.  So what you would do 
	// is pass an alignment of "2" (last argument to HookFunction) to fix this, so 7 instructions get saved instead, 
	// and the flow of execution returns to the correct spot.  Alignment can never be negative
	// (for example, we could not "stop" our calculation at 005C0382 because the minimum 5 bytes needed isn't reached)
	int SLSAlignment = 1;   // needs to be aligned by 1 byte.
	int GLOAlignment = 0;   // no alignment needed.  Just luck.


	strcpy(logFileName, GetLogDir());
	strcat(logFileName, "\\nwnx.txt");
	CIniFile iniFile ("nwnx.ini");


	debuglevel = iniFile.ReadInteger("NWNX", "debuglevel", 0);
	iniFile.ReadString("NWNX", "ListingService", new_masterserver, 256, DEFAULT_LISTING_SERVICE);
	listGame = iniFile.ReadBool("NWNX", "ListGame", true);



	logFile = fopen(logFileName, "w");

	NewMasterServerInit();

	if(SLSHook != NULL) {
		if(!HookFunction((void *)SetLocalStringHookProc,  &SetLocalStringNextHook, (void *)SLSHook, SLSAlignment)) {
			fprintf(logFile, "SLSHook -failed-\n");
			fflush(logFile);
		}

		if(debuglevel >= 3) {
			fprintf(logFile, "SLSHook=%X\n", SLSHook);
			fprintf(logFile, "Bridge address=%X\n", SetLocalStringNextHook);
			BridgeDump(logFile, SetLocalStringNextHook, SLSAlignment);
			fprintf(logFile, "Trampoline should bounce to: %X\n\n", InterpretAddress(SetLocalStringNextHook, SLSAlignment));
			fflush(logFile);
		}

	}
	if(GLOHook != NULL) {
		if(!HookFunction(GetLocalObjectHookProc, (void **) &GetLocalObjectNextHook, (void *) GLOHook, GLOAlignment)) {
			fprintf(logFile, "GLOHook -failed-\n");
			fflush(logFile);
		}

		if(debuglevel >= 3) {
			fprintf(logFile, "GLOHook=%X\n", GLOHook);
			fprintf(logFile, "Bridge address=%X\n", GetLocalObjectNextHook);
			BridgeDump(logFile, GetLocalObjectNextHook, GLOAlignment);
			fprintf(logFile, "Trampoline should bounce to: %X\n\n", InterpretAddress(GetLocalObjectNextHook, GLOAlignment));
			fflush(logFile);

		}
	}
	
	// Don't even put down hooks if the user wants it disabled, in case they are using a separate plugin that 
	// needs these hooks.
	if(listGame) {
		if(!USE_HOST) {
			if(!HookFunction(sendtoProc, (void **)&sendtoOriginal, sendto, WINDOWS_LIBRARY)) {
				fprintf(logFile, "sendto() hook failed, expect interruption in server registration services.\n");
				fflush(logFile); 
			}
		}
		else {
			HookFunction(gethostbynameProc, (void **) &gethostbynameOriginal, gethostbyname, WINDOWS_LIBRARY);
		}
	}
	

		

	fprintf(logFile, "NWN Extender V.2.71-beta1\n");
	fprintf(logFile, "(c) 2004 by Ingmar Stieger (Papillon) and Jeroen Broekhuizen\n");
	fprintf(logFile, "(c) 2007-2008 by virusman\n");
	fprintf(logFile, "(c) 2013 by addicted2rpg (https://github.com/addicted2rpg/NWNX2-windows)\n");
	fprintf(logFile, "visit us at http://www.nwnx.org\n\n");
	fprintf(logFile, "* Loading plugins...\n");
	LoadLibraries ();
	fprintf(logFile, "* NWNX2 activated.\n");
	fflush(logFile);

    return 0; 
} 




BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		DWORD dwThreadId;
		HANDLE hThread; 

		hThread = CreateThread(NULL, 0, Init, NULL, 0, &dwThreadId); 
		CloseHandle( hThread );
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		fprintf(logFile, "* NWNX2 shutting down...\n");
		Libraries.DeleteAll ();
		fprintf(logFile, "* NWNX2 shutdown successfull.\n");
		fclose(logFile);
	}

	return TRUE;
}


//	MessageBoxA(NULL, name, "Error", MB_TASKMODAL | MB_TOPMOST | MB_ICONERROR | MB_OK);