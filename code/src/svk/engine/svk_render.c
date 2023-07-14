#include "svk/svk_engine.h"

internal VkResult CreateImageViews(
    const SVKVECTOR_TYPE(VkImage) swapChainImages,
    SVKVECTOR_TYPE(VkImageView)* outSwapChainImageViews,
    const VkDevice device,
    const VkFormat imageFormat)
{
    *outSwapChainImageViews = svkVector_Create(swapChainImages->size, sizeof(VkImageView));
    VkImageView* tempImages = SVK_ALLOCSTRUCT(VkImageView, swapChainImages->size);

    VkResult result = VK_SUCCESS;
    for (size_t i = 0; i < swapChainImages->size; i++)
    {
        VkImageViewCreateInfo createInfo = SVK_ZMSTRUCT2(VkImageViewCreateInfo);
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

        createInfo.image = (VkImage)swapChainImages->data[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = imageFormat;

        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        result = vkCreateImageView(device, &createInfo, NULL, &tempImages[i]);

        // Copy to vector
        memcpy(&(*outSwapChainImageViews)->data[i], &tempImages[i], sizeof(VkImageView));
        (*outSwapChainImageViews)->size++;
        
        if (result != VK_SUCCESS)
        {
            SVK_FREE(tempImages);
            return result;
        }
    }

    SVK_FREE(tempImages);
    return result;
}

internal VkResult CreateGraphicsPipeline(
    const VkDevice device,
    const SVKVECTOR_TYPE(svkShader) shaders,
    const VkExtent2D swapChainExtent,
    const VkRenderPass renderPass,
    VkPipeline* outPipeline,
    VkPipelineLayout* outPipelineLayout)
{
    VkDynamicState dynamicStates[2] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    // Dynamic State
    VkPipelineDynamicStateCreateInfo dynamicState = SVK_ZMSTRUCT2(VkPipelineDynamicStateCreateInfo);
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = SVK_ARRAY_SIZE(dynamicStates);
    dynamicState.pDynamicStates = dynamicStates;

    // Vertex Input
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = SVK_ZMSTRUCT2(VkPipelineVertexInputStateCreateInfo);
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;

    // Input Assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = SVK_ZMSTRUCT2(VkPipelineInputAssemblyStateCreateInfo);
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Viewports & Scissors
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0;
    viewport.maxDepth = 0;

    VkRect2D scissor;
    scissor.offset = (VkOffset2D){ 0, 0 };
    scissor.extent = swapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState = SVK_ZMSTRUCT2(VkPipelineViewportStateCreateInfo);
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer = SVK_ZMSTRUCT2(VkPipelineRasterizationStateCreateInfo);
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    // Multisampling
    VkPipelineMultisampleStateCreateInfo multisampling = SVK_ZMSTRUCT2(VkPipelineMultisampleStateCreateInfo);
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // Color Blending
    VkPipelineColorBlendAttachmentState colorBlendAttachment = SVK_ZMSTRUCT2(VkPipelineColorBlendAttachmentState);
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending = SVK_ZMSTRUCT2(VkPipelineColorBlendStateCreateInfo);
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    // Pipeline Layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = SVK_ZMSTRUCT2(VkPipelineLayoutCreateInfo);
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL, outPipelineLayout);
    if (result != VK_SUCCESS)
        return result;

    // Pipeline Info
    VkPipelineShaderStageCreateInfo* shaderStages = SVK_ZMSTRUCT(VkPipelineShaderStageCreateInfo, shaders->size);
    for (size_t i = 0; i < shaders->size; i++)
        shaderStages[i] = ((svkShader*)shaders->data[i])->stageInfo;

    VkGraphicsPipelineCreateInfo pipelineInfo = SVK_ZMSTRUCT2(VkGraphicsPipelineCreateInfo);
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = *outPipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineIndex = -1;

    SVK_FREE(shaderStages); // maybe free after I use `pipelineInfo`, it might make a copy.

    // Pipeline
    result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, outPipeline);
    if (result != VK_SUCCESS)
        return result;

    return VK_SUCCESS;
}

internal VkResult CreateRenderPass(
    const VkDevice device,
    const VkFormat swapChainImageFormat,
    VkRenderPass* outRenderPass)
{
    VkAttachmentDescription colorAttachment;
    colorAttachment.flags = 0;
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // Subpasses and Attachment References
    VkAttachmentReference colorAttachmentRef = SVK_ZMSTRUCT2(VkAttachmentReference);
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = SVK_ZMSTRUCT2(VkSubpassDescription);
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    // Subpass Dependencies
    VkSubpassDependency dependency = SVK_ZMSTRUCT2(VkSubpassDependency);
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // Render Pass
    VkRenderPassCreateInfo renderPassInfo = SVK_ZMSTRUCT2(VkRenderPassCreateInfo);
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    return vkCreateRenderPass(device, &renderPassInfo, NULL, outRenderPass);
}

internal VkResult CreateFrameBuffers(
    const VkDevice device,
    const VkRenderPass renderPass,
    const VkExtent2D swapChainExtent,
    const SVKVECTOR_TYPE(VkImageView) swapChainImageViews,
    SVKVECTOR_TYPE(VkFramebuffer)* outSwapChainBuffers)
{
    VkResult result = VK_SUCCESS;

    *outSwapChainBuffers = svkVector_Create(swapChainImageViews->size, sizeof(VkFramebuffer));
    VkFramebuffer* tempFramebuffers = SVK_ALLOCSTRUCT(VkFramebuffer, swapChainImageViews->size);

    for (size_t i = 0; i < swapChainImageViews->size; i++)
    {
        VkImageView attachments[] = {
            (VkImageView)swapChainImageViews->data[i]
        };

        VkFramebufferCreateInfo frameBufferInfo = SVK_ZMSTRUCT2(VkFramebufferCreateInfo);
        frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frameBufferInfo.renderPass = renderPass;
        frameBufferInfo.attachmentCount = 1;
        frameBufferInfo.pAttachments = attachments;
        frameBufferInfo.width = swapChainExtent.width;
        frameBufferInfo.height = swapChainExtent.height;
        frameBufferInfo.layers = 1;

        result = vkCreateFramebuffer(device, &frameBufferInfo, NULL, &tempFramebuffers[i]);
        if (result != VK_SUCCESS)
        {
            SVK_FREE(tempFramebuffers);
            return result;
        }

        // Copy to vector
        memcpy(&(*outSwapChainBuffers)->data[i], &tempFramebuffers[i], sizeof(VkFramebuffer));
        (*outSwapChainBuffers)->size++;
    }

    SVK_FREE(tempFramebuffers);
    return result;
}