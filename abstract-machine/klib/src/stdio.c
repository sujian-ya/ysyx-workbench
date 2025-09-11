#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int vsprintf(char *out, const char *fmt, va_list ap) {
  char *start = out;
  while (*fmt) {
    if (*fmt != '%') {
      *out++ = *fmt++;
      continue;
    }
    
    fmt++; // 跳过 '%'
    
    // 1. 处理标志位（flags）和宽度（width）
    bool zero_pad = false;
    int width = 0;
    if (*fmt == '0') {
      zero_pad = true;
      fmt++;
    }
    while (*fmt >= '0' && *fmt <= '9') {
      width = width * 10 + (*fmt - '0');
      fmt++;
    }
    
    // 2. 处理类型说明符（specifier）
    switch (*fmt) {
      case 's': {
        const char *s = va_arg(ap, const char *);
        while (*s) {
          *out++ = *s++;
        }
        fmt++;
        break;
      }
      
      case 'd': {
        int num = va_arg(ap, int);
        if (num < 0) {
          *out++ = '-';
          num = -num;
        }
        
        char buffer[16];
        int idx = 0;
        if (num == 0) {
            buffer[idx++] = '0';
        } else {
            do {
                buffer[idx++] = '0' + (num % 10);
                num /= 10;
            } while (num > 0);
        }
        
        // 应用零填充和宽度
        int pad_count = width - idx;
        if (zero_pad && pad_count > 0) {
          for (int i = 0; i < pad_count; i++) {
            *out++ = '0';
          }
        }
        
        while (idx > 0) {
          *out++ = buffer[--idx];
        }
        fmt++;
        break;
      }
      
      case '%':
        *out++ = '%';
        fmt++;
        break;
      
      default:
        // 不支持的格式：原样输出
        *out++ = '%';
        if (width > 0) {
          // 如果有宽度，也打印出来
          int i = 0;
          char temp_width_buf[4];
          if (width == 0) {
            temp_width_buf[i++] = '0';
          } else {
            int temp_w = width;
            while(temp_w > 0) {
              temp_width_buf[i++] = (temp_w % 10) + '0';
              temp_w /= 10;
            }
          }
          while(i > 0) {
            *out++ = temp_width_buf[--i];
          }
        }
        *out++ = *fmt++;
        break;
    }
  }
  *out = '\0';
  return out - start;
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
