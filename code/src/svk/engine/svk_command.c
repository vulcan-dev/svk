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

VkResult _svkEngine_RecordCommandBuffer(svkEngine* engine, const uint32_t imageIndex)
{
    struct _svkEngineSwapChain* swapChain = engine->swapChain;
    struct _svkEngineCore* core = &engine->core;

    VkCommandBufferBeginInfo beginInfo;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pNext = VK_NULL_HANDLE;
    beginInfo.pInheritanceInfo = NULL;

    VkCommandBuffer commandBuffer = core->commandBuffers[core->currentFrame];
    VkResult result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    if (result != VK_SUCCESS)
        return result;

    // Starting the Render Pass
    {
        VkRenderPassBeginInfo renderPassInfo;
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.pNext = VK_NULL_HANDLE;
        renderPassInfo.renderPass = core->renderPass;
        renderPassInfo.framebuffer = (VkFramebuffer)swapChain->frameBuffers->data[imageIndex];
        renderPassInfo.renderArea.offset = (VkOffset2D){ 0, 0 };
        renderPassInfo.renderArea.extent = swapChain->extent;

        VkClearValue depthClearValue = SVK_ZMSTRUCT2(VkClearValue);
        depthClearValue.depthStencil = (VkClearDepthStencilValue){ 1.0f, 0 };

        VkClearValue clearValues[2] = { core->renderer.clearColor, depthClearValue };

        renderPassInfo.clearValueCount = SVK_ARRAY_SIZE(clearValues);
        renderPassInfo.pClearValues = clearValues;

        // Begin Render Pass
        vkCmdResetQueryPool(commandBuffer, core->timeQueryPool, core->currentFrame * 2, 2);
        vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, core->timeQueryPool, core->currentFrame * 2);

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    // Bind and Render
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, core->graphicsPipeline);

        VkViewport viewport;
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)swapChain->extent.width; 
        viewport.height = (float)swapChain->extent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor;
        scissor.offset = (VkOffset2D){ 0, 0 };
        scissor.extent = swapChain->extent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        svkScene_Render(engine, commandBuffer);
    }

    // End
    vkCmdEndRenderPass(commandBuffer);
    vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, core->timeQueryPool, core->currentFrame * 2 + 1);
    return vkEndCommandBuffer(commandBuffer);
}