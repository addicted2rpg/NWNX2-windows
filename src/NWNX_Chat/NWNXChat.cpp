/***************************************************************************
    Chat plugin for NWNX - Implementation of the CNWNXChat class.
    (c) 2005,2006 dumbo (dumbo@nm.ru)
    (c) 2006-2007 virusman (virusman@virusman.ru)
	(c) 2013 addicted2rpg (duckbreath@yahoo.com)

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

#include "NWNXChat.h"
#include "HookChat.h"
#include "../NWNXdll/IniFile.h"
//#include "nwn_crc.h"
#include <stdio.h>
#include "..\RockLib\include\types.h"
#include "..\RockLib\include\CExoString.h"
#include "..\RockLib\include\CNWSMessage.h"


extern char *pChatThis;


//.text:0043DEA0 ; CNWSMessage::SendServerToPlayerChat_ServerTell(unsigned long pID, CExoString str)
//int 		(__thiscall *CNWSMessage__SendServerToPlayerChat_ServerTell)(CNWSMessage *pTHIS, uint32_t Receiver, CExoString Msg) = (int (__thiscall*)(CNWSMessage *pTHIS, uint32_t Receiver, CExoString Msg))0x0043DEA0;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNWNXChat::CNWNXChat()
{
	m_LogLevel = processNPC = ignore_silent = 0;
	confKey = "CHAT";
	heapAddress = (DWORD *)0x5EEFF00;  // nwserver keeps their heap's address at this address.
}

CNWNXChat::~CNWNXChat()
{
}

BOOL CNWNXChat::OnCreate (const char* LogDir)
{
	// call the base class function
	char log[MAX_PATH];
	sprintf_s(log, sizeof(char) * MAX_PATH, "%s\\nwnx_chat.txt", LogDir);
	if (!CNWNXBase::OnCreate(log))
		return false;
    WriteLogHeader();
	LoadConfiguration();
	lastMsg = new char[maxMsgLen+13];
	return HookFunctions();
}

int CNWNXChat::SendServerMessage(char *sMessage, int nRecipientID) {
	int retval;
	DWORD fn = 0x0043DEA0;

	
	CExoString *c = new CExoString(sMessage);
	CNWSMessage *m = new CNWSMessage();
	
	//retval = m->SendServerToPlayerChat_ServerTell(nRecipientID, *c);
	
	
	retval = strlen(sMessage) + 1;
	__asm {
		push eax
		push ebx
		push ecx
		push edx
		push esi
		push edi

		mov eax, 0  		            
		mov ebx, 1  
		mov esi, 1  

		// the crappy edi hack... not even sure if its even looked at.
		push 0
		lea edi, [esp]
		push edi
		lea edi, [esp]
		push edi
		lea edi, [esp]

		push retval
		push sMessage
		push nRecipientID
		mov ecx, pChatThis 
		call fn
		mov retval, eax

		// or just add esp, 0xC
		pop eax
		pop eax
		pop eax

		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		pop eax
	}
	
	delete c;
	delete m;
	return retval;
}

char *CNWNXChat::NWNXSendMessage(char* Parameters)
{
	int oSender, oRecipient, nResult;
	DWORD nChannel;
	char *sMessage;
	HANDLE sharedHeap;

    if (m_LogLevel >= logAll)
		Log("o SPEAK: %s\n", Parameters);

	if(m_LogLevel >= logAll) {
		Log("[+-+-+-+Heap Love-+-+-+-+\n\tGetProcessHeap() located at %X\n", GetProcessHeap());
		Log("\tNWN is storing its heap handle at %X\n", *heapAddress);
	}

	sharedHeap = (HANDLE) *heapAddress;

    int nParamLen = strlen(Parameters);
    char *nLastDelimiter = strrchr(Parameters, '�');
    if (!nLastDelimiter || (nLastDelimiter-Parameters)<0)
    {
		if (m_LogLevel >= logAll)
			Log("o nLastDelimiter error\n");
		return "0";
    }

	int nMessageLen = nParamLen-(nLastDelimiter-Parameters)+1;
	

	sMessage = (char *)HeapAlloc(sharedHeap, NULL, nMessageLen); //new char[nMessageLen];

    if (sscanf_s(Parameters, "%x�%x�%d�", &oSender, &oRecipient, &nChannel) < 3)
    {
		if (m_LogLevel >= logAll)
			Log("o sscanf error\n");
		//delete[] sMessage;
		HeapFree(sharedHeap, NULL, sMessage);
		return "0";
    }
	strncpy_s(sMessage, sizeof(char) * nMessageLen, nLastDelimiter+1, nMessageLen-1);
//	if (m_LogLevel >= logAll) Log("o sMessage='%s', oID=%X, ", sMessage, oRecipient);
    int nRecipientID = GetID(oRecipient);
	if (m_LogLevel >= logAll) Log("GetID()=%d\n", nRecipientID);
    if ((nChannel==4 || nChannel==20) && oRecipient<=0x7F000000)
    {
		if (m_LogLevel >= logAll)
			Log("o oRecipient is not a PC\n");
		// delete[] sMessage;
		HeapFree(sharedHeap, NULL, sMessage);
		return "0";
    }

    if(nChannel!=4 && nChannel!=20) {
		if(nChannel != 5)
			nRecipientID=-1;
	}
	

	if(nChannel == 5) {
		oSender = oRecipient;
	} 
	else {
	}
    if (m_LogLevel >= logAll)
		Log("o SendMsg(%d, %08lX, '%s', %d)\n", nChannel, oSender, sMessage, nRecipientID);
	switch(nChannel) {
		case 5:
			nResult = this->SendServerMessage(sMessage, nRecipientID); 
			// Bioware frees this.  Subsequent calls can result in heap corruption.
			// It does this at address line .text:00602642 and 'esi' (arg 3) is the same pointer as sMessage.  
			// Don't free it!  A memory leak should NOT result.
			// HeapFree(sharedHeap, NULL, sMessage);
			if(nResult) return "1";
			break;
		default:
			nResult = SendMsg(nChannel, oSender, sMessage, nRecipientID);
			break;
	}


	if (m_LogLevel >= logAll)
		Log("o Return value: %d\n", nResult); //return value for full message delivery acknowledgement
	//delete[] sMessage;

	if(nResult) return "1";
	else return "0";
}


char* CNWNXChat::OnRequest (char* gameObject, char* Request, char* Parameters)
{
	char *returnStr;

	if (strncmp(Request, "GETID", 5) == 0)
	{
		unsigned long OID;
		unsigned long retrievedID;
		for(retrievedID=0; retrievedID < 17;retrievedID++) {
			if(Parameters[retrievedID] == ' ') {
				Parameters[retrievedID] = 0;
				break;
			}
		}

		OID = strtol(Parameters, NULL, 16);

		if (m_LogLevel >= logScripter) Log("o GETID: sOID='%s' =? iOID='%x'\n", Parameters, OID);

		if (!OID) {
			strcpy_s(Parameters, 17,  "-1");
		}
		else {
			retrievedID = GetID(OID);
			sprintf_s(Parameters, 17, "%ld", retrievedID);
		}

		if (m_LogLevel >= logScripter) Log("o GETID: ID=%s\n", Parameters);

		//mytest = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(char)* strlen(Parameters) + 1);
		//strcpy(mytest, Parameters);
		return NULL;
	}
	else if (strncmp(Request, "LOGNPC", 6) == 0)
	{
		processNPC = atoi(Parameters);
	}
	else if (strncmp(Request, "IGNORESILENT", 12) == 0)
	{
		ignore_silent = atoi(Parameters);
		if (m_LogLevel >= logScripter)
			Log("o ignore_silent = %d\n", ignore_silent);
	}
	else if (strncmp(Request, "SPEAK", 5) == 0)
	{
		char *sReturn = NWNXSendMessage(Parameters);
		strncpy_s(Parameters, sizeof(char) * (strlen(Parameters) + 1), sReturn, strlen(Parameters));
		//Parameters[strlen(sReturn)] = 0;
		return NULL;
	}	
	else if(strncmp(Request, "GETCHATSCRIPT", 13) == 0) {
		returnStr = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(char)* strlen(chatScript) + 1);
		strncpy_s(returnStr, sizeof(char)*strlen(chatScript) + 1, chatScript, strlen(chatScript));

		return returnStr;
		//strncpy_s(Parameters, 17, chatScript, strlen(chatScript));
		//return NULL;
	}

	if (!scriptRun) return NULL; // all following cmds - only in chat script
	
	if (strncmp(Request, "TEXT", 4) == 0)
	{
		// Because of the spacer in NWScript, Parameters should be 1024 easily.
		// strcpy_s(Parameters, 1024, lastMsg);

		unsigned int length = strlen(lastMsg);
		//char *ret = (char *) malloc(length+1);
		char *ret = (char *) HeapAlloc(GetProcessHeap(), 0, (length+1));
		strncpy_s(ret, (length+1)*sizeof(char), lastMsg, length);
		ret[length]=0;
		return ret;
	}
	else if (strncmp(Request, "LOG", 3) == 0) {
		Log("%s", Parameters);
	}
	else if (strncmp(Request, "SUPRESS", 7) == 0)
	{
		if(m_LogLevel >= logAll) {
			Log("o SUPPRESS called.\n");
		}
		if (atoi(Parameters)==1)
			supressMsg = 1;
	}    	
	return NULL;
}

BOOL CNWNXChat::OnRelease ()
{
	Log ("o Shutdown.\n");
	// call base class function
	return CNWNXBase::OnRelease();
}

// Called from the hook in nwserver
int CNWNXChat::Chat(const int mode, const int id, const char *msg, const int to)
{

	if ( !msg ) return 0; // don't process null-string
	

	int cmode = mode & 0xFF;
	if (m_LogLevel >= logAll)
		Log("o CHAT: mode=%lX, from_oID=%08lX, msg='%s', to_ID=%08lX\n", cmode, id, (char *)msg, to);
	sprintf_s(lastMsg, sizeof(char) * (maxMsgLen+13), "%02d%10d", cmode, to);
	strncat_s(lastMsg, sizeof(char) * (maxMsgLen+13), (char*)msg, maxMsgLen);
	supressMsg = 0;
	if (ignore_silent && (cmode==0xD || cmode==0xE)) return 0;
	if ( (processNPC && id != 0x7F000000) || (!processNPC && (unsigned long)id >> 16 == 0x7FFF) )
	{
		RunScript(chatScript, id);
	}
	else if (cmode==5 && id==0x7F000000) {
		RunScript(servScript, to);
	}
	return supressMsg;
}

void CNWNXChat::LoadConfiguration ()
{
	CIniFile iniFile ("nwnx.ini");

	// read log file setting
	m_LogLevel = iniFile.ReadLong(confKey, "LogLevel", 0);
	maxMsgLen = iniFile.ReadLong(confKey, "max_msg_len", 1024);
	processNPC = iniFile.ReadLong(confKey, "processnpc", 0);
	ignore_silent = iniFile.ReadLong(confKey, "ignore_silent", 0);
	iniFile.ReadString(confKey, "chat_script", chatScript, 16, "chat_script");
	iniFile.ReadString(confKey, "server_script", servScript, 16, "server_script");

	if (m_LogLevel > 0)
	{
		fprintf(m_fFile, "Settings:\n");
		fprintf(m_fFile, "chat_script: %s\n", chatScript);
		fprintf(m_fFile, "server_script: %s\n", servScript);
		fprintf(m_fFile, "max_msg_len: %d\n", maxMsgLen);
		fprintf(m_fFile, "processnpc: %d\n", processNPC);
		fprintf(m_fFile, "ignore_silent: %d\n\n", ignore_silent);
	}
}

void CNWNXChat::WriteLogHeader()
{
	fprintf(m_fFile, "NWNX Chat version 0.3.4 for Windows.\n");
	fprintf(m_fFile, "(c) 2005-2006 by dumbo (dumbo@nm.ru)\n");
	fprintf(m_fFile, "(c) 2006-2008 virusman (virusman@virusman.ru)\n");
}