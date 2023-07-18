#include "svk/engine/svk_scene.h"
#include "svk/engine/svk_buffer.h"
#include "svk/svk_engine.h"

void svkScene_AddDrawable(svkEngine* svke, svkDrawable* drawable)
{
    drawable->buffers.vertexBuffer = SVK_MALLOC(sizeof(VkBuffer));

    SVK_ASSERT(svke->scene, "scene is NULL!");
    if (_svkEngine_CreateVertexBuffer(
        svke->core.physicalDevice,
        drawable->vertices,
        &svke->core.device,
        &drawable->buffers.vertexMemory,
        &drawable->buffers.vertexBuffer,
        &svke->core.commandPool,
        &svke->core.queues.graphics) != VK_SUCCESS)
    {
        fprintf(stderr, "_svkEngine_CreateVertexBuffer failed!\n");
        return;
    }

    SVK_LogDebug("Added Drawable with %d vertices and %d indices", drawable->vertices->size, drawable->indices->size);

    if (drawable->indices->size == 0)
    {
        svkVector_PushBack(svke->scene->drawables, (void*)drawable);
        return;
    }

    drawable->buffers.indexBuffer = SVK_MALLOC(sizeof(VkBuffer));

    if (_svkEngine_CreateIndexBuffer(
        svke->core.physicalDevice,
        drawable->indices,
        &svke->core.device,
        &drawable->buffers.indexMemory,
        &drawable->buffers.indexBuffer,
        &svke->core.commandPool,
        &svke->core.queues.graphics) != VK_SUCCESS)
    {
        fprintf(stderr, "_svkEngine_CreateIndexBuffer failed!\n");
        return;
    }

    svkVector_PushBack(svke->scene->drawables, (void*)drawable);
}