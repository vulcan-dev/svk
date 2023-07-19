#ifndef SVK_DESCRIPTOR_H
#define SVK_DESCRIPTOR_H

#include "svk/svk_engine.h"

VkResult _svkEngine_CreateDescriptorPool(
    VkDevice device,
    VkDescriptorPool* outPool);

VkResult _svkEngine_CreateDescriptorLayout(
    VkDevice device,
    VkDescriptorSetLayout* outLayout);

VkResult _svkEngine_CreateDescriptorSets(
    const VkDevice device,
    const VkDescriptorSetLayout descriptorSetlayout,
    const VkDescriptorPool descriptorPool,
    SVKARRAY_TYPE(VkBuffer) uniformBuffers,
    SVKARRAY_TYPE(VkDescriptorSet)* descriptorSets);

#endif