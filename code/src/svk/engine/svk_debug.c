#include "svk/svk_engine.h"

internal VKAPI_ATTR
VkBool32 VKAPI_CALL
__DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
    void* pUserData) {

    SVK_LogDebug("[VL] %s", callbackData->pMessage);

    return VK_FALSE;
}

internal void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT* createInfo) {
    SVK_ZM(*createInfo, sizeof(*createInfo));
    createInfo->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo->pfnUserCallback = __DebugCallback;
}

internal VkResult
CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* createInfo, const VkAllocationCallbacks* allocator, VkDebugUtilsMessengerEXT* debugMessenger) {
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != NULL) {
        return func(instance, createInfo, allocator, debugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

internal void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* allocator) {
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func == NULL)
        return; 
       
    func(instance, debugMessenger, allocator);
}

internal VkResult SetupDebugMessenger(VkInstance* instance, VkDebugUtilsMessengerEXT* debugMessenger)
{
    VkDebugUtilsMessengerCreateInfoEXT createInfo = SVK_ZMSTRUCT2(VkDebugUtilsMessengerCreateInfoEXT);
    PopulateDebugMessengerCreateInfo(&createInfo);

    return CreateDebugUtilsMessengerEXT(*instance, &createInfo, NULL, debugMessenger);
}

internal VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
    {
        SVK_LogDebug("[Vulkan - %s] %s", messageType, pCallbackData->pMessageIdName, pCallbackData->pMessage);
    }
    else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
    {
        SVK_LogInfo("[Vulkan - %s] %s", pCallbackData->pMessageIdName, pCallbackData->pMessage);
    }
    else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        SVK_LogWarn("[Vulkan - %s] %s", pCallbackData->pMessageIdName, pCallbackData->pMessage);
    }
    else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        SVK_LogError("[Vulkan - %s] %s", pCallbackData->pMessageIdName, pCallbackData->pMessage);
    }

    return VK_FALSE;
}

static VkDebugUtilsMessengerEXT callback;
internal VkResult SetupDebugCallback(VkInstance instance)
{
    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

    VkDebugUtilsMessengerCreateInfoEXT createInfo = SVK_ZMSTRUCT2(VkDebugUtilsMessengerCreateInfoEXT);
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT    |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;

    return vkCreateDebugUtilsMessengerEXT(instance, &createInfo, VK_NULL_HANDLE, &callback);
}

internal void DestroyDebugCallback(VkInstance instance)
{
    PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

    vkDestroyDebugUtilsMessengerEXT(instance, callback, VK_NULL_HANDLE);
}