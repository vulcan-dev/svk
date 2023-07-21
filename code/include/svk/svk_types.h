#ifndef SVK_TYPES_H
#define SVK_TYPES_H

#include <vulkan/vulkan.h>

typedef struct svkVec2
{
    float x;
    float y;
} svkVec2;

typedef struct svkVec2i
{
    int x;
    int y;
} svkVec2i;

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
    svkVec3 pos;
    svkVec3 color;
} svkVertex;

#endif