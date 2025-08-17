#ifndef __UTILS_H__
#define __UTILS_H__

#include <common.h>

// ----------- state -----------

enum { NPC_RUNNING, NPC_STOP, NPC_END, NPC_ABORT, NPC_QUIT };

typedef struct {
  int state;
  vaddr_t halt_pc;
  uint32_t halt_ret;
} NPCState;

extern NPCState npc_state;

// ----------- timer -----------

uint64_t get_time();

// ----------- log -----------

#define ANSI_FG_BLACK   "\33[1;30m"
#define ANSI_FG_RED     "\33[1;31m"
#define ANSI_FG_GREEN   "\33[1;32m"
#define ANSI_FG_YELLOW  "\33[1;33m"
#define ANSI_FG_BLUE    "\33[1;34m"
#define ANSI_FG_MAGENTA "\33[1;35m"
#define ANSI_FG_CYAN    "\33[1;36m"
#define ANSI_FG_WHITE   "\33[1;37m"
#define ANSI_BG_BLACK   "\33[1;40m"
#define ANSI_BG_RED     "\33[1;41m"
#define ANSI_BG_GREEN   "\33[1;42m"
#define ANSI_BG_YELLOW  "\33[1;43m"
#define ANSI_BG_BLUE    "\33[1;44m"
#define ANSI_BG_MAGENTA "\33[1;45m"
#define ANSI_BG_CYAN    "\33[1;46m"
#define ANSI_BG_WHITE   "\33[1;47m"
#define ANSI_NONE       "\33[0m"

// 柔和的粉色系
#define ANSI_FG_LIGHTPINK     "\33[38;5;217m"  // 浅粉（编号217）
#define ANSI_FG_PASTELPINK    "\33[38;5;225m"  // 粉白（编号225）
#define ANSI_BG_LIGHTPINK      "\33[48;5;217m"  // 浅粉背景色

// 清新的蓝绿系
#define ANSI_FG_SKY_BLUE       "\33[38;5;117m"  // 天蓝（编号117）
#define ANSI_FG_TEAL           "\33[38;5;49m"   // 青蓝（编号49）
#define ANSI_FG_SEA_GREEN      "\33[38;5;64m"   // 海绿（编号64）
#define ANSI_BG_SUNSETORANGE   "\33[48;5;214m"  // 橙黄背景色

// 温暖的橙黄系
#define ANSI_FG_APRICOT        "\33[38;5;223m"  // 杏色（编号223）
#define ANSI_FG_SUNSET_ORANGE  "\33[38;5;208m"  // 日落橙（编号208）

// 浅紫色文本
#define ANSI_FG_LAVENDER      "\33[38;5;183m"  // 淡薰衣草紫（柔和）
#define ANSI_FG_LILAC         "\33[38;5;182m"  // 丁香紫（偏粉调）
#define ANSI_FG_PALE_VIOLET   "\33[38;5;147m"  // 浅紫罗兰（明亮）

// 浅紫色背景
#define ANSI_BG_LAVENDER      "\33[48;5;225m"  // 淡薰衣草紫背景
#define ANSI_BG_LILAC_BG      "\33[48;5;224m"  // 丁香紫背景（更浅）
#define ANSI_BG_PALE_VIOLET_BG "\33[48;5;147m" // 浅紫罗兰背景（稍深）

#define ANSI_FMT(str, fmt) fmt str ANSI_NONE

// #define log_write(...) IFDEF(CONFIG_TARGET_NATIVE_ELF, \
//   do { \
//     extern FILE* log_fp; \
//     extern bool log_enable(); \
//     if (log_enable() && log_fp != NULL) { \
//       fprintf(log_fp, __VA_ARGS__); \
//       fflush(log_fp); \
//     } \
//   } while (0) \
// )

// #define _Log(...) \
//   do { \
//     printf(__VA_ARGS__); \
//     log_write(__VA_ARGS__); \
//   } while (0)

#define _Log(...) \
  do { \
    printf(__VA_ARGS__); \
  } while (0)


#endif
