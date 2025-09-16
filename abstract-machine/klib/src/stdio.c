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
    
    // 处理标志位（flags）和宽度（width）
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
    
    // 检查长度修饰符（l 或 ll）
    bool is_long = false;
    bool is_long_long = false;
    if (*fmt == 'l') {
      fmt++;
      if (*fmt == 'l') { // 处理 %llu
        is_long_long = true;
        fmt++;
      } else { // 处理 %lu
        is_long = true;
      }
    }
    
    // 检查是否有其他长度修饰符（如z for size_t）
    bool is_size_t = false;
    if (*fmt == 'z') {
      is_size_t = true;
      fmt++; // 跳过 'z'
    }
    
    // 处理类型说明符（specifier）
    switch (*fmt) {
      case 'c': {
        // 处理字符类型
        char c = (char)va_arg(ap, int); // 使用int接收是因为char会被提升
        
        // 处理宽度（如果指定了宽度且大于1，需要填充）
        int pad_count = width - 1;
        if (pad_count > 0) {
          char pad_char = zero_pad ? '0' : ' ';
          for (int i = 0; i < pad_count; i++) {
            *out++ = pad_char;
          }
        }
        
        // 输出字符
        *out++ = c;
        fmt++;
        break;
      }
      
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

      case 'u': {
        uint64_t num;
        // 根据长度修饰符选择数据类型
        if (is_long_long) {
          num = (uint64_t)va_arg(ap, unsigned long long);
        } else if (is_long) {
          num = (uint64_t)va_arg(ap, unsigned long);
        } else if (is_size_t) {
          num = (uint64_t)va_arg(ap, size_t);
        } else {
          num = (uint64_t)va_arg(ap, unsigned int);
        }
        
        char buffer[20]; // 足够容纳64位无符号整数的十进制表示
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
        
        // 反转输出
        while (idx > 0) {
          *out++ = buffer[--idx];
        }
        fmt++;
        break;
      }

      case 'x': {
        uint64_t num;
        // 根据长度修饰符选择数据类型
        if (is_long_long) {
          num = (uint64_t)va_arg(ap, unsigned long long);
        } else if (is_long) {
          num = (uint64_t)va_arg(ap, unsigned long);
        } else if (is_size_t) {
          num = (uint64_t)va_arg(ap, size_t);
        } else {
          num = (uint64_t)va_arg(ap, unsigned int);
        }
        
        char buffer[20]; // 足够容纳64位无符号整数的十六进制表示
        int idx = 0;
        
        if (num == 0) {
            buffer[idx++] = '0';
        } else {
            do {
                uint8_t nibble = num & 0x0F;
                buffer[idx++] = (nibble < 10) ? ('0' + nibble) : ('a' + nibble - 10);
                num >>= 4;
            } while (num > 0);
        }
        
        // 应用零填充和宽度
        int pad_count = width - idx;
        if (zero_pad && pad_count > 0) {
          for (int i = 0; i < pad_count; i++) {
            *out++ = '0';
          }
        }
        
        // 反转输出（因为我们是从低位开始存储的）
        while (idx > 0) {
          *out++ = buffer[--idx];
        }
        fmt++;
        break;
      }

      case 'p': {
        // 处理指针地址，通常以十六进制显示并添加0x前缀
        uintptr_t ptr = (uintptr_t)va_arg(ap, void *);
        
        // 指针地址至少要显示8位（32位系统）
        int min_digits = sizeof(void*) * 2; // 每个字节2个十六进制数字
        int pad_count = (width > min_digits) ? (width - min_digits) : 0;
        
        // 输出0x前缀
        *out++ = '0';
        *out++ = 'x';
        
        // 处理零填充
        if (zero_pad && pad_count > 0) {
          for (int i = 0; i < pad_count; i++) {
            *out++ = '0';
          }
        }
        
        // 转换为十六进制
        char buffer[16];
        int idx = 0;
        uintptr_t temp = ptr;
        
        // 计算需要的位数
        int digits = 0;
        if (temp == 0) digits = 1;
        else while (temp > 0) { temp >>= 4; digits++; }
        
        // 确保至少有min_digits位
        for (int i = digits; i < min_digits; i++) {
          buffer[idx++] = '0';
        }
        
        // 转换数字
        if (ptr == 0) {
          buffer[idx++] = '0';
        } else {
          while (ptr > 0) {
            uint8_t nibble = ptr & 0x0F;
            buffer[idx++] = (nibble < 10) ? ('0' + nibble) : ('a' + nibble - 10);
            ptr >>= 4;
          }
        }
        
        // 反转输出（因为我们是从低位开始存储的）
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
        if (is_size_t) {
          *out++ = 'z'; // 输出之前解析的z修饰符
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
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
