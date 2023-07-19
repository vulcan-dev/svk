#ifndef SVK_SYSTEM_H
#define SVK_SYSTEM_H

typedef struct svkCPUInfo
{
    char vendor[32];
    char brand[64];
} svkCPUInfo;

svkCPUInfo svkGetSystemCPUInfo();

#endif