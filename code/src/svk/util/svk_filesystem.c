#define _CRT_SECURE_NO_WARNINGS

#include "svk/util/svk_filesystem.h"
#include "svk/svk.h"

svkString* svkFS_FileToString(const char* filename, svkFilesystemResult* outResult)
{
    *outResult = FS_SUCCESS;
    svkString* str = SVK_ALLOCSTRUCT(svkString, 1);

    char* buf = NULL;
    uint32_t stringSize = 0;
    uint32_t readSize = 0;

    FILE* f = fopen(filename, "rb");
    if (f == NULL)
    {
        *outResult = FS_FILENOTFOUND;
        return NULL;
    }
    
    fseek(f, 0, SEEK_END);
    stringSize = ftell(f);
    rewind(f);

    buf = (char*)malloc(sizeof(char) * (stringSize + 1));
    readSize = fread(buf, sizeof(char), stringSize, f);

    buf[stringSize] = '\0';
    if (stringSize != readSize)
    {
        free(buf);
        buf = NULL;
        fclose(f);
        *outResult = FS_READERROR;
        return NULL;
    }

    fclose(f);
    str->data = buf;
    str->len = stringSize;

    return str;
}