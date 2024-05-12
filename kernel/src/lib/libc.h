#pragma once

#include <types.h>

int memcmp(const void* s1, const void* s2, usize n);
void* memcpy(void* dest, const void* src, usize n);
void* memmove(void* dest, const void* src, usize n);
void* memset(void* s, int c, usize n);
int strcmp(const char* s1, const char* s2);
int strlen(const char* s);
int toupper(char c);
char *strtok(char *src_string, const char *delim);