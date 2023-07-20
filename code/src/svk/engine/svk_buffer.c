#include "svk/svk_engine.h"

// Internal Functions
//------------------------------------------------------------------------
internal VkResult CreateBuffer(
    VkDevice* device,
    const VkPhysicalDevice physicalDevice,
    const VkDeviceSize size,
    const VkBufferUsageFlags usage,
    const VkMemoryPropertyFlags flags,
    VkBuffer* outBuffer,
    VkDeviceMemory* outMemory)
{
    VkBufferCreateInfo bufferInfo = SVK_ZMSTRUCT2(VkBufferCreateInfo);
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result = vkCreateBuffer(*device, &bufferInfo, VK_NULL_HANDLE, outBuffer);
    if (result != VK_SUCCESS)
        return result;

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(*device, *outBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = VK_NULL_HANDLE;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = _svkEngine_FindMemoryType(physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    result = vkAllocateMemory(*device, &allocInfo, VK_NULL_HANDLE, outMemory);
    if (result != VK_SUCCESS)
        return result;

    return vkBindBufferMemory(*device, *outBuffer, *outMemory, 0);
}

internal VkResult CopyBuffer(
    VkDevice* device,
    VkCommandPool* commandPool,
    VkQueue* graphicsQueue,
    const VkBuffer src,
    const VkBuffer dest,
    const VkDeviceSize size)
{
    VkCommandBufferAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.pNext = VK_NULL_HANDLE;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = *commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuf = SVK_ZMSTRUCT2(VkCommandBuffer);
    VkResult result = vkAllocateCommandBuffers(*device, &allocInfo, &commandBuf);
    if (result != VK_SUCCESS)
        return result;

    VkCommandBufferBeginInfo beginInfo = SVK_ZMSTRUCT2(VkCommandBufferBeginInfo);
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    result = vkBeginCommandBuffer(commandBuf, &beginInfo);
    if (result != VK_SUCCESS)
        return result;

    VkBufferCopy copyRegion;
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuf, src, dest, 1, &copyRegion);
    result = vkEndCommandBuffer(commandBuf);
    if (result != VK_SUCCESS)
        return result;

    VkSubmitInfo submitInfo = SVK_ZMSTRUCT2(VkSubmitInfo);
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuf;

    result = vkQueueSubmit(*graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    if (result != VK_SUCCESS)
        return result;

    result = vkQueueWaitIdle(*graphicsQueue);
    if (result != VK_SUCCESS)
        return result;

    vkFreeCommandBuffers(*device, *commandPool, 1, &commandBuf);

    return VK_SUCCESS;
}

// Public Functions
//------------------------------------------------------------------------
VkResult _svkEngine_CreateVertexBuffer(
    const VkPhysicalDevice physicalDevice,
    const SVKVECTOR_TYPE(svkVertex) vertices,
    VkDevice* device,
    VkDeviceMemory* outVertexDeviceMem,
    VkBuffer* outVertexBuffer,
    VkCommandPool* commandPool,
    VkQueue* graphicsQueue)
{
    VkDeviceSize size = sizeof(svkVertex) * vertices->size;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    VkResult result = CreateBuffer(device, physicalDevice, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);
    if (result != VK_SUCCESS)
        return result;

    void* data;
    vkMapMemory(*device, stagingBufferMemory, 0, size, 0, &data);
    svkVertex* newVertices = SVK_ALLOCSTRUCT(svkVertex, vertices->size);
    for (size_t i = 0; i < vertices->size; i++)
        newVertices[i] = *(svkVertex*)vertices->data[i];

    memcpy(data, newVertices, size);
    vkUnmapMemory(*device, stagingBufferMemory);
    SVK_FREE(newVertices);

    result = CreateBuffer(device, physicalDevice, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, outVertexBuffer, outVertexDeviceMem);
    if (result != VK_SUCCESS)
        return result;

    CopyBuffer(device, commandPool, graphicsQueue, stagingBuffer, *outVertexBuffer, size);

    vkDestroyBuffer(*device, stagingBuffer, VK_NULL_HANDLE);
    vkFreeMemory(*device, stagingBufferMemory, VK_NULL_HANDLE);

    return VK_SUCCESS;
}

VkResult _svkEngine_CreateIndexBuffer(
    const VkPhysicalDevice physicalDevice,
    const SVKVECTOR_TYPE(uint16_t) indices,
    VkDevice* device,
    VkDeviceMemory* outIndexDeviceMem,
    VkBuffer* outIndexBuffer,
    VkCommandPool* commandPool,
    VkQueue* graphicsQueue)
{
    VkDeviceSize size = sizeof(uint16_t) * indices->size;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    VkResult result = CreateBuffer(device, physicalDevice, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);
    if (result != VK_SUCCESS)
        return result;

    void* data;
    vkMapMemory(*device, stagingBufferMemory, 0, size, 0, &data);
    uint16_t* newIndices = SVK_ALLOCSTRUCT(uint16_t, indices->size);
    for (size_t i = 0; i < indices->size; i++)
        newIndices[i] = *(uint16_t*)indices->data[i];

    memcpy(data, newIndices, size);
    vkUnmapMemory(*device, stagingBufferMemory);
    SVK_FREE(newIndices);

    result = CreateBuffer(device, physicalDevice, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, outIndexBuffer, outIndexDeviceMem);
    if (result != VK_SUCCESS)
        return result;

    CopyBuffer(device, commandPool, graphicsQueue, stagingBuffer, *outIndexBuffer, size);

    vkDestroyBuffer(*device, stagingBuffer, VK_NULL_HANDLE);
    vkFreeMemory(*device, stagingBufferMemory, VK_NULL_HANDLE);

    return VK_SUCCESS;
}

VkResult _svkEngine_CreateUniformBuffers(
    const VkPhysicalDevice physicalDevice,
    VkDevice* device,
    SVKARRAY_TYPE(VkBuffer)* outBuffers,
    SVKARRAY_TYPE(VkDeviceMemory)* outMemory,
    SVKARRAY_TYPE(void**) outMappedBuffers)
{
    VkDeviceSize size = sizeof(svkUniformBufferObj);
    *outBuffers = SVK_MALLOC(sizeof(VkBuffer) * MAX_FRAMES_IN_FLIGHT);
    *outMemory = SVK_MALLOC(sizeof(VkDeviceMemory) * MAX_FRAMES_IN_FLIGHT);
    *outMappedBuffers = SVK_MALLOC(sizeof(void*) * MAX_FRAMES_IN_FLIGHT);

    VkResult result = VK_SUCCESS;
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        result = CreateBuffer(device, physicalDevice, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &((*outBuffers)[i]), &((*outMemory)[i]));
        if (result != VK_SUCCESS)
            return result;

        result = vkMapMemory(*device, (*outMemory)[i], 0, size, 0, &((*outMappedBuffers)[i]));
        if (result != VK_SUCCESS)
            return result;
    }

    return VK_SUCCESS;
}