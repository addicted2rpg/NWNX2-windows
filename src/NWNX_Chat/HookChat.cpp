/***************************************************************************
    Chat plugin for NWNX  - hooks implementation
    (c) 2005,2006 dumbo (dumbo@nm.ru)
    (c) 2006-2007 virusman (virusman@virusman.ru)

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
//#include "madCHook.h"
#include "..\NWNXdll\hook_funcs.h"
#include "NWNXChat.h"

extern CNWNXChat chat;

void (*ChatNextHook)();
void (*pRunScript)();

dword pServThis = 0;
dword pList = 0;
dword pScriptThis = 0;
char *pChatThis = 0;

dword * (*pGetPCobj)();
int (*pChat)(int mode, int id, char *msg, int len, int to);

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

/*
SendMsg below looks like this in the debugger-


nwnx_chat.dll:07E51692 loc_7E51692:                            ; CODE XREF: nwnx_chat.dll:07E51697j
nwnx_chat.dll:07E51692 mov     cl, [eax]              /// Loop, most likely strlen
nwnx_chat.dll:07E51694 inc     eax
nwnx_chat.dll:07E51695 test    cl, cl
nwnx_chat.dll:07E51697 jnz     short loc_7E51692     /// End loop, most likely strlen
nwnx_chat.dll:07E51699 sub     eax, edx
nwnx_chat.dll:07E5169B mov     [ebp-8], eax
nwnx_chat.dll:07E5169E push    0
nwnx_chat.dll:07E516A0 push    dword ptr [ebp+14h]
nwnx_chat.dll:07E516A3 push    dword ptr [ebp-8]
nwnx_chat.dll:07E516A6 push    dword ptr [ebp+10h]
nwnx_chat.dll:07E516A9 push    dword ptr [ebp+0Ch]
nwnx_chat.dll:07E516AC push    dword ptr [ebp+8]
nwnx_chat.dll:07E516AF mov     ecx, ds:dword_7E64434    // In a particular run, 0
nwnx_chat.dll:07E516B5 call    ds:off_7E64428      // definitely calls Chat
nwnx_chat.dll:07E516BB mov     [ebp-4], eax
nwnx_chat.dll:07E516BE mov     eax, [ebp-4]
nwnx_chat.dll:07E516C1 mov     esp, ebp
nwnx_chat.dll:07E516C3 pop     ebp
nwnx_chat.dll:07E516C4 retn


Stack frame of a particular run of Chat ('blah' in Talk)-
Stack[00001894]:0018FC94 ; [BEGIN OF STACK FRAME Chat. PRESS KEYPAD "-" TO COLLAPSE]
Stack[00001894]:0018FC94 db  18h          // this is the return address pushed onto the stack from an ASM 'call' directive
Stack[00001894]:0018FC95 db  50h ; P
Stack[00001894]:0018FC96 db  53h ; S
Stack[00001894]:0018FC97 db    0
Stack[00001894]:0018FC98 arg_0 dd 457F9901h   // The last BYTE (i.e., 01) appears to be the channel.  Code doesn't seem to use the other stuff in Chat()
Stack[00001894]:0018FC9C arg_4 dd 7FFFFFFDh    // Some kind of PC identifier
Stack[00001894]:0018FCA0 arg_8 dd offset aBlah_0                 ; Obviously the message
Stack[00001894]:0018FCA0                                         ; "blah"
Stack[00001894]:0018FCA4 db    5   // length of the message
Stack[00001894]:0018FCA5 db    0
Stack[00001894]:0018FCA6 db    0
Stack[00001894]:0018FCA7 db    0
Stack[00001894]:0018FCA8 arg_10 dd 0FFFFFFFFh    // probably 'to' ID
Stack[00001894]:0018FCA8 ; [END OF STACK FRAME Chat. PRESS KEYPAD "-" TO COLLAPSE]


*/


int SendMsg(const int mode, const int id, char *msg, const int to)
{
	int nRetVal;
	if (pChat && pChatThis && msg)
	{
		int len = strlen(msg);
		
		_asm { 
		  push to
		  push len
		  push msg  
		  push id 

// OK LISTEN UP!  "mode" is a DWORD that I can't replicate at the moment, but it looks like this:
// 0xAABBBBCC
// AA = Packet ID.  It keeps incrementing.  I've seen this a thousand times in NWN-network debugging.
// BBBB = CRC value used in UDP error correction.  I have the code that will generate this (from another project), 
//        but I am not putting it in until I find a way to spoof AA (the Packet ID byte).  Just putting a number 
//        - even a valid one - won't work unless we increment it inside the server engine or the player will 
//        de-synch and lose connection.
// CC = this is the 'mode' that is passed to this function.  Its the actual chat channel the message is on.
//      It is typically a number 1-6 representing the various NWN chat channels.
//      this value is masked off the DWORD starting at 0043CA7E in your neighborhood IDA debugger.
//
// Oh, and the application crashes if you send 000000CC, or basically "push mode" like below as is :P
		  push mode 
		  mov ecx, pChatThis // This pChatThis MIGHT be represented by the following instructions (in order, left to right):
			                 // mov ecx, [ecx+4] --> mov ecx, [ecx+4] --> mov ecx, [ecx+10018h]
							 // As pChatThis, it appears to be pointing correctly, so no need to do this.
		  call [pChat]
		  mov nRetVal, eax
		}
		/*
		_asm { 
			//push return_address that was on the stack from last call. i.e., 00535018
			push mode   // there is a lot more to the mode encoding, but it appears it only uses the last BYTE.
			push id
			push msg
			push len
			push to //push arg_10_mystery_FFFFFFFF, probably the RECIPIENT...
			mov ecx, pChatThis  
			call [pChat]
			mov nRetVal, eax
		}
		*/


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

   

	//dword * (__fastcall *pGetPCObj)(void *pThis, int edx, unsigned int OID)
	//(3:50:35 PM) virusman: that's how __fastcall can be used to emulate __thiscall
	//(3:51:09 PM) virusman: on Windows, with __thiscall, this goes to ecx, and method arguments are pushed onto the stack

	// Wants ExoAppInternal
	// CNWSPlayer * __thiscall CServerExoAppInternal::GetClientObjectByObjectId(unsigned long)
/*  
	DWORD *x;
	DWORD raw;
	x = (DWORD *)pServThis;
	x = (DWORD *) *x;
//	fprintf(chat.m_fFile, "x--> %X\n", x);
	raw = (DWORD) x;
	raw = raw + 4;
	x = (DWORD *) raw;
	x = (DWORD *) *x;
//	fprintf(chat.m_fFile, "x--> %X\n", x);
	raw = (DWORD) x;
	raw = raw + 4;
	x = (DWORD *)raw;
	x = (DWORD *) *x;
//	fprintf(chat.m_fFile, "x--> %X\n", x);
	
	_asm {
		mov ecx, x
		push OID
	}
	
	
	x = pGetPCobj();
	fprintf(chat.m_fFile, "pGetPCobj() called.  x=%X\n", x);
	fflush(chat.m_fFile);
	return x;
*/	
}

unsigned long GetID(DWORD OID)
{
	dword * pcObj = GetPCobj(OID);
	if(!pcObj) {
		return NULL;
	}
	if(chat.m_LogLevel >= chat.logAll) {
		fprintf(chat.m_fFile, "pcObj+1 = %X, *(pcObj+1)= %X\n", pcObj+1, *(pcObj+1));
		fflush(chat.m_fFile);
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
		pServThis = *(DWORD *)(org_Chat + 0x1f);
		pScriptThis = pServThis - 8;

		//success1 = HookCode((PVOID) org_Chat, ChatHookProc, (PVOID*) &ChatNextHook);
		success1 = HookFunction( ChatHookProc, (PVOID*) &ChatNextHook, (PVOID) org_Chat, 2);
		*(dword*)&pChat = org_Chat;   // nothing wrong with hooking our own message
		// *(DWORD *)&pChat = (DWORD)ChatNextHook + (DWORD)16;  
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

