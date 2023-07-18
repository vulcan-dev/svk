#include "svk/util/svk_logger.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

// Globals
//------------------------------------------------------------------------
local const char* levelColors[] = {
    "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[35m"
};

local const char* levelNames[] = {
    "DEBUG", "INFO", "WARNING", "ERROR"
};

// Functions
//------------------------------------------------------------------------
void SVK_Log(const u8 level, const char* file, u16 line, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    time_t rawTime;
    struct tm timeInfo;
    char timeBuffer[80];
    time(&rawTime);
    localtime_s(&timeInfo, &rawTime);
    strftime(timeBuffer, sizeof(timeBuffer), "%H:%M:%S", &timeInfo);

    const char* color = levelColors[level];
    const char* levelName = levelNames[level];

    char filename[MAX_PATH];
    if (PathFindFileNameA(file) != NULL)
        strncpy_s(filename, sizeof(filename), PathFindFileNameA(file), _TRUNCATE);
    else
        strncpy_s(filename, sizeof(filename), file, _TRUNCATE);

    printf("\x1b[37m%s %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m ", timeBuffer, color, levelNames[level], filename, line);
    vprintf(fmt, args);
    printf("\x1b[0m\n");

    va_end(args);
}