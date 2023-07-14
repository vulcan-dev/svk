#include "svk/util/svk_assert.h"
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

void svkAssert_Exit(const char* file, int line, const char* message)
{
    fprintf(stderr, "Error at %s:%d - %s\n", file, line, message);

    char errorMsg[256];
    sprintf(errorMsg, "Error at %s:%d\n\n%s", file, line, message);

    MessageBoxA(NULL, errorMsg, "Assertion Failed", MB_ICONERROR | MB_OK);
    exit(EXIT_FAILURE);
}