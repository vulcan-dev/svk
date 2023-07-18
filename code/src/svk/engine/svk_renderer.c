#include "svk/engine/svk_renderer.h"
#include "svk/engine/svk_command.h"
#include <Windows.h>

local uint32_t currentFrame = 0;

internal void FetchRenderTimeResults(
    const VkDevice device,
    struct _svkEngineDebug* debug,
    const VkPhysicalDevice physicalDevice,
    VkQueryPool timeQueryPool)
{
    uint64_t buffer[2];
    VkResult result = vkGetQueryPoolResults(device, timeQueryPool, currentFrame * 2, 2, sizeof(uint64_t) * 2, buffer, sizeof(uint64_t), VK_QUERY_RESULT_64_BIT);
    if (result == VK_NOT_READY)
    {
        return;
    } else if (result != VK_SUCCESS)
    {
        SVK_LogError("Failed querying pool results for time, error code: %d\n", result);
        return;
    }

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    float timestampPeriod = properties.limits.timestampPeriod;
    debug->gpuTime = (buffer[1] - buffer[0]) * timestampPeriod * 1e-6f;
}

VkResult _svkEngine_DrawFrame(
    SDL_Window* window,
    const VkPhysicalDevice physicalDevice,
    const VkSurfaceKHR surface,
    const VkDevice device,
    const VkCommandPool commandPool,
    VkQueryPool timeQueryPool,
    const SVKARRAY_TYPE(VkCommandBuffer) commandBuffers,
    const SVKVECTOR_TYPE(svkDrawable) drawables,
    const VkPipeline graphicsPipeline,
    const VkRenderPass renderPass,
    struct _svkEngineSwapChain* swapChain,
    struct _svkEngineCoreQueue queue,
    struct _svkEngineDebug* debug,
    struct _svkEngineRenderer* renderer)
{
    FetchRenderTimeResults(device, debug, physicalDevice, timeQueryPool);

    SVK_CHECK_VKRESULT(vkQueueWaitIdle(queue.graphics), "Failed to wait idle");
    SVK_CHECK_VKRESULT(vkWaitForFences(device, 1, &renderer->inFlightFences[currentFrame], VK_TRUE, UINT64_MAX), "Failed to wait for fence");

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, swapChain->swapChain, UINT64_MAX, renderer->imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        svkEngine_RecreateSwapChain(swapChain, device, physicalDevice, renderPass, surface, window);
        return result;
    }

    SVK_CHECK_VKRESULT(vkResetFences(device, 1, &renderer->inFlightFences[currentFrame]), "Failed to reset fence");

    vkResetCommandBuffer(commandBuffers[currentFrame], 0);
    _svkEngine_RecordCommandBuffer(device, commandBuffers[currentFrame], imageIndex, renderPass, renderer->clearColor, timeQueryPool, currentFrame, swapChain->frameBuffers, drawables, swapChain->extent, graphicsPipeline);

    VkSemaphore signalSemaphores[] = { renderer->renderFinishedSemaphores[currentFrame] };
    {
        VkSubmitInfo submitInfo = SVK_ZMSTRUCT2(VkSubmitInfo);
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { renderer->imageAvailableSemaphores[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        SVK_CHECK_VKRESULT(vkQueueSubmit(queue.graphics, 1, &submitInfo, renderer->inFlightFences[currentFrame]), "Failed to submit");
    }

    // Presentation
    VkPresentInfoKHR presentInfo = SVK_ZMSTRUCT2(VkPresentInfoKHR);
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { swapChain->swapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = NULL;

    result = vkQueuePresentKHR(queue.present, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        svkEngine_RecreateSwapChain(swapChain, device, physicalDevice, renderPass, surface, window);
        return result;
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    return VK_SUCCESS;
}