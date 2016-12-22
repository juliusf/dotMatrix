#ifndef UTIL_H
#define UTIL_H

#define TRUE 1
#define FALSE 0

#include <stdint.h>

#ifdef DEBUG
#define DEBUG_PRINT 1
#else
#define DEBUG_PRINT 0
#endif

#define debug_print(fmt, ...) \
        do { if (DEBUG_PRINT) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__); } while (0)
int32_t read_from_disk(const char* path, uint64_t* length, unsigned char* rom[]);

#endif /* UTIL_H */
