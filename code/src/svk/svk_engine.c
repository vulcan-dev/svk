#include "svk/svk_engine.h"
#include "svk/util/svk_vector.h"
#include "svk/engine/svk_command.h"
#include "svk/engine/svk_descriptor.h"

#include "engine/svk_device.c"
#include "engine/svk_debug.c"
#include "engine/svk_render.c"

#include <SDL2/SDL_vulkan.h>

#define VALIDATION_LAYER_ENABLED true // TODO: Make it a variable

local const char* validationLayers[1] = {
    "VK_LAYER_KHRONOS_validation"
};

SvkResult lastErrorCode = SVK_SUCCESS; // TODO: Remove
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

internal void CleanupSwapChain(
    svkEngine* svke,
    const VkDevice device,
    struct _svkEngineSwapChain* swapchain)
{
    vkDestroyImageView(svke->core.device, svke->core.depth.imageView, VK_NULL_HANDLE);
    vkDestroyImage(svke->core.device, svke->core.depth.image, VK_NULL_HANDLE);
    vkFreeMemory(svke->core.device, svke->core.depth.imageMemory, VK_NULL_HANDLE);

    for (size_t i = 0; i < swapchain->imageViews->size; i++)
        vkDestroyImageView(device, swapchain->imageViews->data[i], VK_NULL_HANDLE);

    vkDestroySwapchainKHR(device, swapchain->swapChain, VK_NULL_HANDLE);
}

internal VkFormat FindSupportedFormat(
    const VkPhysicalDevice physicalDevice,
    const SVKARRAY_TYPE(VkFormat) candidates,
    const uint32_t candidatesLength,
    VkImageTiling tiling,
    VkFormatFeatureFlags features)
{
    for (size_t i = 0; i < candidatesLength; i++)
    {
        VkFormat format = (VkFormat)candidates[i];

        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
        if ((tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
            || tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
            return format;
    }

    return VK_FORMAT_UNDEFINED;
}

internal bool HasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkFormat _svkEngine_FindDepthFormat(const VkPhysicalDevice physicalDevice)
{
    const VkFormat formats[3] = {
        VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT
    };

    return FindSupportedFormat(
        physicalDevice,
        formats,
        3,
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

internal VkResult _svkEngine_CreateDepthResources(
    VkDevice device,
    VkPhysicalDevice physicalDevice,
    VkExtent2D extent,
    VkImage* outDepthImage,
    VkImageView* outDepthImageView,
    VkDeviceMemory* outDepthImageMemory)
{
    VkFormat depthFormat = _svkEngine_FindDepthFormat(physicalDevice);
    if (depthFormat == VK_FORMAT_UNDEFINED)
        return VK_ERROR_UNKNOWN;

    VkResult result = _svkEngine_CreateImage(device, physicalDevice, extent.width, extent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, outDepthImage, outDepthImageMemory);
    if (result != VK_SUCCESS)
        return result;

    return _svkEngine_CreateImageView(device, *outDepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, outDepthImageView);
}

// Engine Functions
//------------------------------------------------------------------------
svkEngine* svkEngine_Create(const char* appName, const u32 appVersion)
{
    svkEngine* svke = SVK_ZMSTRUCT(svkEngine, 1);
    svke->swapChain = SVK_ZMSTRUCT(struct _svkEngineSwapChain, 1);
    svke->scene = SVK_ZMSTRUCT(struct _svkEngineScene, 1);
//    svke->core.vertexBuffers = svkVector_Create(0, sizeof(VkBuffer));
//    svke->core.indexBuffers = svkVector_Create(0, sizeof(VkBuffer));

    svke->info = (struct _svkEngineInfo){
        appVersion,
        appName
    };

    return svke;
}

VkResult svkEngine_RecreateSwapChain(svkEngine* engine, SDL_Window* window)
{
    int width, height = 0;
    SDL_GetWindowSize(window, &width, &height);
    while (width == 0 || height == 0)
    {
        SDL_PumpEvents();
        SDL_GetWindowSize(window, &width, &height);
    }

    VkResult result = vkDeviceWaitIdle(engine->core.device);
    if (result != VK_SUCCESS)
        return result;

    CleanupSwapChain(engine, engine->core.device, engine->swapChain);

    result = CreateSwapChain(
        &engine->swapChain->swapChain,
        &engine->swapChain->images,
        &engine->swapChain->imageFormat,
        &engine->swapChain->extent,
        engine->core.device,
        engine->core.physicalDevice,
        engine->core.surface,
        window);
    if (result != VK_SUCCESS)
        return result;

    result = CreateImageViews(engine->swapChain->images, &engine->swapChain->imageViews, engine->core.device, engine->swapChain->imageFormat);
    if (result != VK_SUCCESS)
        return result;

    result = _svkEngine_CreateDepthResources(
        engine->core.device,
        engine->core.physicalDevice,
        engine->swapChain->extent,
        &engine->core.depth.image,
        &engine->core.depth.imageView,
        &engine->core.depth.imageMemory);
    if (result != VK_SUCCESS)
        return result;

//    result = CreateFrameBuffers(
//        engine->core.device,
//        engine->core.renderPass,
//        engine->swapChain->extent,
//        engine->swapChain->imageViews,
//        engine->core.depth.imageView,
//        &engine->swapChain->frameBuffers);
//    if (result != VK_SUCCESS)
//        return result;

    return VK_SUCCESS;
}

VkResult _svkEngine_Initialize(svkEngine* svke, SDL_Window* window)
{
    SVK_LogInfo("Setting engine up...");

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
        return VK_ERROR_UNKNOWN;

    VkInstanceCreateInfo createInfo = SVK_ZMSTRUCT2(VkInstanceCreateInfo);
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = extensions->size;
    createInfo.ppEnabledExtensionNames = (const char* const*)extensions->data;

    if (VALIDATION_LAYER_ENABLED && !ValidateLayerSupport()) // Create before createInfo?
    {
        lastErrorCode = SVK_ERROR_VALIDATE_LAYER_SUPPORT;
        SVK_FREE(extensions);
        return VK_ERROR_UNKNOWN;
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
        return VK_ERROR_UNKNOWN;
    }

    SVK_FREE(extensions);

    svke->core.instance = instance;
    VkResult result = VK_SUCCESS;

    // Setup everything else from here on
    if (VALIDATION_LAYER_ENABLED)
    {
        result = SetupDebugMessenger(&svke->core.instance, &svke->debugMessenger);
        if (result != VK_SUCCESS)
            return result;
    }

    // Create surface
    if (SDL_Vulkan_CreateSurface(window, instance, &svke->core.surface) == SDL_FALSE)
        return VK_ERROR_UNKNOWN;

    // Pick physical device
    lastErrorCode = PickPhysicalDevice(instance, &svke->core.physicalDevice, svke->core.surface);
    if (lastErrorCode != SVK_SUCCESS)
        return VK_ERROR_UNKNOWN;

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(svke->core.physicalDevice, &deviceProperties);

    SVK_LogInfo("Found GPU:");
    SVK_LogInfo("    Name: %s", deviceProperties.deviceName);
    SVK_LogInfo("    Vendor ID: %d", deviceProperties.vendorID);
    SVK_LogInfo("    Device ID: %d", deviceProperties.deviceID);
    SVK_LogInfo("    API Version: %d.%d.%d",
                VK_VERSION_MAJOR(deviceProperties.apiVersion),
                VK_VERSION_MINOR(deviceProperties.apiVersion),
                VK_VERSION_PATCH(deviceProperties.apiVersion));

    svkCPUInfo info = svkGetSystemCPUInfo();
    SVK_LogInfo("CPU Vendor: %s", info.vendor);
    SVK_LogInfo("CPU Brand: %s", info.brand);

    // Create logical device
    result = CreateLogicalDevice(svke->core.physicalDevice, &svke->core.device, &svke->core.queues, svke->core.surface, VALIDATION_LAYER_ENABLED, validationLayers);
    if (result != VK_SUCCESS)
        return result;

    // Create swapchain
    result = CreateSwapChain(
        &svke->swapChain->swapChain,
        &svke->swapChain->images,
        &svke->swapChain->imageFormat,
        &svke->swapChain->extent,
        svke->core.device,
        svke->core.physicalDevice,
        svke->core.surface,
        window);
    if (result != VK_SUCCESS)
        return result;

    // Create image views
    result = CreateImageViews(svke->swapChain->images, &svke->swapChain->imageViews, svke->core.device, svke->swapChain->imageFormat);
    if (result != VK_SUCCESS)
        return result;
    
    // Create render pass
//    result = CreateRenderPass(svke->core.device, svke->core.physicalDevice, svke->swapChain->imageFormat, &svke->core.renderPass);
//    if (result != VK_SUCCESS)
//        return result;

    // Create descriptor layout
    result = _svkEngine_CreateDescriptorLayout(svke->core.device, &svke->core.descriptorSetLayout);
    if (result != VK_SUCCESS)
        return result;

    // Setup shaders for pipeline
    svke->core.shaders = svkVector_Create(2, sizeof(svkShader));
    svkShader* coreVertexShader = svkShader_CreateFromFile(svke->core.device, "rom/shaders/compiled/vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    if (coreVertexShader == NULL)
        return VK_ERROR_UNKNOWN;

    svkShader* coreFragmentShader = svkShader_CreateFromFile(svke->core.device, "rom/shaders/compiled/frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    if (coreFragmentShader == NULL)
        return VK_ERROR_UNKNOWN;

    svkVector_PushBack(svke->core.shaders, coreVertexShader);
    svkVector_PushBack(svke->core.shaders, coreFragmentShader);

    // Create graphics pipeline
    result = CreateGraphicsPipeline(
        svke->core.device,
        svke->core.physicalDevice,
        svke->swapChain->imageFormat,
        svke->core.shaders,
        svke->swapChain->extent,
        svke->core.renderPass,
        svke->core.descriptorSetLayout,
        &svke->core.graphicsPipeline,
        &svke->core.pipelineLayout);
    if (result != VK_SUCCESS)
        return result;

    // Create command pool
    result = _svkEngine_CreateCommandPool(svke->core.device, svke->core.physicalDevice, svke->core.surface, &svke->core.commandPool);
    if (result != VK_SUCCESS)
        return result;

    // Create depth resources
    result = _svkEngine_CreateDepthResources(svke->core.device, svke->core.physicalDevice, svke->swapChain->extent, &svke->core.depth.image, &svke->core.depth.imageView, &svke->core.depth.imageMemory);
    if (result == VK_ERROR_UNKNOWN) {
        SVK_LogError("Could not find supported format for depth buffer");
        return VK_ERROR_UNKNOWN;
    } else if (result != VK_SUCCESS)
    {
        return result;
    }

    // Create framebuffers
//    CreateFrameBuffers(
//        svke->core.device,
//        svke->core.renderPass,
//        svke->swapChain->extent,
//        svke->swapChain->imageViews,
//        svke->core.depth.imageView,
//        &svke->swapChain->frameBuffers);

    // Create descriptor pool
    result = _svkEngine_CreateDescriptorPool(svke->core.device, &svke->core.descriptorPool);
    if (result != VK_SUCCESS)
        return result;

    // Create command buffer
    result = _svkEngine_CreateCommandBuffers(svke->core.device, svke->core.commandPool, &svke->core.commandBuffers);
    if (result != VK_SUCCESS)
        return result;

    // Create sync objects
    result = CreateSyncObjects(svke->core.device, &svke->core.renderer);
    if (result != VK_SUCCESS)
        return result;

    // Create time query pool
    VkQueryPoolCreateInfo poolCreateInfo = SVK_ZMSTRUCT2(VkQueryPoolCreateInfo);
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    poolCreateInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
    poolCreateInfo.queryCount = 4;

    result = vkCreateQueryPool(svke->core.device, &poolCreateInfo, VK_NULL_HANDLE, &svke->core.timeQueryPool);
    if (result != VK_SUCCESS)
        return result;

    // Finish up
    svke->core.renderer.clearColor = (VkClearValue){{{ 0.11f, 0.13f, 0.18f, 1.0f }}};

    return VK_SUCCESS;
}

svkDrawable* svkDrawable_Create(
    const svkVertex* vertices,
    const uint16_t vertexCount,
    const uint16_t* indices,
    const uint16_t indexCount)
{
    svkDrawable* drawable = SVK_ZMSTRUCT(svkDrawable, 1);
    drawable->vertices = svkVector_Create(vertexCount, sizeof(svkVertex));
    drawable->indices = svkVector_Create(indexCount, 8);

    for (uint16_t i = 0; i < vertexCount; i++)
        svkVector_PushBack(drawable->vertices, (void*)&vertices[i]);

    for (uint16_t i = 0; i < indexCount; i++)
        svkVector_PushBack(drawable->indices, (void*)&indices[i]);

    return drawable;
}

void svkEngine_Destroy(svkEngine* engine)
{
    SVK_LogInfo("Destroying!");
    SVK_ASSERT(engine, "svke is NULL!");

    CleanupSwapChain(engine, engine->core.device, engine->swapChain);

    vkDestroyDescriptorPool(engine->core.device, engine->core.descriptorPool, VK_NULL_HANDLE);
    vkDestroyDescriptorSetLayout(engine->core.device, engine->core.descriptorSetLayout, VK_NULL_HANDLE);

    svkScene_Destroy(&engine->core, engine->scene);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        // TODO: Check if renderer is valid before accessing
        vkDestroySemaphore(engine->core.device, engine->core.renderer.renderFinishedSemaphores[i], VK_NULL_HANDLE);
        vkDestroySemaphore(engine->core.device, engine->core.renderer.imageAvailableSemaphores[i], VK_NULL_HANDLE);
        vkDestroyFence(engine->core.device, engine->core.renderer.inFlightFences[i], VK_NULL_HANDLE);
    }

    vkDestroyQueryPool(engine->core.device, engine->core.timeQueryPool, VK_NULL_HANDLE);
    vkDestroyCommandPool(engine->core.device, engine->core.commandPool, VK_NULL_HANDLE);
    vkDestroyPipeline(engine->core.device, engine->core.graphicsPipeline, VK_NULL_HANDLE);
    vkDestroyPipelineLayout(engine->core.device, engine->core.pipelineLayout, VK_NULL_HANDLE);
    vkDestroyRenderPass(engine->core.device, engine->core.renderPass, VK_NULL_HANDLE);

    for (size_t i = 0; i < engine->core.shaders->size; i++)
        vkDestroyShaderModule(engine->core.device, ((svkShader*)engine->core.shaders->data[i])->shader, VK_NULL_HANDLE);

    vkDestroySurfaceKHR(engine->core.instance, engine->core.surface, VK_NULL_HANDLE);
    vkDestroyDevice(engine->core.device, VK_NULL_HANDLE);

    if (VALIDATION_LAYER_ENABLED)
        DestroyDebugUtilsMessengerEXT(engine->core.instance, engine->debugMessenger, VK_NULL_HANDLE);

    vkDestroyInstance(engine->core.instance, VK_NULL_HANDLE);

    SVK_FREE(engine->core.commandBuffers);
    SVK_FREE(engine->core.renderer.imageAvailableSemaphores);
    SVK_FREE(engine->core.renderer.renderFinishedSemaphores);
    SVK_FREE(engine->core.renderer.inFlightFences);

    svkVector_Free(engine->core.shaders);
    svkVector_Free(engine->swapChain->images);
    svkVector_Free(engine->swapChain->imageViews);

    SVK_FREE(engine->swapChain);
    SVK_FREE(engine);
}