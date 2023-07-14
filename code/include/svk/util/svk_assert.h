#ifndef SVK_ASSERT_H
#define SVK_ASSERT_H

void svkAssert_Exit(const char* file, int line, const char* message);

#define SVK_ASSERT(cond, msg) \
    if (cond) { \
        svkAssert_Exit(__FILE__, __LINE__, msg); \
    }

#endif