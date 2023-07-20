#include "svk/svk_engine.h"

// Internal Functions
//------------------------------------------------------------------------
internal VkRenderingInfoKHR PreRender(uint32_t imageIndex, svkEngine* engine, VkCommandBuffer commandBuffer, VkImageMemoryBarrier* imageMemoryBarrier)
{
    struct _svkEngineCore* core = &engine->core;
    struct _svkEngineSwapChain* swapChain = engine->swapChain;

    VkClearValue depthClearValue = SVK_ZMSTRUCT2(VkClearValue);
    depthClearValue.depthStencil = (VkClearDepthStencilValue){ 1.0f, 0 };

    // Color Attachment Info
    static VkRenderingAttachmentInfoKHR colorAttachmentInfo = SVK_ZMSTRUCT2(VkRenderingAttachmentInfoKHR);
    colorAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
    colorAttachmentInfo.imageView = (VkImageView)swapChain->imageViews->data[imageIndex];
    colorAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentInfo.clearValue = core->renderer.clearColor;

    // Depth
    static VkRenderingAttachmentInfoKHR depthStencilAttachment = SVK_ZMSTRUCT2(VkRenderingAttachmentInfoKHR);
    depthStencilAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
    depthStencilAttachment.imageView = core->depth.imageView;
    depthStencilAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthStencilAttachment.clearValue = depthClearValue;

    // Render Info
    VkRenderingInfoKHR renderInfo = SVK_ZMSTRUCT2(VkRenderingInfoKHR);
    renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
    renderInfo.renderArea.offset = (VkOffset2D){ 0, 0 };
    renderInfo.renderArea.extent = swapChain->extent;
    renderInfo.layerCount = 1;
    renderInfo.colorAttachmentCount = 1;
    renderInfo.pColorAttachments = &colorAttachmentInfo;
    renderInfo.pDepthAttachment = &depthStencilAttachment;

    // Subresource Range
    static VkImageSubresourceRange imageSubresourceRange = SVK_ZMSTRUCT2(VkImageSubresourceRange);
    imageSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageSubresourceRange.baseMipLevel = 0;
    imageSubresourceRange.levelCount = 1;
    imageSubresourceRange.baseArrayLayer = 0;
    imageSubresourceRange.layerCount = 1;

    // Image Barrier
    imageMemoryBarrier->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier->srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
    imageMemoryBarrier->dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    imageMemoryBarrier->oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageMemoryBarrier->newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    imageMemoryBarrier->image = (VkImage)swapChain->images->data[imageIndex];
    imageMemoryBarrier->subresourceRange = imageSubresourceRange;
    imageMemoryBarrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        0,
        0,
        NULL,
        0,
        NULL,
        1,
        imageMemoryBarrier);

    return renderInfo;
}

// Functions
//------------------------------------------------------------------------
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
    SVK_CHECK_VKRESULT(vkBeginCommandBuffer(commandBuffer, &beginInfo), "vkBeginCommandBuffer failed!");

    // TODO: Move these.
    PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR = (PFN_vkCmdBeginRenderingKHR)vkGetDeviceProcAddr(core->device, "vkCmdBeginRenderingKHR");
    PFN_vkCmdEndRenderingKHR vkCmdEndRenderingKHR = (PFN_vkCmdEndRenderingKHR)vkGetDeviceProcAddr(core->device, "vkCmdEndRenderingKHR");

    // Start Rendering
    VkImageMemoryBarrier imageMemoryBarrier = SVK_ZMSTRUCT2(VkImageMemoryBarrier);
    VkRenderingInfoKHR renderInfo = PreRender(imageIndex, engine, commandBuffer, &imageMemoryBarrier);
    vkCmdResetQueryPool(commandBuffer, core->timeQueryPool, core->currentFrame * 2, 2);
    vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, core->timeQueryPool, core->currentFrame * 2);
    vkCmdBeginRenderingKHR(commandBuffer, &renderInfo);

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
    vkCmdEndRenderingKHR(commandBuffer);

    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        0,
        0,
        NULL,
        0,
        NULL,
        1,
        &imageMemoryBarrier);

    vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, core->timeQueryPool, core->currentFrame * 2 + 1);
    return vkEndCommandBuffer(commandBuffer);
}