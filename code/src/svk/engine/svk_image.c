#include "svk/svk_engine.h"

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
    VkDeviceMemory* outImageMemory)
{
    VkImageCreateInfo imageInfo = SVK_ZMSTRUCT2(VkImageCreateInfo);
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result = vkCreateImage(device, &imageInfo, VK_NULL_HANDLE, outImage);
    if (result != VK_SUCCESS)
        return result;

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, *outImage, &memRequirements);

    VkMemoryAllocateInfo allocInfo = SVK_ZMSTRUCT2(VkMemoryAllocateInfo);
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = _svkEngine_FindMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

    result = vkAllocateMemory(device, &allocInfo, VK_NULL_HANDLE, outImageMemory);
    if (result != VK_SUCCESS)
        return result;

    result = vkBindImageMemory(device, *outImage, *outImageMemory, 0);
    return result;
}

VkResult _svkEngine_CreateImageView(
    const VkDevice device,
    const VkImage image,
    const VkFormat format,
    const VkImageAspectFlags aspectFlags,
    VkImageView* outImageView)
{
    VkImageViewCreateInfo viewInfo = SVK_ZMSTRUCT2(VkImageViewCreateInfo);
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    return vkCreateImageView(device, &viewInfo, NULL, outImageView);
}