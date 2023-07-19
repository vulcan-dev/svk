#include "svk/svk_engine.h"

// Internal Functions
//------------------------------------------------------------------------
VkResult _svkEngine_CreateDescriptorLayout(
    VkDevice device,
    VkDescriptorSetLayout* outLayout)
{
    VkDescriptorSetLayoutBinding uboLayoutBinding = SVK_ZMSTRUCT2(VkDescriptorSetLayoutBinding);
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo = SVK_ZMSTRUCT2(VkDescriptorSetLayoutCreateInfo);
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;
    
    return vkCreateDescriptorSetLayout(device, &layoutInfo, NULL, outLayout);
}

VkResult _svkEngine_CreateDescriptorPool(
    VkDevice device,
    VkDescriptorPool* outPool)
{
    VkDescriptorPoolSize poolSize;
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = MAX_FRAMES_IN_FLIGHT;

    VkDescriptorPoolCreateInfo poolInfo = SVK_ZMSTRUCT2(VkDescriptorPoolCreateInfo);
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = MAX_FRAMES_IN_FLIGHT;

    return vkCreateDescriptorPool(device, &poolInfo, NULL, outPool);
}

// Public Functions
//------------------------------------------------------------------------
VkResult _svkEngine_CreateDescriptorSets(
    const VkDevice device,
    const VkDescriptorSetLayout descriptorSetlayout,
    const VkDescriptorPool descriptorPool,
    SVKARRAY_TYPE(VkBuffer) uniformBuffers,
    SVKARRAY_TYPE(VkDescriptorSet)* descriptorSets)
{
    SVK_ASSERT(MAX_FRAMES_IN_FLIGHT == 2, "_svkEngine_CreateDescriptorSets has to be 2");
    VkDescriptorSetLayout* layouts = SVK_ALLOCSTRUCT(VkDescriptorSetLayout, MAX_FRAMES_IN_FLIGHT);
    layouts[0] = descriptorSetlayout;
    layouts[1] = descriptorSetlayout;

    VkDescriptorSetAllocateInfo allocInfo = SVK_ZMSTRUCT2(VkDescriptorSetAllocateInfo);
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
    allocInfo.pSetLayouts = layouts;

    *descriptorSets = SVK_ALLOCSTRUCT(VkDescriptorSet, MAX_FRAMES_IN_FLIGHT);
    VkResult result = vkAllocateDescriptorSets(device, &allocInfo, *descriptorSets);
    if (result != VK_SUCCESS)
        return result;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        SVK_ASSERT(uniformBuffers[i], "uniformBuffers was NULL at unknown index");

        VkDescriptorBufferInfo bufferInfo;
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(svkUniformBufferObj);

        VkWriteDescriptorSet descriptorWrite = SVK_ZMSTRUCT2(VkWriteDescriptorSet);
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = (*descriptorSets)[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, NULL);
    }

    return VK_SUCCESS;
}