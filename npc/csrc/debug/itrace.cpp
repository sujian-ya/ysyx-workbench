#include <common.h>
#include <debug/itrace.h>

// 定义环形缓冲区
IringbufEntry iringbuf[IRINGBUF_SIZE];
int iringbuf_index = 0;

void display_iringbuf() {
  printf("\nInstructon ring buffer:\n");
  int i;
  for (i = 0; i < IRINGBUF_SIZE; i++) {
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
