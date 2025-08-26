#ifndef __ISA_ISA_H__
#define __ISA_ISA_H__

#include <common.h>
#include <isa-def.h>

typedef concat(__GUEST_ISA__, _CPU_state) CPU_state;

// reg
extern CPU_state cpu;
void npc_reg_display();

#endif
