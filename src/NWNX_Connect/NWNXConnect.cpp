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
//#include "..\NWNXdll\IniFile.h"
#include "..\NWNXdll\hook_funcs.h"

//extern int __stdcall CNWSMessage__HandlePlayerToServerMessageFilter(CNWSMessage *pMessage, void *, unsigned long nPlayerID, char *pData, unsigned long nLen);
//extern int (__stdcall *CNWSMessage__HandlePlayerToServerMessageNext)(CNWSMessage *pMessage, void *, unsigned long nPlayerID, char *pData, unsigned long nLen);
extern CNWNXConnect nwnxConnect;
void *CNWSMessage__HandlePlayerToServerMessageBridge = NULL;	






//int __stdcall CNWSMessage__HandlePlayerToServerMessageFilter(CNWSMessage *pMessage, unsigned char *p1, unsigned long nPlayerID, char *pData, unsigned long nLen)
void __fastcall CNWSMessage__HandlePlayerToServerMessageFilter(CNWSMessage *pMessage, DWORD c_edx, DWORD nPlayerID, BYTE *pData, DWORD nLen)
{


	nwnxConnect.Log(0, "Message: PID %d, type %x, subtype %x\n", nPlayerID, pData[1], pData[2]);


	
	if(pData[1] == 1)
	{
//		nwnxConnect.SendHakList(pMessage, nPlayerID);
	}

	
	/*
	(((int (__fastcall *)( CNWSMessage * , DWORD,  DWORD, BYTE *, DWORD)) 
		CNWSMessage__HandlePlayerToServerMessageBridge)) 
	   (pMessage, c_edx, nPlayerID, pData, nLen);
	 */
	_asm {
		pop eax;
		pop ebp;
	}
	(((int (__thiscall *)( void )) 
		CNWSMessage__HandlePlayerToServerMessageBridge)) 
	   ();
	
}




CNWNXConnect::CNWNXConnect() {
	
}

CNWNXConnect::~CNWNXConnect() {

}


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

	// Doesn't use any parameters I know of.
	//CIniFile iniFile ("nwnx.ini");

//	nRet = HookCode( (PVOID) 0x5426A0, CNWSMessage__HandlePlayerToServerMessage, (PVOID*) &CNWSMessage__HandlePlayerToServerMessageNext);
	

	nRet = HookFunction((void *) CNWSMessage__HandlePlayerToServerMessageFilter, 
					    (void **) &CNWSMessage__HandlePlayerToServerMessageBridge, 
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



char *CNWNXConnect::OnRequest(char *gameObject, char* Request, char* Parameters){

	return NULL;
}
void CNWNXConnect::WriteLogHeader()
{
	fprintf(m_fFile, "NWNX Connect v1.2 alpha created by Shadooow based on Virusman's linux original, updated by addicted2rpg\n\n");
	fflush (m_fFile);
}


void CNWNXConnect::SendHakList(CNWSMessage *pMessage, int nPlayerID)
{

	unsigned char *pData;
	long unsigned int nSize;


	CNWSModule *pModule = (CNWSModule *) (*NWN_AppManager)->app_server->srv_internal->GetModule();
	if(pModule)
	{
		Log(0, "Sending hak list...\n");
		CNWMessage *message = (CNWMessage*)pMessage;
	    message->CreateWriteMessage(80, -1, 1);
		

		message->WriteINT(pModule->HakList.alloc, 32);
		
		for(int i = pModule->HakList.alloc - 1; i >= 0; --i)
		{
			message->WriteCExoString(pModule->HakList.data[i], 32);//TODO
			Log(0, "%s\n", pModule->HakList.data[i].text);//TODO
		}
		message->WriteCExoString(pModule->m_sCustomTLK, 32);
		Log(0, "%s\n", pModule->m_sCustomTLK.text);
		message->GetWriteMessage((char **) &pData, (uint32_t *) &nSize);
		pMessage->SendServerToPlayerMessage(nPlayerID, 100, 1, pData, nSize);
	}

	
}




