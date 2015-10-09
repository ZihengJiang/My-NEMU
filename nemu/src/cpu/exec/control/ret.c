#include "cpu/exec/helper.h"

make_helper(ret) {
	// EIP = Pop()
	// 
	// Pop:
	// DEST = (SS:ESP); (* copy a dword *)
	// ESP = ESP + 4;
	
	cpu.eip = swaddr_read(cpu.esp, 4);
	cpu.esp += 4;
	print_asm("ret");
	return 1;
}
