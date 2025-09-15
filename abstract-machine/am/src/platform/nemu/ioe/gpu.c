#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
  uint32_t vga_ctl = inl(VGACTL_ADDR);
  // 宽度和高度分别存储在高16位和低16位
  int w = (vga_ctl >> 16) & 0xffff;
  int h = vga_ctl & 0xffff;
  // 定义一个指向显存起始地址的指针，并将其初始化为FB_ADDR
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  for (int i = 0; i < w * h; i++) {
    fb[i] = i;
  }
  outl(SYNC_ADDR, 1);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  // *cfg = (AM_GPU_CONFIG_T) {
  //   .present = true, .has_accel = false,
  //   .width = 0, .height = 0,
  //   .vmemsz = 0
  // };
  // 再次从VGA控制寄存器读取屏幕宽度和高度，以及计算显存大小
  uint32_t vga_ctl = inl(VGACTL_ADDR);
  int w = (vga_ctl >> 16) & 0xffff;
  int h = vga_ctl & 0xffff;
  size_t vmem_size = w * h * sizeof(uint32_t);

  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = w, .height = h,
    .vmemsz = vmem_size
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  uint32_t vga_ctl = inl(VGACTL_ADDR);
  // 宽度和高度分别存储在高16位和低16位
  int screen_w = (vga_ctl >> 16) & 0xffff;
  int screen_h = vga_ctl & 0xffff;
  // 获取帧缓冲区和像素数据的指针
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  uint32_t *pixels = (uint32_t*)ctl->pixels;
  int draw_x = ctl->x, draw_y = ctl->y, draw_w = ctl->w, draw_h = ctl->h;

  // 优化边界检查，避免在循环内部频繁判断
  int start_x = (draw_x < 0) ? 0 : draw_x;
  int end_x = (draw_x + draw_w > screen_w) ? screen_w : draw_x + draw_w;
  int start_y = (draw_y < 0) ? 0 : draw_y;
  int end_y = (draw_y + draw_h > screen_h) ? screen_h : draw_y + draw_h;

  for (int j = start_y; j < end_y; j++) {
    for (int i = start_x; i < end_x; i++) {
      // 计算源像素的相对位置
      int src_x = i - draw_x;
      int src_y = j - draw_y;
      
      fb[j * screen_w + i] = pixels[src_y * draw_w + src_x];
    }
  }

  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
