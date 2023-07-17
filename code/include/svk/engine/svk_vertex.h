#ifndef SVK_VERTEX_H
#define SVK_VERTEX_H

#include "svk/svk_engine.h"

VkVertexInputBindingDescription svkVertex_GetBindingDescription();
SVKARRAY_TYPE(VkVertexInputAttributeDescription) svkVertex_GetAttribDescriptions();

#endif