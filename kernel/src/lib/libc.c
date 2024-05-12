#include <lib/libc.h>

int memcmp(const void *s1, const void *s2, size_t n) {
  const u8 *p1 = (const u8 *)s1;
  const u8 *p2 = (const u8 *)s2;
  for (size_t i = 0; i < n; i++) {
    if (p1[i] != p2[i]) {
      return p1[i] < p2[i] ? -1 : 1;
    }
  }

  return 0;
}

void *memcpy(void *d, const void *s, size_t n) {
  __asm__ volatile ("rep movsb"
                : "=D" (d),
                  "=S" (s),
                  "=c" (n)
                : "0" (d),
                  "1" (s),
                  "2" (n)
                : "memory");
  return d;
}

void *memmove(void *dest, const void *src, size_t n) {
  u8 *pdest = (u8 *)dest;
  const u8 *psrc = (const u8 *)src;

  if (src > dest) {
    for (size_t i = 0; i < n; i++) {
      pdest[i] = psrc[i];
    }
  } else if (src < dest) {
    for (size_t i = n; i > 0; i--) {
      pdest[i-1] = psrc[i-1];
    }
  }

  return dest;
}

void *memset(void *s, int c, size_t n) {
  u8 *p = (u8 *)s;

  for (size_t i = 0; i < n; i++) {
    p[i] = (u8)c;
  }

  return s;
}

int strcmp(const char* str, const char* str2) {
  if (strlen(str) != strlen(str2))
    return 1;
  
  for (int i = 0; i < strlen(str); i++)
    if (str[i] != str2[i])
      return 1;
    
  return 0;
}

int strlen(const char* str) {
  int i = 0;
  while (*str != '\0') {
    i++;
    str++;
  }
  return i;
}

int toupper(char c) {
  if (c >= 'a' && c <= 'z') return c-0x20;
  return c;
}

char* strtok(char* str, const char* delim) {
  static char* p = NULL;
  if (str != NULL) { p = str; }
  else if (p == NULL) { return NULL; }

  char* start = p;
  while (*p != '\0')  {
    const char* d = delim;
    while (*d != '\0') {
      if (*p == *d) {
        *p = '\0';
        p++;
        if (start == p) {
          start = p;
          continue;
        }
        return start;
      }
      d++;
    }
    p++;
  }
  if (start == p) { return NULL; }
  return start;
}