#include "NWNXConnect.h"


CNWNXConnect nwnxConnect;




extern "C" __declspec(dllexport) CNWNXBase* GetClassObject()
{

	return &nwnxConnect;
}

BOOL APIENTRY DllMain (HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}