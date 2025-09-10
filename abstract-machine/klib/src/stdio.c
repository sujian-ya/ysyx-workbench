#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int vsprintf(char *out, const char *fmt, va_list ap) {
  char *start = out;  // 记录起始位置
  while (*fmt) {
    if (*fmt != '%') {
      // 普通字符直接复制
      *out++ = *fmt++;
      continue;
    }
    
    // 处理格式说明符
    fmt++;  // 跳过 '%'
    switch (*fmt) {
      case 's': {
        // 字符串参数
        const char *s = va_arg(ap, const char *);
        while (*s) {
          *out++ = *s++;
        }
        fmt++;
        break;
      }
      
      case 'd': {
        // 整数参数
        int num = va_arg(ap, int);
        if (num < 0) {
          *out++ = '-';
          num = -num;  // 转换为正数处理
        }
        
        // 将整数转换为字符串（反向存储）
        char buffer[16];
        int idx = 0;
        do {
          buffer[idx++] = '0' + (num % 10);
          num /= 10;
        } while (num > 0);
        
        // 反向复制到输出
        while (idx > 0) {
          *out++ = buffer[--idx];
        }
        fmt++;
        break;
      }
      
      case '%':
        // 转义百分号
        *out++ = '%';
        fmt++;
        break;
      
      default:
        // 不支持格式：原样输出'%'和字符
        *out++ = '%';
        *out++ = *fmt++;
        break;
    }
  }
  
  *out = '\0';  // 终止字符串
  return out - start;  // 返回写入字符数（不含终止符）
//   panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int n = vsprintf(out, fmt, ap);
  va_end(ap);
  return n;
//   panic("Not implemented");
}

int printf(const char *fmt, ...) {
  char buf[256]; 
  va_list ap;
  
  va_start(ap, fmt);
  int len = vsprintf(buf, fmt, ap); // 调用 vsprintf 将结果写入 buf
  va_end(ap);
  
  // 遍历缓冲区，用 putch() 逐个字符打印
  for (int i = 0; i < len; i++) {
    putch(buf[i]);
  }
  
  return len;
//   panic("Not implemented");
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
