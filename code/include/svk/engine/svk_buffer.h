#ifndef SVK_BUFFER_H
#define SVK_BUFFER_H

#include "svk/svk_engine.h"

VkResult _svkEngine_CreateVertexBuffer(
    const VkPhysicalDevice physicalDevice,
    const SVKVECTOR_TYPE(svkVertex) vertices,
    VkDevice* device,
    VkDeviceMemory* outVertexDeviceMem,
    VkBuffer* outVertexBuffer,
    VkCommandPool* commandPool,
    VkQueue* graphicsQueue);

VkResult _svkEngine_CreateIndexBuffer(
    const VkPhysicalDevice physicalDevice,
    const SVKVECTOR_TYPE(uint16_t) indices,
    VkDevice* device,
    VkDeviceMemory* outIndexDeviceMem,
    VkBuffer* outIndexBuffer,
    VkCommandPool* commandPool,
    VkQueue* graphicsQueue);

VkResult _svkEngine_CreateUniformBuffers(
    const VkPhysicalDevice physicalDevice,
    VkDevice* device,
    SVKARRAY_TYPE(VkBuffer)* outBuffers,
    SVKARRAY_TYPE(VkDeviceMemory)* outMemory,
    SVKARRAY_TYPE(void**) outMappedBuffers);

#endif