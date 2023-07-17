#ifndef SVK_LOGGER_H
#define SVK_LOGGER_H

#include "svk/svk_common.h"

enum svkLogLevel
{
    SVK_LOG_LEVEL_DEBUG = 0,
    SVK_LOG_LEVEL_INFO  = 1,
    SVK_LOG_LEVEL_WARN  = 2,
    SVK_LOG_LEVEL_ERROR = 3
};

void SVK_Log(const u8 level, const char* file, u16 line, const char* fmt, ...);

#define SVK_LogDebug(...) SVK_Log(SVK_LOG_LEVEL_DEBUG, __FILE__, __LINE__, __VA_ARGS__);
#define SVK_LogInfo(...) SVK_Log(SVK_LOG_LEVEL_INFO, __FILE__, __LINE__, __VA_ARGS__);
#define SVK_LogWarn(...) SVK_Log(SVK_LOG_LEVEL_WARN, __FILE__, __LINE__, __VA_ARGS__);
#define SVK_LogError(...) SVK_Log(SVK_LOG_LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__);

#endif