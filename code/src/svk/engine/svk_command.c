#include "svk/svk_engine.h"

VkResult _svkEngine_CreateCommandPool(
    const VkDevice device,
    const VkPhysicalDevice physicalDevice,
    const VkSurfaceKHR surface,
    VkCommandPool* outCommandPool)
{
    svkQueueFamilyIndices queueFamilyIndices = _svkEngine_FindQueueFamilies(physicalDevice, surface);

    VkCommandPoolCreateInfo poolInfo;
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.pNext = VK_NULL_HANDLE;

    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

    return vkCreateCommandPool(device, &poolInfo, NULL, outCommandPool);
}

VkResult _svkEngine_CreateCommandBuffers(
    const VkDevice device,
    const VkCommandPool commandPool,
    SVKARRAY_TYPE(VkCommandBuffer)* commandBuffers)
{
    *commandBuffers = SVK_ALLOCSTRUCT(VkCommandBuffer, MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo = SVK_ZMSTRUCT2(VkCommandBufferAllocateInfo);
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

    return vkAllocateCommandBuffers(device, &allocInfo, *commandBuffers);
}

VkResult _svkEngine_RecordCommandBuffer(
    const VkCommandBuffer commandBuffer,
    const u8 imageIndex,
    const VkRenderPass renderPass,
    const SVKVECTOR_TYPE(VkFramebuffer) swapChainFramebuffers,
    const VkExtent2D swapChainExtent,
    const VkPipeline graphicsPipeline)
{
    VkCommandBufferBeginInfo beginInfo;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pNext = VK_NULL_HANDLE;
    beginInfo.pInheritanceInfo = NULL;

    VkResult result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    if (result != VK_SUCCESS)
        return result;

    // Starting the Render Pass
    VkRenderPassBeginInfo renderPassInfo;
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.pNext = VK_NULL_HANDLE;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = (VkFramebuffer)swapChainFramebuffers->data[imageIndex];
    renderPassInfo.renderArea.offset = (VkOffset2D){ 0, 0 };
    renderPassInfo.renderArea.extent = swapChainExtent;

    VkClearValue clearColor = {{{ 0.0f, 0.0f, 0.0f, 1.0f }}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor;
    scissor.offset = (VkOffset2D){ 0, 0 };
    scissor.extent = swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    vkCmdEndRenderPass(commandBuffer);

    result = vkEndCommandBuffer(commandBuffer);
    return result;
}