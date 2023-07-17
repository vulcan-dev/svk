#ifndef SVK_VECTOR_H
#define SVK_VECTOR_H

#include "svk/svk_common.h"

typedef struct svkVector
{
    void** data;
    u32 cap;
    u32 size;
    size_t typeSize;
} svkVector;

svkVector* svkVector_Create(u32 initialCapacity, size_t size);
void svkVector_Init(svkVector* svkVec, u32 initialCapacity);
void svkVector_Free(svkVector* svkVec);
void svkVector_FreeWithData(svkVector* svkVec);
void svkVector_Resize(svkVector* svkVec, size_t newSize);
//void svkVector_Resize(svkVector* svkVec, u32 newSize);
void svkVector_UpdateSize(svkVector* svkVec);
void svkVector_PushBack(svkVector* svkVec, void* pValue);
void svkVector_PushBackCopy(svkVector* svkVec, void* pValue, size_t size);
void svkVector_PushBackString(svkVector* svkVec, void* pValue);
void svkVector_Erase(svkVector* svkVec, void* value);

#define SVKVECTOR_TYPE(type) svkVector*
#define SVKARRAY_TYPE(type) type*
#define SVKVECTOR_CREATE(cap, type) svkVector_Create(cap, sizeof(type))
#define SVKVECTOR_PUSHBACK(v,x) \
    _Generic((x), \
    char*: (svkVector_PushBackString), \
    const char*: (svkVector_PushBackString), \
    default: (svkVector_PushBack) \
)(v, (void*)(x))

#define SVKVECTOR_ERASE(v,x) svkVector_Erase(v, (void*)x);
#define SVKVECTOR_CONVERT(vec, type)                             \
    ({                                                           \
        type* convertedArray = SVK_ZMSTRUCT(type, vec->size);    \
        for (u32 i = 0; i < vec->size; i++) {                    \
            convertedArray[i] = *((type*)vec->data[i]);          \
        }                                                        \
        convertedArray;                                          \
    }) \

#define SVKVECTOR_COPY_ARR_TO(dest, src, len, tsize)     \
    do {                                                 \
        void* copy = SVK_MALLOC(tsize * len);            \
        for (u32 i = 0; i < len; i++) {                  \
            memcpy(copy, &src[i], len);                  \
            (*(dest))->data[i] = copy;                   \
            (*(dest))->size++;                           \
        }                                                \
    } while (0)                                          \

#endif