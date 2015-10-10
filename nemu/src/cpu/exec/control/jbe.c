#include "cpu/exec/helper.h"

make_helper(jbe_rel8) {
	int32_t temp = instr_fetch(cpu.eip + 1, 1);
	temp = temp << 24 >> 24;
	if(cpu.eflags.CF == 1 || cpu.eflags.ZF == 1) {
		cpu.eip += temp;
		print_asm("jbe 0x%x", cpu.eip + 2);
	}
	else
		print_asm("jbe 0x%x", cpu.eip + temp + 2);
	return 2;
}
