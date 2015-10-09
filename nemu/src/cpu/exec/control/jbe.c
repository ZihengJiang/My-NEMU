#include "cpu/exec/helper.h"

make_helper(jbe_rel8) {
	if(cpu.eflags.CF == 1 || cpu.eflags.ZF == 1) {
		int32_t temp = instr_fetch(cpu.eip + 1, 1);
		cpu.eip += temp << 24 >> 24;
	}
	print_asm("jbe 0x%x", cpu.eip + 2);
	return 2;
}
