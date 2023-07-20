#include "svk/engine/svk_renderer.h"
#include "svk/engine/svk_command.h"
#include <Windows.h>

internal void FetchRenderTimeResults(
    const VkDevice device,
    struct _svkEngineDebug* debug,
    const VkPhysicalDevice physicalDevice,
    const uint32_t currentFrame,
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

VkResult _svkEngine_DrawFrame(SDL_Window* window, svkEngine* engine)
{
    struct _svkEngineCore* core = &engine->core;
    struct _svkEngineSwapChain* swapChain = engine->swapChain;
    struct _svkEngineRenderer* renderer = &core->renderer;

    //FetchRenderTimeResults(core->device, &engine->debug, core->physicalDevice, core->currentFrame, core->timeQueryPool);

    SVK_CHECK_VKRESULT(vkQueueWaitIdle(core->queues.graphics), "Failed to wait idle");
    SVK_CHECK_VKRESULT(vkWaitForFences(core->device, 1, &renderer->inFlightFences[core->currentFrame], VK_TRUE, UINT64_MAX), "Failed to wait for fence");

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(core->device, swapChain->swapChain, UINT64_MAX, renderer->imageAvailableSemaphores[core->currentFrame], VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        svkEngine_RecreateSwapChain(engine, window);
        return result;
    }

    SVK_CHECK_VKRESULT(vkResetFences(core->device, 1, &renderer->inFlightFences[core->currentFrame]), "Failed to reset fence");

    vkResetCommandBuffer(core->commandBuffers[core->currentFrame], 0);
    _svkEngine_RecordCommandBuffer(engine, imageIndex);

    VkSemaphore signalSemaphores[] = { renderer->renderFinishedSemaphores[core->currentFrame] };
    {
        VkSubmitInfo submitInfo = SVK_ZMSTRUCT2(VkSubmitInfo);
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { renderer->imageAvailableSemaphores[core->currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &core->commandBuffers[core->currentFrame];

        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        SVK_CHECK_VKRESULT(vkQueueSubmit(core->queues.graphics, 1, &submitInfo, renderer->inFlightFences[core->currentFrame]), "Failed to submit");
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

    result = vkQueuePresentKHR(core->queues.present, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        svkEngine_RecreateSwapChain(engine, window);
        return result;
    }

    core->currentFrame = (core->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    svkScene_PostRender(engine);
    return VK_SUCCESS;
}