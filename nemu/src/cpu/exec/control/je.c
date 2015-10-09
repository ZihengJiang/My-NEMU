#include "cpu/exec/helper.h"

make_helper(je_rel8) {
	if(cpu.eflags.ZF == 1) {
		cpu.eip += instr_fetch(cpu.eip + 1, 1);
	}
	return 2;
}