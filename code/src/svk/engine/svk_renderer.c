#include "svk/engine/svk_renderer.h"
#include "svk/engine/svk_command.h"

local uint32_t currentFrame = 0;

VkResult _svkEngine_DrawFrame(
    SDL_Window* window,
    const VkPhysicalDevice physicalDevice,
    const VkSurfaceKHR surface,
    const VkDevice device,
    const SVKARRAY_TYPE(VkCommandBuffer) commandBuffers,
    const VkPipeline graphicsPipeline,
    const VkRenderPass renderPass,
    struct _svkEngineSwapChain* swapChain,
    struct _svkEngineCoreQueue queue,
    struct _svkEngineRenderer* renderer)
{
    vkWaitForFences(device, 1, &renderer->inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, swapChain->swapChain, UINT64_MAX, renderer->imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        svkEngine_RecreateSwapChain(swapChain, device, physicalDevice, renderPass, surface, window);
        return result;
    }

    vkResetFences(device, 1, &renderer->inFlightFences[currentFrame]);

    vkResetCommandBuffer(commandBuffers[currentFrame], 0);
    _svkEngine_RecordCommandBuffer(commandBuffers[currentFrame], imageIndex, renderPass, swapChain->frameBuffers, swapChain->extent, graphicsPipeline);

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

        result = vkQueueSubmit(queue.graphics, 1, &submitInfo, renderer->inFlightFences[currentFrame]);
        if (result != VK_SUCCESS)
            return result; // TODO: Cleanup
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