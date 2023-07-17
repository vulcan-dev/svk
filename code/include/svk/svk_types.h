#ifndef SVK_TYPES_H
#define SVK_TYPES_H

#include <vulkan/vulkan.h>

typedef struct svkVec2
{
    float x;
    float y;
} svkVec2;

typedef struct svkVec3
{
    float x;
    float y;
    float z;
} svkVec3;

// Vertex
//------------------------------------------------------------------------
typedef struct svkVertex
{
    svkVec2 pos;
    svkVec3 color;
} svkVertex;

//inline VkVertexInputBindingDescription svkVertex_GetBindingDescription()
//{
//    VkVertexInputBindingDescription desc;
//    desc.binding = 0;
//    desc.stride = sizeof(svkVertex);
//    desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
//    return desc;
//}
//
//inline VkVertexInputAttributeDescription* svkVertex_GetAttribDescriptions()
//{
//    VkVertexInputAttributeDescription* descs = (VkVertexInputAttributeDescription*)malloc(sizeof(VkVertexInputAttributeDescription) * 2);
//
//    descs[0].binding = 0;
//    descs[0].location = 0;
//    descs[0].format = VK_FORMAT_R32G32_SFLOAT;
//    descs[0].offset = offsetof(svkVertex, pos);
//
//    descs[1].binding = 0;
//    descs[1].location = 1;
//    descs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
//    descs[1].offset = offsetof(svkVertex, color);
//
//    return descs;
//}

#endif