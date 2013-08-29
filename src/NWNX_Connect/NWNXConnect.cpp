/***************************************************************************
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


#include "NWNXConnect.h"
#include "../NWNXdll/IniFile.h"
#include "../NWNXdll/hook_funcs.h"
#include "types.h"

extern int __fastcall CNWSMessage__HandlePlayerToServerMessage(CNWSMessage *pMessage, void *, unsigned long nPlayerID, char *pData, unsigned long nLen);
extern int (__fastcall *CNWSMessage__HandlePlayerToServerMessageNext)(CNWSMessage *pMessage, void *, unsigned long nPlayerID, char *pData, unsigned long nLen);

BOOL CNWNXConnect::OnRelease( )
{
	Log( 0,"o Shutting down\n" );
	return CNWNXBase::OnRelease();
}

BOOL CNWNXConnect::OnCreate(const char* LogDir)
{
	char log[MAX_PATH];
	int nRet;
	
	sprintf_s(log, MAX_PATH *sizeof(char), "%s\\nwnx_connect.txt", LogDir);

	// call the base class function
	if (!CNWNXBase::OnCreate(log))
		return false;

	WriteLogHeader();

	CIniFile iniFile ("nwnx.ini");

//	nRet = HookCode( (PVOID) 0x5426A0, CNWSMessage__HandlePlayerToServerMessage, (PVOID*) &CNWSMessage__HandlePlayerToServerMessageNext);
	nRet = HookFunction((void *) CNWSMessage__HandlePlayerToServerMessage, 
					    (void **) &CNWSMessage__HandlePlayerToServerMessageNext, 
						(void *)0x5426A0, 
						2);

	if( nRet ){
		fprintf(m_fFile, "! HandlePlayerToServerMessage() hooked at 0x5426A0 .\n");
	}	
	else{
		fprintf(m_fFile, "X HandlePlayerToServerMessage() hook at 0x5426A0 failed!\n");
	}

	
	fprintf(m_fFile, "\n" );
	fflush(m_fFile);


	return true;
}
unsigned long CNWNXConnect::OnRequestObject (char *gameObject, char* Request){

	return OBJECT_INVALID;
}

char* CNWNXConnect::OnRequest(char *gameObject, char* Request, char* Parameters){

	return NULL;
}
void CNWNXConnect::WriteLogHeader()
{
	fprintf(m_fFile, "NWNX Connect v1.2 alpha created by Shadooow based on Virusman's linux original, updated by addicted2rpg\n\n");
	fflush (m_fFile);
}