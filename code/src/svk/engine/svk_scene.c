#include "svk/engine/svk_scene.h"
#include "svk/engine/svk_buffer.h"
#include "svk/svk_engine.h"

void svkScene_AddDrawable(svkEngine* svke, svkDrawable* drawable)
{
    SVK_ASSERT(svke->scene, "scene is NULL!");
    svkVector_PushBack(svke->scene->drawables, (void*)drawable);
    if (_svkEngine_CreateVertexBuffer(
        svke->core.physicalDevice,
        drawable->vertices,
        &svke->core.device,
        &svke->core.vertexBufferMemory,
        &svke->core.vertexBuffer,
        &svke->core.commandPool,
        &svke->core.queues.graphics) != VK_SUCCESS)
    {
        fprintf(stderr, "_svkEngine_CreateVertexBuffer failed!\n");
    }

    SVK_LogDebug("Added Drawable with %d vertices and %d indices\n", drawable->vertices->size, drawable->indices->size);

    if (drawable->indices->size == 0)
        return;

    if (_svkEngine_CreateIndexBuffer(
        svke->core.physicalDevice,
        drawable->indices,
        &svke->core.device,
        &svke->core.indexBufferMemory,
        &svke->core.indexBuffer,
        &svke->core.commandPool,
        &svke->core.queues.graphics) != VK_SUCCESS)
    {
        fprintf(stderr, "_svkEngine_CreateIndexBuffer failed!\n");
    }
}