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

	*((unsigned long *) (clobber + 1)) = (unsigned long) (destination - clobber - 4);


	if (!VirtualProtect(clobber, 5, old_mask, &old_mask))
		return 0;


	return 1;
}



/// 
int CreateStackFrame(void *MemoryBuffer)
{
	unsigned long OldProtection;

	unsigned char *DestinationMemory = (unsigned char *)MemoryBuffer;

	if (!VirtualProtect(MemoryBuffer, 5, PAGE_EXECUTE_READWRITE, &OldProtection))
		return 0;

	// mov edi, edi.... a Windows thing?
	// *((unsigned long *) DestinationMemory) = 0xFF89;

	*((unsigned long *) DestinationMemory) = 0x9090;
	DestinationMemory += 2;

	// *((unsigned long *) DestinationMemory) = 0x55;  // push ebp
	// *((unsigned long *) DestinationMemory) = 0xEC8B;  // mov ebp, esp
	*((unsigned long *) DestinationMemory) = 0x58;  // pop eax=0x58
	DestinationMemory++;


	*((unsigned long *) DestinationMemory) = 0x90;  // pop ebp=0x5D
	DestinationMemory++;


	*((unsigned long *) DestinationMemory) = 0x90;  // push eax=0x50
	DestinationMemory += 2;

	
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
		if (!CreateStackFrame(*BridgePointer)) {
			return 0;
		}
		offset += 5;
	}

	c_bridge = (unsigned char *)*BridgePointer;
	if(fill != NULL) {
		for(i=0; i < fill_len;i++) {
			c_bridge[offset+i] = fill[i];
		}
		offset += fill_len;
	}

	if(create_stackframe && !CREATE_FRAME_FIRST) {
		if (!CreateStackFrame(&(c_bridge[offset]))) {
			return 0;
		}
		offset += 5;
	}

	
	// my only concern is there could be an alignment issue here dependent on the instruction that follows
	// where the initial clobber address was.
	if (!make_jmp(  &(c_bridge[offset]) , ((unsigned char *) fn)  + alignment + 4, unused, 5))
		return 0;

	return 1;
}




// HOLY RETURN FORMULA: relative_offset = (function_A_trampoline + 6) - (function_A + 6) - 5;

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

	previous = (unsigned char *)malloc(alignment + 5);  // the jump instruction, plus how much you're aligning
	previous_length = alignment + 5;

	if (!make_jmp( targetFN_cast,  (unsigned char *)FilterFunction , 
	                previous, previous_length)) {
		return 0;
	}

	if(SAVE_PREVIOUS)
		CreateGeneralBridge(BridgePointer, target_fn, previous, previous_length, alignment, CREATE_FRAME);
	else
		CreateGeneralBridge(BridgePointer, target_fn, NULL, previous_length, alignment, CREATE_FRAME);
	

	free(previous);

/* 


	size_desired = 5;  // for the JMP to get out

	if(save_previous)
		size_desired += 5;  // for the previous
	if(create_frame)
		size_desired += 5;  // for the stack frame

	// make size_desired bytes pointed to by heap_ptr allocated and executable.
	process_heap = GetProcessHeap();
	heap_ptr = (unsigned char *)HeapAlloc(process_heap, HEAP_ZERO_MEMORY, size_desired);
	if(heap_ptr == NULL) {
		return 0;
	}
	if(!VirtualProtect((void *)heap_ptr, size_desired, PAGE_EXECUTE_READWRITE, &oldMask)) {
		return 0;
	}

	//make_jmp(targetFN_cast, heap_ptr, previous);
	make_jmp(targetFN_cast, (unsigned char *)FilterFunction, previous); // going to wreck my work...
	
	if(create_frame) {
		CreateStackFrame((void *)heap_ptr);
		heap_ptr = heap_ptr + 5;  // forward past the stack frame we just made
	}

	if(save_previous) {
		heap_ptr[0] = previous[0];
		heap_ptr[1] = previous[1];
		heap_ptr[2] = previous[2];
		heap_ptr[3] = previous[3];
		heap_ptr[4] = previous[4];
		heap_ptr = heap_ptr + 5;   // For writing to jump filter
	}


	make_jmp(heap_ptr, (unsigned char *)FilterFunction, unused);

	if(save_previous) 
		heap_ptr = heap_ptr - 5;

	if(create_frame)
		heap_ptr = heap_ptr - 5;  //back to the original from stackframe

	newMask = oldMask;
	VirtualProtect((void *)heap_ptr, size_desired, newMask, &oldMask);

	//////  Ok, now let's worry about the journey home.
	size_desired = 5;
	// process_heap = GetProcessHeap(); 
	heap_ptr = (unsigned char *)HeapAlloc(process_heap, HEAP_ZERO_MEMORY, size_desired);
	VirtualProtect((void *)heap_ptr, size_desired, PAGE_EXECUTE_READWRITE, &oldMask);

	make_jmp(heap_ptr, targetFN_cast + 5 + alignment, unused);

	newMask = oldMask;
	VirtualProtect((void *)heap_ptr, size_desired, newMask, &oldMask);

	*/

	


	return 1;
}


void BridgeDump(FILE *fhandle, void *bridge_location, int alignment) {
	unsigned char *p;
	int length, i;
	int saved_bytes = alignment + 5;

	length = 5;  // minimum length, a jump mandatory (5 bytes)
	if(CREATE_FRAME) {
		length += 5;
	}
	if(SAVE_PREVIOUS) {
		length += saved_bytes;
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
	int previous_byte_length = alignment + 5;

	offset = 4;

	if(SAVE_PREVIOUS) {
		offset += previous_byte_length;
	}
	if(CREATE_FRAME) {
		offset += 5;
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


