#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;

Context* __am_irq_handle(Context *c) {
  if (user_handler) {
    Event ev = {0};
    switch (c->mcause) {
      case 11: {
        ev.event = EVENT_YIELD;
        c->mepc += 4;
        break;
      }
      default: ev.event = EVENT_ERROR; break;
    }

    c = user_handler(ev, c);
    assert(c != NULL);
  }
  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context*(*handler)(Event, Context*)) {
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

  // register event handler
  user_handler = handler;

  return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  // 从栈的底部开始创建上下文
  Context *ctx = (Context *)(kstack.end - sizeof(Context));
  
  // 清零上下文结构
  memset(ctx, 0, sizeof(Context));

  // 初始化上下文寄存器
  memset(ctx, 0, sizeof(Context));
  ctx->mepc = (uintptr_t)entry;
  ctx->mstatus = 0x00001800; // 设置MIE位以启用中断
  ctx->gpr[10] = (uintptr_t)arg; // a0寄存器传递参数

  return ctx;

}

void yield() {
#ifdef __riscv_e
  asm volatile("li a5, -1; ecall");
#else
  asm volatile("li a7, -1; ecall");
#endif
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
