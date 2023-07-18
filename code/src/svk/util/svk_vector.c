#include "svk/util/svk_vector.h"
#include "svk/util/svk_assert.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

svkVector* svkVector_Create(u32 initialCapacity, size_t size)
{
    svkVector* svkVec = SVK_ZMSTRUCT(svkVector, 1);
    svkVec->data = calloc(initialCapacity, size);
    svkVec->size = 0;
    svkVec->cap = initialCapacity;
    svkVec->typeSize = size;

    return svkVec;
}

void
svkVector_Resize(svkVector* svkVec, size_t newSize)
{
    SVK_ASSERT(svkVec, "svkVector is NULL!");
    void* newData = (void*)realloc(svkVec->data, svkVec->typeSize * newSize);
    if (newData == NULL)
    {
        fprintf(stderr, "Failed to resize vector\n");
        return;
    }

    svkVec->data = newData;
    svkVec->cap = newSize;

    if (newSize < svkVec->size)
        svkVec->size = newSize;
}

void
svkVector_Init(svkVector* svkVec, u32 initialCapacity)
{
    SVK_ASSERT(svkVec, "svkVector is NULL!");
    svkVec->data = SVK_ZMSTRUCT(void*, initialCapacity);
    svkVec->size = 0;
    svkVec->cap = initialCapacity;
}

void
svkVector_Free(svkVector* svkVec)
{
    SVK_ASSERT(svkVec, "svkVector is NULL!");
    SVK_FREE(svkVec->data);
    SVK_FREE(svkVec);
}

void
svkVector_PushBack(svkVector* svkVec, void* pValue)
{
    SVK_ASSERT(svkVec, "svkVector is NULL!");
    if (svkVec->size >= svkVec->cap)
        svkVector_Resize(svkVec, svkVec->cap == 0 ? 2 : svkVec->cap * 2);

    svkVec->data[svkVec->size] = pValue;
    svkVec->size++;
}

void svkVector_FreeWithData(svkVector* svkVec)
{
    for (u32 i = 0; i < svkVec->size; i++)
        SVK_FREE(svkVec->data[i]);
    SVK_FREE(svkVec->data);
    SVK_FREE(svkVec);
}

void
svkVector_UpdateSize(svkVector* svkVec)
{
    SVK_ASSERT(svkVec, "svkVector is NULL!");
    u32 newSize = 0;
    for (u32 i = 0; i < svkVec->cap; i++)
    {
        if (svkVec->data[i] != NULL)
            newSize++;
        else
            break;
    }
    svkVec->size = newSize;
}

void
svkVector_PushBackCopy(svkVector* svkVec, void* pValue, size_t size)
{
    void* copy = SVK_MALLOC(size);
    memcpy(copy, pValue, size);
    svkVector_PushBack(svkVec, copy);
}

void
svkVector_PushBackString(svkVector* svkVec, void* pValue)
{
    SVK_ASSERT(svkVec, "svkVector is NULL!");
    if (svkVec->size >= svkVec->cap)
        svkVector_Resize(svkVec, svkVec->cap * 2);

    const u16 len = strlen((char*)pValue) + 1;

    char* strElement = malloc(len);
    strcpy_s(strElement, len, (char*)pValue);

    svkVec->data[svkVec->size] = strElement;
    svkVec->size++;
}

void
svkVector_Erase(svkVector* svkVec, void* value)
{
    bool isStringValue = false;
    char* stringValue = NULL;

    if (value != NULL)
    {
        stringValue = (char*)value;
        isStringValue = true;
    }

    for (u32 i = 0; i < svkVec->size; i++)
    {
        if ((isStringValue && svkVec->data[i] != NULL && strcmp((char*)svkVec->data[i], stringValue) == 0) ||
            (!isStringValue && svkVec->data[i] == value))
        {
            free(svkVec->data[i]);
            svkVec->data[i] = NULL;

            for (u32 j = i; j < svkVec->size - 1; j++)
                svkVec->data[j] = svkVec->data[j + 1];

            svkVec->size--;
            break;
        }
    }
}