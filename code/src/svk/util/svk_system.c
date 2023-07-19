#include "svk/util/svk_system.h"

#include <cpuid.h>
#include <memory.h>

svkCPUInfo svkGetSystemCPUInfo()
{
    svkCPUInfo info;
    unsigned int eax, ebx, ecx, edx;

    // Retrieve vendor string
    __cpuid(0, eax, ebx, ecx, edx);
    memcpy(info.vendor, &ebx, 4);
    memcpy(info.vendor + 4, &edx, 4);
    memcpy(info.vendor + 8, &ecx, 4);

    // Retrieve brand string
    for (int i = 0x80000002; i <= 0x80000004; ++i) {
        __cpuid(i, eax, ebx, ecx, edx);
        memcpy(info.brand + (i - 0x80000002) * 16, &eax, 4);
        memcpy(info.brand + (i - 0x80000002) * 16 + 4, &ebx, 4);
        memcpy(info.brand + (i - 0x80000002) * 16 + 8, &ecx, 4);
        memcpy(info.brand + (i - 0x80000002) * 16 + 12, &edx, 4);
    }

    return info;
}