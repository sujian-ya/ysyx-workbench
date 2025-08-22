#ifndef __DEBUG_ITRACE_H__
#define __DEBUG_ITRACE_H__

#include <common.h>

// 定义环形缓冲区大小
#define IRINGBUF_SIZE 16

// 定义环形缓冲区结构
typedef struct {
  char logbuf[128];  // 每条指令的日志信息
} IringbufEntry;

// 声明外部变量
extern IringbufEntry iringbuf[IRINGBUF_SIZE];  
extern int iringbuf_index;

// 打印 iringbuf 的内容
void display_iringbuf();

#endif
