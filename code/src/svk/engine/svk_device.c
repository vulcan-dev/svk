#include "svk/svk_engine.h"
#include "svk/util/svk_vector.h"

#include "svk_swapchain.c"

local const char* deviceExtensions[] = {
    "VK_KHR_swapchain"
};

internal bool CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice)
{
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &extensionCount, NULL);

    VkExtensionProperties* availableExtensions = SVK_ALLOCSTRUCT(VkExtensionProperties, extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &extensionCount, availableExtensions);

    svkVector* requiredExtensions = svkVector_Create(SVK_ARRAY_SIZE(deviceExtensions), sizeof(const char*));
    for (u32 i = 0; i < SVK_ARRAY_SIZE(deviceExtensions); i++)
        SVKVECTOR_PUSHBACK(requiredExtensions, deviceExtensions[i]);

    for (u32 i = 0; i < extensionCount; i++)
    {
        VkExtensionProperties* extension = &availableExtensions[i];
        SVKVECTOR_ERASE(requiredExtensions, extension->extensionName);
    }

    bool isEmpty = (requiredExtensions->size == 0);

    SVK_FREE(availableExtensions);
    svkVector_Free(requiredExtensions);

    return isEmpty;
}

internal bool IsDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
    vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

    bool extensionsSupported = CheckDeviceExtensionSupport(physicalDevice);
    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        svkSwapChainSupportDetails* swapChainSupport = QuerySwapChainSupport(physicalDevice, surface);
        swapChainAdequate = swapChainSupport->formats->size > 0 && swapChainSupport->presentModes->size > 0;
        svkVector_Free(swapChainSupport->formats);
        svkVector_Free(swapChainSupport->presentModes);
        SVK_FREE(swapChainSupport);
    }

    const svkQueueFamilyIndices indices = _svkEngine_FindQueueFamilies(physicalDevice, surface);
    return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader
        && indices.graphicsFamily != -1 && indices.presentFamily != -1 && swapChainAdequate;
}

internal SvkResult PickPhysicalDevice(VkInstance instance, VkPhysicalDevice* outPhysicalDevice, VkSurfaceKHR surface)
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
    if (deviceCount == 0)
        return SVK_ERROR_NO_VULKAN_SUPPORT;

    VkPhysicalDevice* devices = SVK_ZMSTRUCT(VkPhysicalDevice, deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices);

    if (deviceCount > 1)
    {
        u32 lastCore = 0;
        VkPhysicalDeviceProperties deviceProperties;
        for (u32 i = 0; i < deviceCount; i++)
        {
            VkPhysicalDevice device = devices[i];
            if (!IsDeviceSuitable(device, surface))
                continue;

            // Get device score
            vkGetPhysicalDeviceProperties(device, &deviceProperties);
            const u32 score = deviceProperties.limits.maxImageDimension3D;
            if (score > lastCore)
                *outPhysicalDevice = device;

            lastCore = score;
        }
    } else
    {
        if (&devices[0] == VK_NULL_HANDLE)
        {
            SVK_FREE(devices);
            return SVK_ERROR_NO_SUITABLE_DEVICE;
        }

        if (!IsDeviceSuitable(devices[0], surface))
        {
            SVK_FREE(devices);
            return SVK_ERROR_NO_SUITABLE_DEVICE;
        }

        *outPhysicalDevice = devices[0];
    }

    SVK_FREE(devices);
    return SVK_SUCCESS;
}

internal VkResult CreateLogicalDevice(VkPhysicalDevice physicalDevice, VkDevice* outDevice, struct _svkEngineCoreQueue* queues, VkSurfaceKHR surface, const bool enableValidationLayer, const char** validationLayers)
{
    svkQueueFamilyIndices indices = _svkEngine_FindQueueFamilies(physicalDevice, surface);

    VkDeviceQueueCreateInfo queueCreateInfos[2];
    uint32_t uniqueQueueFamilies[2] =
    {
        indices.graphicsFamily,
        indices.presentFamily
    };

    float queuePriority = 1.0f;
    for (u32 i = 0; i < 2; i++)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = SVK_ZMSTRUCT2(VkDeviceQueueCreateInfo);
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = uniqueQueueFamilies[i];
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos[i] = queueCreateInfo;
    }

    VkPhysicalDeviceFeatures deviceFeatures = SVK_ZMSTRUCT2(VkPhysicalDeviceFeatures);
    VkDeviceCreateInfo createInfo = SVK_ZMSTRUCT2(VkDeviceCreateInfo);
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos;
    createInfo.queueCreateInfoCount = SVK_ARRAY_SIZE(queueCreateInfos);
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = SVK_ARRAY_SIZE(deviceExtensions);
    createInfo.ppEnabledExtensionNames = deviceExtensions;

    if (enableValidationLayer)
    {
        createInfo.enabledLayerCount = SVK_ARRAY_SIZE(validationLayers);
        createInfo.ppEnabledLayerNames = validationLayers;
    } else
    {
        createInfo.enabledLayerCount = 0;
    }

    const VkResult result = vkCreateDevice(physicalDevice, &createInfo, NULL, outDevice);
    if (result != VK_SUCCESS)
        return result;

    vkGetDeviceQueue(*outDevice, indices.graphicsFamily, 0, &queues->graphics);
    vkGetDeviceQueue(*outDevice, indices.presentFamily, 0, &queues->present);

    return VK_SUCCESS;
}