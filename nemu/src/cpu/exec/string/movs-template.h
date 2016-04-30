#include "cpu/exec/template-start.h"

#define instr movs

make_helper(concat3(instr, _, SUFFIX)) {
    Log("copy from 0x%x to 0x%x: %u", REG(R_ESI), REG(R_EDI), MEM_R(REG(R_ESI)));
    MEM_W(REG(R_EDI), MEM_R(REG(R_ESI)));
    int flag = cpu.eflags.DF;
    cpu.edi += (-2 * flag + 1) * DATA_BYTE;
    cpu.esi += (-2 * flag + 1) * DATA_BYTE;
    print_asm(str(instr));
    return 1;
}

#include "cpu/exec/template-end.h"
