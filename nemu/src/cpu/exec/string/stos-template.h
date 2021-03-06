#include "cpu/exec/template-start.h"

#define instr stos

make_helper(concat3(instr, _, SUFFIX)) {
    MEM_W(reg_l(R_EDI), reg_l(R_EAX));
    int flag = cpu.eflags.DF;
    cpu.edi += (-2 * flag + 1) * DATA_BYTE;
    print_asm(str(instr));
    return 1;
}

#include "cpu/exec/template-end.h"
