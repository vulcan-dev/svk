#ifndef SVK_RENDERER_H
#define SVK_RENDERER_H

#include "svk/svk_engine.h"

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
    struct _svkEngineRenderer* renderer);

#endif