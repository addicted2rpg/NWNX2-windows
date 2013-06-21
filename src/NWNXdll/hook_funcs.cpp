#include <Windows.h>
#include <stdio.h>
#include "hook_funcs.h"

// more like fix FRAME :P
#define CREATE_FRAME TRUE
#define SAVE_PREVIOUS TRUE
#define CREATE_FRAME_FIRST TRUE


// clobber = address to clobber with a jump instruction
// destination = where you want the jump instruction to take you
// five_previous_bytes should be allocated by the caller to be 5 bytes.  They were the old bytes 
// that got 'clobbered' with the new jump instruction.
int make_jmp(unsigned char *clobber, unsigned char *destination, unsigned char *save_bytes, int savelen)
{
	unsigned long old_mask;
	int i;

	

	if (!VirtualProtect( (void *)clobber, 5, PAGE_EXECUTE_READWRITE, &old_mask))
		return 0;
	


	save_bytes[0] = *clobber;
	*clobber = (unsigned char) 0xE9;    // FYI,  0xE8 is CALL <offset> as opposed to JMP <offset>

	for(i=1; i < savelen;i++) {
		save_bytes[i] = clobber[i];
		clobber[i] = (unsigned char) 0x90;  // nop -- jump instruction will punch over most of this, unless 
		                                    // savelen > 5, then we'll want to padd with nops
	}

	*((unsigned long *) (clobber + 1)) = (unsigned long) (destination - (clobber + 1) - 4);


	if (!VirtualProtect(clobber, 5, old_mask, &old_mask))
		return 0;


	return 1;
}



/// 
int CreateStackFrame(void *MemoryBuffer, int windowslib)
{
	unsigned long OldProtection;

	unsigned char *DestinationMemory = (unsigned char *)MemoryBuffer;

	if (!VirtualProtect(MemoryBuffer, 5, PAGE_EXECUTE_READWRITE, &OldProtection))
		return 0;

	if(windowslib) {
	
		// mov edi, edi.... yep, first inst on every windows lib.  Absolutely amazing. :)
		*((unsigned long *) DestinationMemory) = 0xFF89;
		DestinationMemory += 2;

		*((unsigned long *) DestinationMemory) = 0x55;  // push ebp
		DestinationMemory++;
		*((unsigned long *) DestinationMemory) = 0xEC8B;  // mov ebp, esp
		DestinationMemory += 2;

	}
	else {

		*((unsigned long *) DestinationMemory) = 0x9090; // nop nop
		DestinationMemory += 2;
		*((unsigned long *) DestinationMemory) = 0x58;  // pop eax=0x58, (fyi, pop ebp=0x5D, push eax=0x50)
		DestinationMemory++;
		*((unsigned long *) DestinationMemory) = 0x90;  // nop 
		DestinationMemory++;


		*((unsigned long *) DestinationMemory) = 0x90; // nop
		DestinationMemory++;

	}

	
	



	
	if (!VirtualProtect(MemoryBuffer, 5, OldProtection, &OldProtection))
		return 0;
	

	return 1;
}



int CreateGeneralBridge(void **BridgePointer, void *fn, unsigned char *fill, int fill_len, int alignment, int create_stackframe)
{
	HANDLE ProcessHeap;
	unsigned long OldProtection;
	unsigned char unused[5];
	int offset;
	unsigned char *c_bridge;
	int size_desired;
	int i;

	size_desired = 5;
	if(create_stackframe) {
		size_desired += 5;
	}
	if(fill != NULL) {
		size_desired += 5;
	}


	// Create the bridge
	ProcessHeap = GetProcessHeap();
	if (!ProcessHeap)
		return 0;

	*BridgePointer = HeapAlloc(ProcessHeap, HEAP_ZERO_MEMORY, size_desired);
	if (!*BridgePointer)
		return 0;

	// Make it executable
	if (!VirtualProtect(*BridgePointer, size_desired, PAGE_EXECUTE_READWRITE, &OldProtection)) {
		return 0;
	}
	
	offset = 0;

	// Restore the stackframe code
	if(create_stackframe && CREATE_FRAME_FIRST) {
		if (!CreateStackFrame(*BridgePointer, (alignment == WINDOWS_LIBRARY) )) {
			return 0;
		}
		offset += 5;
	}

	c_bridge = (unsigned char *)*BridgePointer;
	if(fill != NULL && alignment != WINDOWS_LIBRARY) {
		for(i=0; i < fill_len;i++) {
			c_bridge[offset+i] = fill[i];
		}
		offset += fill_len;
	}

	if(create_stackframe && !CREATE_FRAME_FIRST) {
		if (!CreateStackFrame(&(c_bridge[offset]), (alignment == WINDOWS_LIBRARY))) {
			return 0;
		}
		offset += 5;
	}

	if(alignment == WINDOWS_LIBRARY) {
		if (!make_jmp(  &(c_bridge[offset]) , ((unsigned char *) fn) + 5, unused, 5)) {
			return 0;
		}
	} 
	else {
	
		if (!make_jmp(  &(c_bridge[offset]) , ((unsigned char *) fn)  + alignment + 5, unused, 5)) {
			return 0;
		}
	}

	return 1;
}




// (Developer notes: HOLY RETURN FORMULA: relative_offset = function_A_trampoline - function_A - 5 - alignment)
// FilterFunction = pointer to your hooking function, most madCHook implementations call this "HookProc"
// BridgePointer = writeable pointer (so a pointer to a pointer) leading to the trampoline that takes us back to 
//				   the original call.  Most madCHook implementations call the bridge pointer "NextHook" in their 
//                 variable names.                  
// targetfn = pointer to the function or address you are hooking.  This is annoyingly my third argument, where 
//			  it is the first in madCHook's HookCode().
// alignment = Increase the number of bytes copied to the trampoline and alter the return address on the 
// trampoline accordingly to land correctly.  See its use in NWNXDll.cpp for comments on how this works.
int HookFunction(void *FilterFunction, void **BridgePointer, void *target_fn, int alignment) {
	unsigned char *previous;
	unsigned char *targetFN_cast = (unsigned char *)target_fn;
	// unsigned char unused[5];
	// void *target_bridge;
	// HANDLE process_heap;
	// unsigned char *heap_ptr;
	// DWORD size_desired;
	// DWORD newMask, oldMask;
	int previous_length;

	if(alignment != WINDOWS_LIBRARY) {
		previous = (unsigned char *)malloc(alignment + 5);  // the jump instruction, plus how much you're aligning
		previous_length = alignment + 5;
	}
	else {
		// so make_jump will succeed, but unused in the case of WINDOWS_LIBRARY
		previous = (unsigned char *)malloc(5);
		previous_length = 5;
	}

	if (!make_jmp( targetFN_cast,  (unsigned char *)FilterFunction , 
	                previous, previous_length)) {
		return 0;
	}

	if(SAVE_PREVIOUS)
		CreateGeneralBridge(BridgePointer, target_fn, previous, previous_length, alignment, CREATE_FRAME);
	else
		CreateGeneralBridge(BridgePointer, target_fn, NULL, previous_length, alignment, CREATE_FRAME);
	

	free(previous);

	


	return 1;
}


void BridgeDump(FILE *fhandle, void *bridge_location, int alignment) {
	unsigned char *p;
	int length, i;
	int saved_bytes;
	
	
	saved_bytes = alignment + 5;

	length = 5;  // minimum length, a jump mandatory (5 bytes)
	if(CREATE_FRAME) {
		length += 5;
	}
	if(SAVE_PREVIOUS) {
		length += saved_bytes;
	}

	// hammer the above and use windows library mode
	if(alignment == WINDOWS_LIBRARY) {
		length = 10;
	}

	fprintf(fhandle, "Bridge Dump: ");

	p = (unsigned char *) bridge_location;
	for(i=0; i < length;i++) {
		fprintf(fhandle, "%X ", p[i]);
	}
	fprintf(fhandle, "\n");
	fflush(fhandle);

}

// Looks through the bridge and calculates the absolute address that the final jump will go to back
// to the original
DWORD InterpretAddress(void *supposed_callback, int alignment) {
	unsigned char *ptr;
	DWORD dbg;
	int offset;
	int previous_byte_length;


	previous_byte_length = alignment + 5;
	offset = 4;

	if(SAVE_PREVIOUS) {
		offset += previous_byte_length;
	}
	if(CREATE_FRAME) {
		offset += 5;
	}

	// hammer the above changes and go with a default lib bridge interpretation.
	if(alignment == WINDOWS_LIBRARY) {
		offset = 9;
	}

	ptr = (unsigned char *)supposed_callback;


	dbg = ptr[offset];  dbg = dbg << 8;  offset--;
	dbg += ptr[offset];  dbg = dbg << 8; offset--;
	dbg += ptr[offset];  dbg = dbg << 8; offset--;
	dbg += ptr[offset];  offset--;
	dbg += (DWORD) &(ptr[offset]);


	// The offset is interpreted AFTER the current address at EIP runs, which is 5 bytes long.
	dbg += 5;
	return dbg;
}


