#include <common.h>
#include <utils.h>
#include <debug/itrace.h>

// 定义环形缓冲区
IringbufEntry iringbuf[IRINGBUF_SIZE];
int iringbuf_index = 0;

void display_iringbuf() {
#ifdef CONFIG_ITRACE
  // 先检查环形缓冲区中是否有有效日志（logbuf不为空）
  int valid_count = 0;
  for (int i = 0; i < IRINGBUF_SIZE; i++) {
    int idx = (iringbuf_index + i) % IRINGBUF_SIZE;
    if (iringbuf[idx].logbuf[0] != '\0') { // 非空字符串即为有效日志
      valid_count++;
    }
  }

  // 仅当存在有效日志时，才打印标题和内容
  if (valid_count > 0) {
    printf("\nInstructon ring buffer:\n"); // 仅此时打印标题
    for (int i = 0; i < IRINGBUF_SIZE; i++) {
      int idx = (iringbuf_index + i) % IRINGBUF_SIZE;
      if (iringbuf[idx].logbuf[0] != '\0') {
        if (i == IRINGBUF_SIZE - 1) {
          printf("--> %s\n", iringbuf[idx].logbuf); // 最后一条指令
        } else {
          printf("    %s\n", iringbuf[idx].logbuf);
        }
      }
    }
  }
  // 若valid_count=0：直接不打印任何内容，退出函数
#endif
}
