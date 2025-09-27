#include <am.h>
#include <npc.h>

#define KEYDOWN_MASK 0x8000

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  // kbd->keydown = 0;
  // kbd->keycode = AM_KEY_NONE;
  uint32_t keyboard = inl(KBD_ADDR);
  // 最高位获取 keydown 状态，低位获取 keycode
  kbd->keydown = (keyboard & KEYDOWN_MASK) ? true : false;
  kbd->keycode = keyboard & (~KEYDOWN_MASK);
}
