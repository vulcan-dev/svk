#include "svk/engine/svk_vertex.h"

VkVertexInputBindingDescription svkVertex_GetBindingDescription()
{
    VkVertexInputBindingDescription desc;
    desc.binding = 0;
    desc.stride = sizeof(svkVertex);
    desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return desc;
}

SVKARRAY_TYPE(VkVertexInputAttributeDescription) svkVertex_GetAttribDescriptions()
{
    VkVertexInputAttributeDescription* descs = SVK_ALLOCSTRUCT(VkVertexInputAttributeDescription, 2);

    descs[0].binding = 0;
    descs[0].location = 0;
    descs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    descs[0].offset = offsetof(svkVertex, pos);

    descs[1].binding = 0;
    descs[1].location = 1;
    descs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    descs[1].offset = offsetof(svkVertex, color);

    return descs;
}