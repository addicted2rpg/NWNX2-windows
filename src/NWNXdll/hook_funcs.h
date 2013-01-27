#pragma once

int JmpTo(void *MemoryBuffer, void *JumpDestination);
int CreateStackFrame(void *MemoryBuffer);
int CreateGeneralBridge(void **BridgePointer, void *fn);
int HookFunction(void *FilterFunction, void **BridgePointer, void *target_fn);

