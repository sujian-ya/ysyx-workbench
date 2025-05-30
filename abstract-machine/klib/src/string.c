#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  const char *p = s;
  while (*p != '\0') {
    p++;
  }
  return p - s;
  panic("Not implemented");
}

char *strcpy(char *dst, const char *src) {
  char *d = dst;
  while ((*d++ = *src++) != '\0');
  return dst;
  panic("Not implemented");
}

char *strncpy(char *dst, const char *src, size_t n) {
  char *d = dst;
  while (n-- > 0) {
    *d = *src;
    if (*src != '\0') {
      src++;
    }
    d++;
  }
  return dst;
  panic("Not implemented");
}

char *strcat(char *dst, const char *src) {
  char *d = dst;
  while (*d != '\0') {
    d++;
  }
  while ((*d++ = *src++) != '\0');
  return dst;
  panic("Not implemented");
}

int strcmp(const char *s1, const char *s2) {
  while (*s1 && *s2 && *s1 == *s2) {
    s1++;
    s2++;
  }
  return *(const unsigned char *)s1 - *(const unsigned char *)s2;
  panic("Not implemented");
}

int strncmp(const char *s1, const char *s2, size_t n) {
  while (n-- > 0) {
    if (*s1 != *s2) {
      return *(const unsigned char *)s1 - *(const unsigned char *)s2;
    }
    if (*s1 == '\0') {
      break;
    }
    s1++;
    s2++;
  }
  return 0;
  panic("Not implemented");
}

void *memset(void *s, int c, size_t n) {
  unsigned char *p = s;
  while (n-- > 0) {
    *p++ = (unsigned char)c;
  }
  return s;
  panic("Not implemented");
}

void *memmove(void *dst, const void *src, size_t n) {
  unsigned char *d = dst;
  const unsigned char *s = src;
  
  if (d < s) {
    while (n-- > 0) {
      *d++ = *s++;
    }
  } else if (d > s) {
    d += n;
    s += n;
    while (n-- > 0) {
      *--d = *--s;
    }
  }
  return dst;
  panic("Not implemented");
}

void *memcpy(void *out, const void *in, size_t n) {
  unsigned char *d = out;
  const unsigned char *s = in;
  while (n-- > 0) {
    *d++ = *s++;
  }
  return out;
  panic("Not implemented");
}

int memcmp(const void *s1, const void *s2, size_t n) {
  const unsigned char *p1 = s1;
  const unsigned char *p2 = s2;
  while (n-- > 0) {
    if (*p1 != *p2) {
      return *p1 - *p2;
    }
    p1++;
    p2++;
  }
  return 0;
  panic("Not implemented");
}

#endif