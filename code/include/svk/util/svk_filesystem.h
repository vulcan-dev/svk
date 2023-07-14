#ifndef SVK_FILESYSTEM_H
#define SVK_FILESYSTEM_H

#include "svk/util/svk_string.h"
#include <stdio.h>

typedef enum svkFilesystemResult
{
    FS_SUCCESS = 0,
    FS_FILENOTFOUND = 1,
    FS_READERROR = 2
} svkFilesystemResult;

// Functions
//------------------------------------------------------------------------
svkString* svkFS_FileToString(const char* filename, svkFilesystemResult* outResult);

#endif