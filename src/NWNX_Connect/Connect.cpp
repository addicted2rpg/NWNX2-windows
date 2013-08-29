#include "NWNXConnect.h"

CNWNXConnect Connect;


#include "../NWNXdll/IniFile.h"
#include "nwn_internals.h"
#include "types.h"


int (__fastcall *CNWSMessage__HandlePlayerToServerMessageNext)(CNWSMessage *pMessage, void *, unsigned long nPlayerID, char *pData, unsigned long nLen);

void SendHakList(CNWSMessage *pMessage, int nPlayerID)
{
	unsigned char *pData;
	long unsigned int nSize;

	CNWSModule *pModule = (CNWSModule *) (*NWN_AppManager)->app_server->srv_internal->GetModule();
	if(pModule)
	{
		Connect.Log(0, "Sending hak list...\n");
		CNWMessage *message = (CNWMessage*)pMessage;
	    message->CreateWriteMessage(80, -1, 1);
		message->WriteINT(pModule->HakList.alloc, 32);
		for(int i = pModule->HakList.alloc - 1; i >= 0; --i)
		{
			message->WriteCExoString(pModule->HakList.data[i], 32);//TODO
			Connect.Log(0, "%s\n", pModule->HakList.data[i].text);//TODO
		}
		message->WriteCExoString(pModule->m_sCustomTLK, 32);
		Connect.Log(0, "%s\n", pModule->m_sCustomTLK.text);
		message->GetWriteMessage((char **) &pData, (uint32_t *) &nSize);
		pMessage->SendServerToPlayerMessage(nPlayerID, 100, 1, pData, nSize);
	}
}

int __fastcall CNWSMessage__HandlePlayerToServerMessage(CNWSMessage *pMessage, void *, unsigned long nPlayerID, char *pData, unsigned long nLen)
{
	int nType = pData[1];
	int nSubtype = pData[2];
	Connect.Log(0, "Message: PID %d, type %x, subtype %x\n", nPlayerID, nType, nSubtype);
	if(nType == 1)
	{
		SendHakList(pMessage, nPlayerID);
	}

	return CNWSMessage__HandlePlayerToServerMessageNext(pMessage, NULL, nPlayerID, pData, nLen);
}








extern "C" __declspec(dllexport) CNWNXBase* GetClassObject()
{
	return &Connect;
}

BOOL APIENTRY DllMain (HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}