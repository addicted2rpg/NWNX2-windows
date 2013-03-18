#pragma once

#define WINDOWS_LIBRARY 99

int make_jmp(unsigned char *clobber, unsigned char *destination, unsigned char *save_bytes, int savelen);

int CreateStackFrame(void *MemoryBuffer, int windowslib);
int CreateGeneralBridge(void **BridgePointer, void *fn, unsigned char *fill, int fill_len, int alignment, int create_stackframe);

int HookFunction(void *FilterFunction, void **BridgePointer, void *target_fn, int alignment);

// These are for debugging my own damn API, lol... :) 
DWORD InterpretAddress(void *supposed_callback, int alignment);
void BridgeDump(FILE *fhandle, void *bridge_location, int alignment);
