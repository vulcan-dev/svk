#include "svk/svk_engine.h"
#include "svk/util/svk_vector.h"
#include "svk/engine/svk_command.h"

#include "engine/svk_device.c"
#include "engine/svk_debug.c"
#include "engine/svk_render.c"

#include <SDL2/SDL_vulkan.h>

#define VALIDATION_LAYER_ENABLED true // TODO: Make it a variable

local const char* validationLayers[1] = {
    "VK_LAYER_KHRONOS_validation"
};

local SvkResult lastErrorCode = SVK_SUCCESS; // svk_engine.h
SvkResult SVK_GetLastError() {
    return lastErrorCode;
}

// Internal Functions
//------------------------------------------------------------------------
internal svkVector* GetRequiredExtensions(SDL_Window* window)
{
    // Get SDL's Extensions
    uint32_t sdlExtensionCount = 0;
    if (SDL_Vulkan_GetInstanceExtensions(window, &sdlExtensionCount, NULL) == SDL_FALSE)
    {
        lastErrorCode = SVK_ERROR_GET_INSTANCE_EXTENSIONS;
        return NULL;
    }

    const char** sdlExtensions = SVK_ZMSTRUCT(const char*, sdlExtensionCount);
    if (SDL_Vulkan_GetInstanceExtensions(window, &sdlExtensionCount, sdlExtensions) == SDL_FALSE)
    {
        lastErrorCode = SVK_ERROR_GET_INSTANCE_EXTENSIONS;
        return NULL;
    }

    // Add the extensions
    u32 totalExtensions = (VALIDATION_LAYER_ENABLED ? 3 : 1) + (u32)sdlExtensionCount;
    svkVector* extensions = svkVector_Create(totalExtensions, sizeof(const char*));
    if (VALIDATION_LAYER_ENABLED)
        SVKVECTOR_PUSHBACK(extensions, "VK_EXT_debug_utils");

    for (u16 i = 0; i < sdlExtensionCount; i++)
        SVKVECTOR_PUSHBACK(extensions, sdlExtensions[i]);

    return extensions;
}

internal bool ValidateLayerSupport()
{
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    VkLayerProperties* availableLayers = SVK_ALLOCSTRUCT(VkLayerProperties, layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

    for (u16 i = 0; i < SVK_ARRAY_SIZE(validationLayers); i++)
    {
        bool layerFound = false;
        for (u16 j = 0; j < layerCount; j++)
        {
            if (strcmp(validationLayers[i], availableLayers[j].layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            SVK_FREE(availableLayers);
            return false;
        }
    }

    SVK_FREE(availableLayers);
    return true;
}

internal VkResult CreateSyncObjects(
    const VkDevice device,
    struct _svkEngineRenderer* renderer)
{
    renderer->imageAvailableSemaphores = SVK_ALLOCSTRUCT(VkSemaphore, MAX_FRAMES_IN_FLIGHT);
    renderer->renderFinishedSemaphores = SVK_ALLOCSTRUCT(VkSemaphore, MAX_FRAMES_IN_FLIGHT);
    renderer->inFlightFences = SVK_ALLOCSTRUCT(VkFence, MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo = SVK_ZMSTRUCT2(VkSemaphoreCreateInfo);
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo;
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.pNext = VK_NULL_HANDLE;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkResult result = vkCreateSemaphore(device, &semaphoreInfo, NULL, &renderer->imageAvailableSemaphores[i]);
        if (result != VK_SUCCESS)
            return result;

        result = vkCreateSemaphore(device, &semaphoreInfo, NULL, &renderer->renderFinishedSemaphores[i]);
        if (result != VK_SUCCESS)
            return result;

        result = vkCreateFence(device, &fenceInfo, NULL, &renderer->inFlightFences[i]);
        if (result != VK_SUCCESS)
            return result;
    }

    return VK_SUCCESS;
}

svkQueueFamilyIndices _svkEngine_FindQueueFamilies(
    const VkPhysicalDevice physicalDevice,
    const VkSurfaceKHR surface)
{
    svkQueueFamilyIndices indices;
    indices.graphicsFamily = -1;
    indices.presentFamily = -1;

    uint32_t queueFamilyCount = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, NULL);
    VkQueueFamilyProperties* queueFamilies = SVK_ALLOCSTRUCT(VkQueueFamilyProperties, queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies);

    VkBool32 presentSupport = false;

    for (u32 i = 0; i < queueFamilyCount; i++)
    {
        const VkQueueFamilyProperties queueFamily = queueFamilies[i];
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphicsFamily = i;

        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
        if (presentSupport)
            indices.presentFamily = i;

        if (indices.graphicsFamily > 0)
            break;
    }

    SVK_FREE(queueFamilies);
    return indices;
}

internal void CleanupSwapChain(const VkDevice device, struct _svkEngineSwapChain* swapchain)
{
    for (size_t i = 0; i < swapchain->frameBuffers->size; i++)
        vkDestroyFramebuffer(device, swapchain->frameBuffers->data[i], VK_NULL_HANDLE);

    for (size_t i = 0; i < swapchain->imageViews->size; i++)
        vkDestroyImageView(device, swapchain->imageViews->data[i], VK_NULL_HANDLE);

    vkDestroySwapchainKHR(device, swapchain->swapChain, VK_NULL_HANDLE);
}

// Engine Functions
//------------------------------------------------------------------------
svkEngine* svkEngine_Create(const char* appName, const u32 appVersion)
{
    svkEngine* svke = SVK_ZMSTRUCT(svkEngine, 1);
    svke->swapChain = SVK_ZMSTRUCT(struct _svkEngineSwapChain, 1);

    svke->info = (struct _svkEngineInfo){
        appVersion,
        appName
    };

    return svke;
}

VkResult svkEngine_RecreateSwapChain(
    struct _svkEngineSwapChain* swapChain,
    const VkDevice device,
    const VkPhysicalDevice physicalDevice,
    const VkRenderPass renderPass,
    const VkSurfaceKHR surface,
    SDL_Window* window)
{
    VkResult result = vkDeviceWaitIdle(device);
    if (result != VK_SUCCESS)
        return result;

    CleanupSwapChain(device, swapChain);

    result = CreateSwapChain(
        &swapChain->swapChain,
        &swapChain->images,
        &swapChain->imageFormat,
        &swapChain->extent,
        device,
        physicalDevice,
        surface,
        window);
    if (result != VK_SUCCESS)
        return result;

    result = CreateImageViews(swapChain->images, &swapChain->imageViews, device, swapChain->imageFormat);
    if (result != VK_SUCCESS)
        return result;

    result = CreateFrameBuffers(
        device,
        renderPass,
        swapChain->extent,
        swapChain->imageViews,
        &swapChain->frameBuffers);
    if (result != VK_SUCCESS)
        return result;

    return VK_SUCCESS;
}

bool _svkEngine_Initialize(svkEngine* svke, SDL_Window* window)
{
    // Application Info
    VkApplicationInfo appInfo = SVK_ZMSTRUCT2(VkApplicationInfo);
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = svke->info.appName;
    appInfo.applicationVersion = svke->info.version;
    appInfo.pEngineName = "SVK";
    appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    // Instance Info
    svkVector* extensions = GetRequiredExtensions(window);
    if (extensions == NULL)
        return false;

    VkInstanceCreateInfo createInfo = SVK_ZMSTRUCT2(VkInstanceCreateInfo);
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = extensions->size;
    createInfo.ppEnabledExtensionNames = (const char* const*)extensions->data;

    if (VALIDATION_LAYER_ENABLED && !ValidateLayerSupport()) // Create before createInfo?
    {
        lastErrorCode = SVK_ERROR_VALIDATE_LAYER_SUPPORT;
        SVK_FREE(extensions);
        return false;
    }

    // Debug Messenger
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = SVK_ZMSTRUCT2(VkDebugUtilsMessengerCreateInfoEXT);
    if (VALIDATION_LAYER_ENABLED)
    {
        createInfo.enabledLayerCount = SVK_ARRAY_SIZE(validationLayers);
        createInfo.ppEnabledLayerNames = validationLayers;

        PopulateDebugMessengerCreateInfo(&debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    } else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = VK_NULL_HANDLE;
    }

    // Instance
    VkInstance instance;
    if (vkCreateInstance(&createInfo, VK_NULL_HANDLE, &instance) != VK_SUCCESS)
    {
        lastErrorCode = SVK_ERROR_CREATE_INSTANCE;
        SVK_FREE(extensions);
        return false;
    }

    SVK_FREE(extensions);

    svke->core.instance = instance;

    // Setup everything else from here on
    if (VALIDATION_LAYER_ENABLED)
        SetupDebugMessenger(&svke->core.instance, &svke->debugMessenger);

    // Create surface
    if (SDL_Vulkan_CreateSurface(window, instance, &svke->core.surface) == SDL_FALSE)
        return false;

    // Pick physical device
    lastErrorCode = PickPhysicalDevice(instance, &svke->core.physicalDevice, svke->core.surface);
    if (lastErrorCode != SVK_SUCCESS)
        return false;

    // Create logical device
    if (CreateLogicalDevice(svke->core.physicalDevice, &svke->core.device, &svke->core.queues, svke->core.surface, VALIDATION_LAYER_ENABLED, validationLayers) != VK_SUCCESS)
        return false;

    // Create swapchain
    CreateSwapChain(
        &svke->swapChain->swapChain,
        &svke->swapChain->images,
        &svke->swapChain->imageFormat,
        &svke->swapChain->extent,
        svke->core.device,
        svke->core.physicalDevice,
        svke->core.surface,
        window);

    // Create image views
    CreateImageViews(svke->swapChain->images, &svke->swapChain->imageViews, svke->core.device, svke->swapChain->imageFormat);
    
    // Create render pass
    CreateRenderPass(svke->core.device, svke->swapChain->imageFormat, &svke->core.renderPass);

    // Setup shaders for pipeline
    svke->core.shaders = svkVector_Create(2, sizeof(svkShader));
    svkShader* coreVertexShader = svkShader_CreateFromFile(svke->core.device, "rom/shaders/compiled/vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    if (coreVertexShader == NULL)
        return false;

    svkShader* coreFragmentShader = svkShader_CreateFromFile(svke->core.device, "rom/shaders/compiled/frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    if (coreFragmentShader == NULL)
        return false;

    svkVector_PushBack(svke->core.shaders, coreVertexShader);
    svkVector_PushBack(svke->core.shaders, coreFragmentShader);

    // Create graphics pipeline
    CreateGraphicsPipeline(
        svke->core.device,
        svke->core.shaders,
        svke->swapChain->extent,
        svke->core.renderPass,
        &svke->core.graphicsPipeline,
        &svke->core.pipelineLayout);

    // Create framebuffers
    CreateFrameBuffers(
        svke->core.device,
        svke->core.renderPass,
        svke->swapChain->extent,
        svke->swapChain->imageViews,
        &svke->swapChain->frameBuffers);

    // Create command pool
    _svkEngine_CreateCommandPool(svke->core.device, svke->core.physicalDevice, svke->core.surface, &svke->core.commandPool);

    // Create command buffer
    _svkEngine_CreateCommandBuffers(svke->core.device, svke->core.commandPool, &svke->core.commandBuffers);

    // Create sync objects
    CreateSyncObjects(svke->core.device, &svke->core.renderer);

    return true;
}

void svkEngine_Destroy(svkEngine* svke)
{
    SVK_ASSERT(svke == NULL, "svke is NULL!");

    CleanupSwapChain(svke->core.device, svke->swapChain);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(svke->core.device, svke->core.renderer.renderFinishedSemaphores[i], VK_NULL_HANDLE);
        vkDestroySemaphore(svke->core.device, svke->core.renderer.imageAvailableSemaphores[i], VK_NULL_HANDLE);
        vkDestroyFence(svke->core.device, svke->core.renderer.inFlightFences[i], VK_NULL_HANDLE);
    }

    vkDestroyCommandPool(svke->core.device, svke->core.commandPool, VK_NULL_HANDLE);
    vkDestroyPipeline(svke->core.device, svke->core.graphicsPipeline, VK_NULL_HANDLE);
    vkDestroyPipelineLayout(svke->core.device, svke->core.pipelineLayout, VK_NULL_HANDLE);
    vkDestroyRenderPass(svke->core.device, svke->core.renderPass, VK_NULL_HANDLE);

    for (size_t i = 0; i < svke->core.shaders->size; i++)
        vkDestroyShaderModule(svke->core.device, ((svkShader*)svke->core.shaders->data[i])->shader, VK_NULL_HANDLE);

    vkDestroyDevice(svke->core.device, VK_NULL_HANDLE);
    vkDestroySurfaceKHR(svke->core.instance, svke->core.surface, VK_NULL_HANDLE);

    if (VALIDATION_LAYER_ENABLED)
        DestroyDebugUtilsMessengerEXT(svke->core.instance, svke->debugMessenger, VK_NULL_HANDLE);

    vkDestroyInstance(svke->core.instance, VK_NULL_HANDLE);

    SVK_FREE(svke->core.commandBuffers);
    SVK_FREE(svke->core.renderer.imageAvailableSemaphores);
    SVK_FREE(svke->core.renderer.renderFinishedSemaphores);
    SVK_FREE(svke->core.renderer.inFlightFences);

    svkVector_Free(svke->core.shaders);
    svkVector_Free(svke->swapChain->images);
    svkVector_Free(svke->swapChain->imageViews);

    SVK_FREE(svke->swapChain);
    SVK_FREE(svke);
}