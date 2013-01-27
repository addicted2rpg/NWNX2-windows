#include <Windows.h>
#include "hook_funcs.h"


int JmpTo(void *MemoryBuffer, void *JumpDestination)
{
	unsigned long OldProtection;
	unsigned char *DestinationMemory = (unsigned char *) MemoryBuffer;

	
	if (!VirtualProtect(MemoryBuffer, 5, PAGE_EXECUTE_READWRITE, &OldProtection))
		return 0;

	// Place a jump code to an allocated place on the heap
	*DestinationMemory = 0xE9;
	DestinationMemory++;
	*((unsigned long *) DestinationMemory) = (((unsigned char *) JumpDestination) - DestinationMemory - 4);

	if (!VirtualProtect(MemoryBuffer, 5, OldProtection, &OldProtection))
		return 0;

	return 1;
}



// poor man's prolog
int CreateStackFrame(void *MemoryBuffer)
{
	unsigned long OldProtection;

	unsigned char *DestinationMemory = (unsigned char *)MemoryBuffer;

	if (!VirtualProtect(MemoryBuffer, 5, PAGE_EXECUTE_READWRITE, &OldProtection))
		return 0;

	// mov edi, edi
	*((unsigned long *) DestinationMemory) = 0xFF89;
	DestinationMemory += 2;

	// push ebp
	*((unsigned long *) DestinationMemory) = 0x55;
	DestinationMemory++;

	// mov ebp, esp
	*((unsigned long *) DestinationMemory) = 0xEC8B;
	DestinationMemory += 2;

	if (!VirtualProtect(MemoryBuffer, 5, OldProtection, &OldProtection))
		return 0;

	return 1;
}



int CreateGeneralBridge(void **BridgePointer, void *fn)
{
		HANDLE ProcessHeap;
		DWORD OldProtection;


	// Create the bridge
	ProcessHeap = GetProcessHeap();
	if (!ProcessHeap)
		return 0;

	*BridgePointer = HeapAlloc(ProcessHeap, HEAP_ZERO_MEMORY, 10);
	if (!*BridgePointer)
		return 0;

	// Make it executable
	if (!VirtualProtect(*BridgePointer, 10, PAGE_EXECUTE_READWRITE, &OldProtection))
		return 0;

	// Restore the stackframe code
	if (!CreateStackFrame(*BridgePointer))
		return 0;

	// Add a jump to the rest of the function
	if (!JmpTo(((unsigned char *) *BridgePointer) + 5, ((unsigned char *) fn) + 5))
		return 0;

	return 1;
}





int HookFunction(void *FilterFunction, void **BridgePointer, void *target_fn) {


	// Write the jump at the first bytes of the function
	if (!JmpTo( target_fn, FilterFunction)) {
		return 0;
	}

	// Create the bridge
	if (!CreateGeneralBridge(BridgePointer, target_fn)) {
		return 0;
	}

	return 1;
}
