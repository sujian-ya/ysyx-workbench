#ifndef __ISA_ISA_H__
#define __ISA_ISA_H__

#include <common.h>
#include <isa-def.h>

typedef concat(__GUEST_ISA__, _CPU_state) CPU_state;

// reg
extern CPU_state cpu;
void npc_reg_display();

// interrupt/exception
extern "C" {
    vaddr_t isa_raise_intr(word_t NO, vaddr_t epc);
    word_t isa_ret_intr();
}
// #define INTR_EMPTY ((word_t)-1)
// word_t isa_query_intr();

#endif
