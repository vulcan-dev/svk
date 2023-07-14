#include "svk/svk_engine.h"

internal svkSwapChainSupportDetails QuerySwapChainSupport(
    const VkPhysicalDevice physicalDevice,
    const VkSurfaceKHR surface)
{
    svkSwapChainSupportDetails details = SVK_ZMSTRUCT2(svkSwapChainSupportDetails);
    details.formats = svkVector_Create(0, sizeof(VkSurfaceFormatKHR));
    details.presentModes = svkVector_Create(0, sizeof(VkPresentModeKHR));

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);

    // Get surface formats
    uint32_t formatCount = 0;
    VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, NULL);
    if (result != VK_SUCCESS)
        SDL_Log("Got bad result from vkGetPhysicalDeviceSurfaceFormatsKHR: %d", result);
    
    if (formatCount != 0)
    {
        svkVector_Resize(details.formats, formatCount);
        VkSurfaceFormatKHR* surfaceFormats = SVK_ZMSTRUCT(VkSurfaceFormatKHR, formatCount);
        result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, surfaceFormats);
        if (result != VK_SUCCESS)
            SDL_Log("Got bad result from vkGetPhysicalDeviceSurfaceFormatsKHR: %d", result);

        for (u32 i = 0; i < formatCount; i++)
            svkVector_PushBackCopy(details.formats, &surfaceFormats[i], sizeof(VkSurfaceFormatKHR));
        SVK_FREE(surfaceFormats);
    }
    
    // Get present modes
    uint32_t presentModeCount = 0;
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, NULL);
    if (result != VK_SUCCESS)
        SDL_Log("Got bad result from vkGetPhysicalDeviceSurfacePresentModesKHR: %d", result);
    
    if (presentModeCount != 0)
    {
        svkVector_Resize(details.presentModes, presentModeCount);
        VkPresentModeKHR* presentModes = SVK_ZMSTRUCT(VkPresentModeKHR, presentModeCount);
        result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes);
        if (result != VK_SUCCESS)
            SDL_Log("Got bad result from vkGetPhysicalDeviceSurfacePresentModesKHR: %d", result);

        for (u32 i = 0; i < presentModeCount; i++)
            svkVector_PushBackCopy(details.presentModes, &presentModes[i], sizeof(VkPresentModeKHR));
        SVK_FREE(presentModes);
    }

    return details;
}

internal VkSurfaceFormatKHR ChooseSwapSurfaceFormat(
    const SVKVECTOR_TYPE(VkSurfaceFormatKHR) availableFormats,
    VkResult* outResult)
{
    *outResult = VK_SUCCESS;
    for (u32 i = 0; i < availableFormats->size; i++)
    {
        const VkSurfaceFormatKHR availableFormat = *(VkSurfaceFormatKHR*)availableFormats->data[i];
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return availableFormat;
    }
    
    VkSurfaceFormatKHR format = *(VkSurfaceFormatKHR*)availableFormats->data[0];
    if (format.format == VK_FORMAT_UNDEFINED)
    {
        *outResult = VK_ERROR_SURFACE_LOST_KHR;
        return (VkSurfaceFormatKHR){ VK_FORMAT_UNDEFINED, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
    }

    return format;
}

internal VkPresentModeKHR ChooseSwapPresentMode(const SVKVECTOR_TYPE(VkPresentModeKHR) availablePresentModes)
{
    for (u32 i = 0; i < availablePresentModes->size; i++)
    {
        VkPresentModeKHR availablePresentMode = *(VkPresentModeKHR*)availablePresentModes->data[i];
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            return availablePresentMode;
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

internal VkExtent2D ChooseSwapExtent(
    const VkSurfaceCapabilitiesKHR* capabilities,
    SDL_Window* window)
{
    if (capabilities->currentExtent.width != UINT32_MAX)
        return capabilities->currentExtent;

    int width, height = 0;
    SDL_Vulkan_GetDrawableSize(window, &width, &height);

    const VkExtent2D actualExtent = {
        SDL_clamp(width, capabilities->minImageExtent.width, capabilities->maxImageExtent.width),
        SDL_clamp(height, capabilities->minImageExtent.height, capabilities->maxImageExtent.height)
    };

    return actualExtent;
}

internal VkResult CreateSwapChain(
    VkSwapchainKHR* outSwapchain,
    SVKVECTOR_TYPE(VkImage)* outImages,
    VkFormat* outFormat,
    VkExtent2D* outExtent,
    const VkDevice device,
    const VkPhysicalDevice physicalDevice,
    const VkSurfaceKHR surface,
    SDL_Window* window)
{
    svkSwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(physicalDevice, surface);
    VkResult result;

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats, &result);
    if (result != VK_SUCCESS)
        return result;

    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = ChooseSwapExtent(&swapChainSupport.capabilities, window);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    const uint32_t maxImageCount = swapChainSupport.capabilities.maxImageCount;

    if (maxImageCount > 0 && imageCount > maxImageCount)
        imageCount = maxImageCount;

    // Create Info
    VkSwapchainCreateInfoKHR createInfo = SVK_ZMSTRUCT2(VkSwapchainCreateInfoKHR);
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    *outFormat = surfaceFormat.format;
    *outExtent = extent;

    svkQueueFamilyIndices indices = _svkEngine_FindQueueFamilies(physicalDevice, surface);
    uint32_t queueFamilyIndices[2] = { indices.graphicsFamily, indices.presentFamily };

    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = NULL;
    }

    svkVector_Free(swapChainSupport.formats);
    svkVector_Free(swapChainSupport.presentModes);

    result = vkCreateSwapchainKHR(device, &createInfo, NULL, outSwapchain);
    if (result != VK_SUCCESS)
        return result;

    VkImage* swapChainImages;
    vkGetSwapchainImagesKHR(device, *outSwapchain, &imageCount, NULL);
    swapChainImages = SVK_ZMSTRUCT(VkImage, imageCount);
    vkGetSwapchainImagesKHR(device, *outSwapchain, &imageCount, swapChainImages);

    *outImages = svkVector_Create(imageCount, sizeof(VkImage));
    for (size_t i = 0; i < imageCount; i++)
    {
        memcpy(&(*outImages)->data[i], &swapChainImages[i], sizeof(VkImage));
        (*outImages)->size++;
    }

    SVK_FREE(swapChainImages);

    return VK_SUCCESS;
}