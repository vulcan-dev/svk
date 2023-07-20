#ifndef SVK_IMAGE_H
#define SVK_IMAGE_H

VkResult _svkEngine_CreateImageView(
    const VkDevice device,
    const VkImage image,
    const VkFormat format,
    const VkImageAspectFlags aspectFlags,
    VkImageView* outImageView);

VkResult _svkEngine_CreateImage(
    const VkDevice device,
    const VkPhysicalDevice physicalDevice,
    const uint32_t width,
    const uint32_t height,
    const VkFormat format,
    const VkImageTiling tiling,
    const VkImageUsageFlags usage,
    const VkMemoryPropertyFlags properties,
    VkImage* outImage,
    VkDeviceMemory* outImageMemory);

#endif