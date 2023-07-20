#include "svk/engine/svk_memory.h"

uint32_t _svkEngine_FindMemoryType(const VkPhysicalDevice physicalDevice,
                                   const uint32_t typeFilter,
                                   const VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }

    return 0;
}
