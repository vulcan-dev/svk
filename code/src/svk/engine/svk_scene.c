#include "svk/engine/svk_scene.h"
#include "svk/engine/svk_buffer.h"
#include "svk/engine/svk_descriptor.h"
#include "svk/svk_engine.h"

#define VEC3_UP (vec3){0.0f, 0.0f, 1.0f}

void svkScene_RotateObject(
    svkDrawable* drawable,
    const vec3 rotation,
    const uint32_t frame)
{
    svkUniformBufferObj ubo;

    memcpy(&ubo, drawable->buffers->mappedBuffers[frame], sizeof(ubo));

    //glm_mat4_identity(ubo.model);
    glm_rotate(ubo.model, glm_rad(rotation[0]), (vec3){1.0f, 0.0f, 0.0f});
    glm_rotate(ubo.model, glm_rad(rotation[1]), (vec3){0.0f, 1.0f, 0.0f});
    glm_rotate(ubo.model, glm_rad(rotation[2]), (vec3){0.0f, 0.0f, 1.0f});

    memcpy(drawable->buffers->mappedBuffers[frame], &ubo, sizeof(ubo));
}

void svkScene_MoveObject(
    svkDrawable* drawable,
    vec3 position,
    const uint32_t frame)
{
    svkUniformBufferObj ubo;
    memcpy(&ubo, drawable->buffers->mappedBuffers[frame], sizeof(ubo));

    //glm_mat4_identity(ubo.model);
    glm_translate(ubo.model, position);

    memcpy(drawable->buffers->mappedBuffers[frame], &ubo, sizeof(ubo));
}

void svkScene_AddDrawable(svkEngine* svke, svkDrawable* drawable)
{
    drawable->buffers = SVK_ALLOCSTRUCT(_svkEngineDrawableBuffers, 1);

    _svkEngine_CreateUniformBuffers(svke->core.physicalDevice, &svke->core.device, &drawable->buffers->uniformBuffers, &drawable->buffers->uniformBuffersMemory, &drawable->buffers->mappedBuffers);
    _svkEngine_CreateDescriptorSets(svke->core.device, svke->core.descriptorSetLayout, svke->core.descriptorPool, drawable->buffers->uniformBuffers, &drawable->descriptorSets);

    vec3 eye = (vec3){ 2.0f, 2.0f, 2.0f };

    svkUniformBufferObj ubo;

    glm_mat4_identity(ubo.model);
    glm_rotate(ubo.model, 0, (vec3){0.0f, 0.0f, 1.0f});

    glm_mat4_identity(ubo.view);
    glm_lookat(eye, (vec3){0.0f, 0.0f, 0.0f}, VEC3_UP, ubo.view);

    glm_mat4_identity(ubo.proj);
    glm_perspective(glm_rad(45.0f), svke->swapChain->extent.width / svke->swapChain->extent.height, 0.1f, 10.0f, ubo.proj);
    ubo.proj[1][1] *= -1.0f;

    memcpy(drawable->buffers->mappedBuffers[0], &ubo, sizeof(ubo));
    memcpy(drawable->buffers->mappedBuffers[1], &ubo, sizeof(ubo));

    drawable->buffers->vertexBuffer = SVK_MALLOC(sizeof(VkBuffer));

    SVK_ASSERT(svke->scene, "scene is NULL!");
    if (_svkEngine_CreateVertexBuffer(
        svke->core.physicalDevice,
        drawable->vertices,
        &svke->core.device,
        &drawable->buffers->vertexMemory,
        &drawable->buffers->vertexBuffer,
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

    drawable->buffers->indexBuffer = SVK_MALLOC(sizeof(VkBuffer));

    if (_svkEngine_CreateIndexBuffer(
        svke->core.physicalDevice,
        drawable->indices,
        &svke->core.device,
        &drawable->buffers->indexMemory,
        &drawable->buffers->indexBuffer,
        &svke->core.commandPool,
        &svke->core.queues.graphics) != VK_SUCCESS)
    {
        fprintf(stderr, "_svkEngine_CreateIndexBuffer failed!\n");
        return;
    }

    svkVector_PushBack(svke->scene->drawables, (void*)drawable);
}

void svkScene_Destroy(struct _svkEngineCore* core, struct _svkEngineScene* scene)
{
    for (size_t i = 0; i < scene->drawables->size; i++)
    {
        svkDrawable* drawable = (svkDrawable*)scene->drawables->data[i];
        vkDestroyBuffer(core->device, drawable->buffers->vertexBuffer, VK_NULL_HANDLE);
        vkFreeMemory(core->device, drawable->buffers->vertexMemory, VK_NULL_HANDLE);

        if (drawable->buffers->mappedBuffers != NULL)
            SVK_FREE(drawable->buffers->mappedBuffers);
        
        if (drawable->buffers->uniformBuffers != NULL)
        {
            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            {
                vkDestroyBuffer(core->device, drawable->buffers->uniformBuffers[i], VK_NULL_HANDLE);
                vkFreeMemory(core->device, drawable->buffers->uniformBuffersMemory[i], VK_NULL_HANDLE);
            }

            SVK_FREE(drawable->buffers->uniformBuffers);
            SVK_FREE(drawable->buffers->uniformBuffersMemory);
        }

        if (drawable->descriptorSets != NULL)
            SVK_FREE(drawable->descriptorSets);

        if (drawable->buffers->indexBuffer)
        {
            vkDestroyBuffer(core->device, drawable->buffers->indexBuffer, VK_NULL_HANDLE);
            vkFreeMemory(core->device, drawable->buffers->indexMemory, VK_NULL_HANDLE);
        }

        SVK_FREE(drawable->buffers);
        SVK_FREE(scene->drawables->data[i]);
    }

    svkVector_Free(scene->drawables);

    SVK_FREE(scene);
}

void svkScene_PostRender(svkEngine* engine)
{
    for (size_t i = 0; i < engine->scene->drawables->size; i++)
    {
        svkDrawable* drawable = (svkDrawable*)engine->scene->drawables->data[i];

        svkUniformBufferObj ubo;
        memcpy(&ubo, drawable->buffers->mappedBuffers[engine->core.currentFrame], sizeof(ubo));
        glm_mat4_identity(ubo.model);
        memcpy(drawable->buffers->mappedBuffers[engine->core.currentFrame], &ubo, sizeof(ubo));
    }
}

void svkScene_Render(svkEngine* engine, VkCommandBuffer commandBuffer)
{
    struct _svkEngineScene* scene = engine->scene;
    struct _svkEngineCore* core = &engine->core;

    VkDeviceSize offsets[] = { 0 };

    for (size_t i = 0; i < scene->drawables->size; i++)
    {
        svkDrawable* drawable = (svkDrawable*)scene->drawables->data[i];
        VkBuffer buffer[1] = { drawable->buffers->vertexBuffer };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffer, offsets);

        if (drawable->descriptorSets)
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, core->pipelineLayout, 0, 1, &drawable->descriptorSets[core->currentFrame], 0, NULL);

        if (drawable->indices->size > 0)
        {
            vkCmdBindIndexBuffer(commandBuffer, drawable->buffers->indexBuffer, 0, VK_INDEX_TYPE_UINT16);
            vkCmdDrawIndexed(commandBuffer, drawable->indices->size, 1, 0, 0, 0);
        } else
        {
            vkCmdDraw(commandBuffer, (uint32_t)drawable->vertices->size, 1, 0, 0);
        }
    }
}