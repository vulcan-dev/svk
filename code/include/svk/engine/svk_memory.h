#ifndef SVK_MEMORY_H
#define SVK_MEMORY_H

#include "svk/svk_engine.h"

uint32_t _svkEngine_FindMemoryType(
    const VkPhysicalDevice physicalDevice,
    const uint32_t typeFilter,
    const VkMemoryPropertyFlags properties);

#endif