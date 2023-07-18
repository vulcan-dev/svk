#ifndef SVK_COMMON_H
#define SVK_COMMON_H

#include <string.h> // for memset
#include <assert.h>
#include <stdlib.h>
#include "svk/util/svk_assert.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;
typedef unsigned long long u64;

typedef unsigned char bool;
#define true 1
#define false 0

#define internal static
#define persist  static
#define local    static

#define SVK_FREE(x) free(x)
#define SVK_MALLOC(size) malloc(size)
#define SVK_REALLOC(block, size) realloc(block, size)
#define SVK_MEMSET(dest, val, size) memset(&dest, 0, size)
#define SVK_ZM(dest, size) memset(&dest, 0, size)
#define SVK_ZMA(dest) SVK_ZM(dest, sizeof(dest))

#define SVK_CHECK_VKRESULT(result, msg, ...) \
    do { \
        SVK_ASSERT(result == VK_SUCCESS, msg, __VA_ARGS__) \
    } while (0)
    
#define SVK_ARRAY_SIZE(x) (sizeof((x)) / sizeof((x)[0]))

#define SVK_ALLOCSTRUCT(type, count) \
    (type*)SVK_MALLOC(sizeof(type) * (count))

#define SVK_ZMSTRUCT(type, count) \
    (type*)memset(SVK_ALLOCSTRUCT(type, count), 0, sizeof(type) * (count))

#define SVK_ZMSTRUCT2(structType) ((structType) {0})

#define SVK_ALLOCSTRUCT2(type) \
    ((type*)SVK_MALLOC(sizeof(type)))

#endif