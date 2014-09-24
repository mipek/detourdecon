#include <ADE32.HPP>
#include "disasm.h"
#include "stddef.h"
#include "assert.h"

CDisasm g_disasm;

int CDisasm::GetInstructionLength(byte *code) const
{
    disasm_struct diza = {0};
	int len = ade32_disasm(code, &diza);
	assert(len > 0);

    return len;
}

CDisasm *GetDisassembler()
{
	return &g_disasm;
}