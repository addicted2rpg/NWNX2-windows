/***************************************************************************
    Chat plugin for NWNX  - hooks implementation
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

#include "HookChat.h"
#include "..\NWNXdll\hook_funcs.h"
#include "NWNXChat.h"
#include "..\RockLib\include\CExoString.h"

extern CNWNXChat chat;

void (*ChatNextHook)();
void (*pRunScript)();

dword pServThis = 0;
dword pList = 0;
dword pScriptThis = 0;
char *pChatThis = 0;

dword * (*pGetPCobj)();
// .text:0043CA00 ; CNWSMessage::SendServerToPlayerChatMessage(unsigned char, unsigned long, CExoString, unsigned long, CExoString const &)
int (*pChat)(unsigned char mode, unsigned long id, CExoString str, unsigned long to, CExoString const &p);

char scriptRun = 0;
char *lastMsg;
char lastIDs[32];

unsigned long lastRet;

void ChatHookProc(const int mode, const int id, const char *msg, const int len, const int to)
{
//  _asm { int 3 }
  _asm { pushad }
	if (!pChatThis) _asm { mov pChatThis, ecx };
  if (!scriptRun)
		lastRet = (unsigned long)chat.Chat(mode, id, msg, to);
  _asm { popad }
  _asm { leave }

	if (!scriptRun && lastRet)
  {
    _asm { mov eax, lastRet }
    _asm { retn 0x18 }
  }

  _asm { push eax }   // added for stack alingment with my hooking lib.  MOST plugins don't need this, but 
                      // virusman and dumbo are ruthlessly efficient :)  Its to counter a pop instruction designed 
                      // for undoing most prologs.
  _asm { jmp ChatNextHook }
}


DWORD FindChat()
{
/*
_asm{
        nop
        nop
//        int 3
        nop
        nop
        nop
}
*/
	char* ptr = (char*) 0x400000;
	while (ptr < (char*) 0x600000)
	{
// 43bdc0:  (as of 1.69, now 43CA00)
		if ((ptr[0] == (char) 0x6A) &&
			(ptr[1] == (char) 0xFF) &&
			(ptr[2] == (char) 0x68) &&
			(ptr[0x6A] == (char) 0xFC) &&
			(ptr[0x6B] == (char) 0x00) &&
			(ptr[0x6C] == (char) 0x00) &&
			(ptr[0x6D] == (char) 0x00) &&
			(ptr[0x72] == (char) 0x80) &&
			(ptr[0x73] == (char) 0x7C) &&
 			(ptr[0x74] == (char) 0x24) &&
 			(ptr[0x75] == (char) 0x4C) &&
 			(ptr[0x76] == (char) 0x02)
			)
			return (DWORD) ptr;
		else
			ptr++;
	}
	return NULL;
}

// 53 8B 5C 24 08 81 FB 00 00 00 7F  = 3 matches
// .text:00452F70 trails in 74 57 8B 87 88 00 01 00 8B 08 8B 01 85 C0 89 44 24 10 74 45 8B 8F 88 00 01 00 8B 09 50 E8 2F D0 FB
// .text:00462470  trails in 0F 84
// .text:00462C80 trails in 74 57 8B 87 88 00 01 00 8B 08 8B 01 85 C0 89 44 24 10 74 45 8B 8F 88 00 01 00 8B 09 50 E8 1F D3 FA
// 
DWORD FindGetPCobjByOID ()
{
    char *ptr = (char *) 0x400000;

    while (ptr < (char *) 0x600000)
    {
        if ((ptr[0] == (char) 0x53) &&
            (ptr[1] == (char) 0x8B) &&
            (ptr[2] == (char) 0x5C) &&
            (ptr[3] == (char) 0x24) &&
            (ptr[4] == (char) 0x08) &&
            (ptr[5] == (char) 0x81) &&
            (ptr[6] == (char) 0xfb) &&
            (ptr[7] == (char) 0x00) &&
            (ptr[8] == (char) 0x00) &&
            (ptr[9] == (char) 0x00) &&
            (ptr[10] == (char) 0x7F) && 
			(ptr[11] == (char) 0x56) && 
			(ptr[12] == (char) 0x57) &&
			(ptr[13] == (char) 0x8B) && 
			(ptr[14] == (char) 0xF9) &&
			(ptr[15] == (char) 0x74) &&
			(ptr[16] == (char) 0x57) &&
			(ptr[45] == (char) 0x2F)
            )
            return (unsigned long) ptr;
	else
	    ptr++;
    }
    return 0;
}

// 53 55 56 57 8B 7C 24 14 8B F1 8B CF
DWORD FindRunScript()
{
	char* ptr = (char*) 0x400000;
	while (ptr < (char*) 0x600000)
	{
		if ((ptr[0] == (char) 0x53) &&
			(ptr[1] == (char) 0x55) &&
			(ptr[2] == (char) 0x56) &&
			(ptr[3] == (char) 0x57) &&
			(ptr[4] == (char) 0x8B) &&
			(ptr[5] == (char) 0x7C) &&
			(ptr[6] == (char) 0x24) &&
			(ptr[7] == (char) 0x14) &&
			(ptr[8] == (char) 0x8B) &&
 			(ptr[9] == (char) 0xF1) &&
 			(ptr[0xA] == (char) 0x8B) &&
 			(ptr[0xB] == (char) 0xCF) &&
			(ptr[0xC] == (char) 0xE8) &&
			(ptr[0x11] == (char) 0x85) &&
			(ptr[0x12] == (char) 0xC0)
			)
			return (DWORD) ptr;
		else
			ptr++;
	}
	//64 A1 00 00 00 00 6A FF 68 ** ** ** ** 50 64 89 25 00 00 00 00 83 EC 1C 53 8B 5C 24 30
	ptr = (char*) 0x400000;
	while (ptr < (char*) 0x600000)
	{
		if ((ptr[0] == (char) 0x64) &&
			(ptr[1] == (char) 0xA1) &&
			(ptr[2] == (char) 0x00) &&
			(ptr[3] == (char) 0x00) &&
			(ptr[4] == (char) 0x00) &&
			(ptr[5] == (char) 0x00) &&
			(ptr[6] == (char) 0x6A) &&
			(ptr[7] == (char) 0xFF) &&
			(ptr[8] == (char) 0x68) &&
			(ptr[0xD] == (char) 0x50) &&
 			(ptr[0xE] == (char) 0x64) &&
 			(ptr[0xF] == (char) 0x89) &&
			(ptr[0x10] == (char) 0x25) &&
			(ptr[0x11] == (char) 0x00) &&
			(ptr[0x12] == (char) 0x00) &&
			(ptr[0x13] == (char) 0x00) &&
			(ptr[0x14] == (char) 0x00) &&
			(ptr[0x15] == (char) 0x83) &&
			(ptr[0x16] == (char) 0xEC) &&
			(ptr[0x17] == (char) 0x1C) &&
			(ptr[0x18] == (char) 0x53) &&
			(ptr[0x19] == (char) 0x8B) &&
			(ptr[0x1A] == (char) 0x5C) &&
			(ptr[0x1B] == (char) 0x24) &&
			(ptr[0x1C] == (char) 0x30) &&
			//8D 8E 70 01 00 00
			(ptr[0x3B] == (char) 0x8D) &&
			(ptr[0x3C] == (char) 0x8E) &&
			(ptr[0x3D] == (char) 0x70) &&
			(ptr[0x3E] == (char) 0x01) &&
			(ptr[0x3F] == (char) 0x00)
			)
			return (DWORD) ptr;
		else
			ptr++;
	}
	return NULL;
}

void RunScript(char * sname, int ObjID)
{
  int sptr[4];
  sptr[1] = strlen(sname);
  _asm {
    lea  edx, sptr
    mov  eax, sname
    mov  [edx], eax
    push 1
    push ObjID
    push edx
    mov ecx, pScriptThis
    mov ecx, [ecx]
  }
  scriptRun = 1;
  pRunScript();
  scriptRun = 0;
}



// Function unchanged largely from the original.
int SendMsg(const DWORD mode, const int id, char *msg, const int to)
{
	int nRetVal;
	DWORD len;

	//CExoString *c = new CExoString(msg);
	//CExoString *d = new CExoString(msg);

	
	if (pChat && pChatThis && msg)
	{
		
		len = strlen(msg) + 1;
		
		_asm {
		  push eax
		  push ebx
		  push ecx
		  push edx
		  push esi
		  push edi


		  push 0   // ??
		  push to
		  push len
		  push msg
		  push id 
		  push mode 

		  mov ecx, pChatThis 
		  call [pChat]
		  mov nRetVal, eax

		  pop edi
		  pop esi
		  pop edx
		  pop ecx
		  pop ebx
		  pop eax
		}
		

		/*
		_asm {
			mov ecx, pChatThis
		}
		pChat((unsigned char)mode, id, *c, to, NULL);
		*/

		//delete c;
		//delete d;


		return nRetVal;
		//pChat(mode, id, msg, len, to);
	}
	else return 0;
}

DWORD *GetPCobj(dword OID)
{

	
  _asm {
    mov  ecx, pServThis
    mov  ecx, [ecx]
    mov  ecx, [ecx+4]
    mov  ecx, [ecx+4]
    push OID

  }
  
  return pGetPCobj();

}

unsigned long GetID(DWORD OID)
{
	dword * pcObj = GetPCobj(OID);
	if(!pcObj) {
		return NULL;
	}


	return *(pcObj+1); // +1 dword = +4
}

int HookFunctions()
{
	int success1 = 0, success2 = 0;
	DWORD org_Chat = FindChat();
	DWORD org_Run  = FindRunScript();
	DWORD org_Get  = FindGetPCobjByOID();
	if (org_Chat)
	{

		// (3:10:59 PM) virusman: so pServThis is a pointer to the ServerExoApp afaik
		//dword * (__fastcall *pGetPCObj)(void *pThis, int edx, unsigned int OID)
		//(3:50:35 PM) virusman: that's how __fastcall can be used to emulate __thiscall
		//(3:51:09 PM) virusman: on Windows, with __thiscall, this goes to ecx, and method arguments are pushed onto the stack

		pServThis = *(DWORD *)(org_Chat + 0x1f);
		pScriptThis = pServThis - 8;

		//success1 = HookCode((PVOID) org_Chat, ChatHookProc, (PVOID*) &ChatNextHook);
		success1 = HookFunction( ChatHookProc, (PVOID*) &ChatNextHook, (PVOID) org_Chat, 2);
		*(dword*)&pChat = org_Chat;   // nothing wrong with hooking our own message
		
	}

	if (org_Chat && success1) {
		fprintf(chat.m_fFile, "! ChatProc hooked at %x.\n", org_Chat);
	}
	else {
		fprintf(chat.m_fFile, "X Could not find Chat function or hook failed: %x\n", org_Chat);
	}
	
	if (org_Run) {
		*(dword*)&pRunScript = org_Run;
		fprintf(chat.m_fFile, "! RunProc located at %x.\n", org_Run);
	}
	else
		fprintf(chat.m_fFile, "X Could not find Run function: %x\n", org_Run);
	
	if (org_Get) {
		*(dword*)&pGetPCobj = org_Get;
		fprintf(chat.m_fFile, "! GetIDProc located at %x.\n", org_Get);
	}
	else
		fprintf(chat.m_fFile, "X Could not find GetID function: %x\n", org_Get);
	
	
	fprintf(chat.m_fFile, "\n");
	fflush(chat.m_fFile);
	
	return (org_Chat && org_Run && org_Get && pServThis && pScriptThis && success1);
}

