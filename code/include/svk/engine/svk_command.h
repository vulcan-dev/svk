#ifndef SVK_COMMAND_H
#define SVK_COMMAND_H

#include "svk/svk_engine.h"

VkResult _svkEngine_CreateCommandPool(
    const VkDevice device,
    const VkPhysicalDevice physicalDevice,
    const VkSurfaceKHR surface,
    VkCommandPool* outCommandPool);

VkResult _svkEngine_CreateCommandBuffers(
    const VkDevice device,
    const VkCommandPool commandPool,
    SVKARRAY_TYPE(VkCommandBuffer)* commandBuffer);

VkResult _svkEngine_RecordCommandBuffer(
    const VkCommandBuffer commandBuffer,
    const u8 imageIndex,
    const VkRenderPass renderPass,
    const SVKVECTOR_TYPE(VkFramebuffer) swapChainFramebuffers,
    const VkExtent2D swapChainExtent,
    const VkPipeline graphicsPipeline);

#endif