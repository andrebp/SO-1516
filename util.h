#ifndef UTIL_HEADER
#define UTIL_HEADER

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

#define size 1024

ssize_t readln(int fildes, void *buf, size_t nbyte);


#endif
