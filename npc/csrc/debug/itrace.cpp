#include <common.h>
#include <utils.h>
#include <debug/itrace.h>

// 定义环形缓冲区
IringbufEntry iringbuf[IRINGBUF_SIZE];
int iringbuf_index = 0;

void display_iringbuf() {
  printf("%s\n", ANSI_FMT("Instructon ring buf:", ANSI_FG_LIGHTPINK));
  int i;
  for (i = 0; i < IRINGBUF_SIZE; i++) {
    int idx = (iringbuf_index + i) % IRINGBUF_SIZE;
    if (iringbuf[idx].logbuf[0] != '\0') {
      if (i == IRINGBUF_SIZE - 1) {
        printf("%s-->%s %s\n", ANSI_FG_LIGHTPINK, iringbuf[idx].logbuf, ANSI_NONE); // 最后一条指令
      } else {
        printf("    %s\n", iringbuf[idx].logbuf);
      }
    }
  }
}
