#include <am.h>
#include <klib.h>
#include <klib-macros.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
static unsigned long int next = 1;

int rand(void) {
  // RAND_MAX assumed to be 32767
  next = next * 1103515245 + 12345;
  return (unsigned int)(next/65536) % 32768;
}

void srand(unsigned int seed) {
  next = seed;
}

int abs(int x) {
  return (x < 0 ? -x : x);
}

int atoi(const char* nptr) {
  int x = 0;
  while (*nptr == ' ') { nptr ++; }
  while (*nptr >= '0' && *nptr <= '9') {
    x = x * 10 + *nptr - '0';
    nptr ++;
  }
  return x;
}

void *malloc(size_t size) {
  // On native, malloc() will be called during initializaion of C runtime.
  // Therefore do not call panic() here, else it will yield a dead recursion:
  //   panic() -> putchar() -> (glibc) -> malloc() -> panic()
#if !(defined(__ISA_NATIVE__) && defined(__NATIVE_USE_KLIB__))
  static char *current = NULL;
  static bool initialized = false;

  // 初始化堆区起始地址，并对齐到 8 字节
  if (!initialized) {
    current = (char *)ROUNDUP((uintptr_t)heap.start, 8);
    initialized = true;
  }

  // 对请求的内存大小进行 8 字节对齐
  size = (size + 7) & ~7;

  // 检查是否超出堆区范围
  if (current + size > (char *)heap.end) {
    printf("malloc failed: requested size = %zu, current = %p, heap.end = %p\n", size, current, heap.end);
    return NULL;
  }

  // 分配内存并更新指针
  void *ret = current;
  current += size;
  return ret;
#endif
  return NULL;
}

void free(void *ptr) {
}

#endif
