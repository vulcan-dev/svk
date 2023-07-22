#include "svk/engine/svk_math.h"
#include "svk/engine/svk_scene.h"
#include "svk/engine/svk_buffer.h"
#include "svk/engine/svk_descriptor.h"
#include "svk/svk_engine.h"

void svkScene_Initialize(svkWindow* window, svkEngine* engine)
{
    _svkEngineScene* scene = engine->scene;
    scene->drawables = svkVector_Create(0, sizeof(svkDrawable));

    scene->camera = SVK_ZMSTRUCT(svkCamera, 1);
    svkCamera* camera = scene->camera;
    camera->firstMouse = true;
    camera->mouseSensitivity = 0.05f;
    camera->yaw = -90.0f;
    camera->pitch = 0.0f;
    camera->speed = 0.003f;
    camera->nearClip = 0.1f;
    camera->farClip = 100.0f;
    camera->aspectRatio = (float)engine->swapChain->extent.width / (float)engine->swapChain->extent.height;
    camera->pos[0] = 0.0f;
    camera->pos[1] = 2.0f;
    camera->pos[2] = 0.0f;
    camera->inputLocked = true;
    camera->mouseEnabled = false;

    svkWindow_LockMouse(window->window, camera->mouseEnabled);
}

void svkScene_PostRender(svkEngine* engine)
{
    svkCamera* camera = engine->scene->camera;

    for (size_t i = 0; i < engine->scene->drawables->size; i++)
    {
        svkDrawable* drawable = (svkDrawable*)engine->scene->drawables->data[i];

        svkUniformBufferObj ubo;
        memcpy(&ubo, drawable->buffers->mappedBuffers[engine->core.currentFrame], sizeof(ubo));

        glm_mat4_identity(ubo.model);
        memcpy(ubo.view, camera->view, sizeof(ubo.view));
        memcpy(ubo.proj, camera->projection, sizeof(ubo.proj));

        memcpy(drawable->buffers->mappedBuffers[engine->core.currentFrame], &ubo, sizeof(ubo));
    }
}

void svkScene_RotateObject(
    svkDrawable* drawable,
    const vec3 rotation,
    const uint32_t frame)
{
    svkUniformBufferObj ubo;

    memcpy(&ubo, drawable->buffers->mappedBuffers[frame], sizeof(ubo));

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

    glm_translate(ubo.model, position);

    memcpy(drawable->buffers->mappedBuffers[frame], &ubo, sizeof(ubo));
}

void svkScene_AddDrawable(svkEngine* engine, svkDrawable* drawable)
{
    drawable->buffers = SVK_ALLOCSTRUCT(_svkEngineDrawableBuffers, 1);
    SVK_ZM(drawable->buffers->indexBuffer, sizeof(VkBuffer));

    _svkEngine_CreateUniformBuffers(engine->core.physicalDevice, &engine->core.device, &drawable->buffers->uniformBuffers, &drawable->buffers->uniformBuffersMemory, &drawable->buffers->mappedBuffers);
    _svkEngine_CreateDescriptorSets(engine->core.device, engine->core.descriptorSetLayout, engine->core.descriptorPool, drawable->buffers->uniformBuffers, &drawable->descriptorSets);

    svkUniformBufferObj ubo;

    svkCamera* camera = engine->scene->camera;

    glm_mat4_identity(ubo.model);
    glm_rotate(ubo.model, 0, (vec3){0.0f, 0.0f, 1.0f});
    memcpy(ubo.view, camera->view, sizeof(ubo.view));
    memcpy(ubo.proj, camera->projection, sizeof(ubo.proj));

    memcpy(drawable->buffers->mappedBuffers[0], &ubo, sizeof(ubo));
    memcpy(drawable->buffers->mappedBuffers[1], &ubo, sizeof(ubo));

    drawable->buffers->vertexBuffer = SVK_MALLOC(sizeof(VkBuffer));

    SVK_ASSERT(engine->scene, "scene is NULL!");
    if (_svkEngine_CreateVertexBuffer(
        engine->core.physicalDevice,
        drawable->vertices,
        &engine->core.device,
        &drawable->buffers->vertexMemory,
        &drawable->buffers->vertexBuffer,
        &engine->core.commandPool,
        &engine->core.queues.graphics) != VK_SUCCESS)
    {
        fprintf(stderr, "_svkEngine_CreateVertexBuffer failed!\n");
        return;
    }

    SVK_LogDebug("Added Drawable with %d vertices and %d indices", drawable->vertices->size, drawable->indices->size);

    if (drawable->indices->size == 0)
    {
        svkVector_PushBack(engine->scene->drawables, (void*)drawable);
        return;
    }

    drawable->buffers->indexBuffer = SVK_MALLOC(sizeof(VkBuffer));

    if (_svkEngine_CreateIndexBuffer(
        engine->core.physicalDevice,
        drawable->indices,
        &engine->core.device,
        &drawable->buffers->indexMemory,
        &drawable->buffers->indexBuffer,
        &engine->core.commandPool,
        &engine->core.queues.graphics) != VK_SUCCESS)
    {
        fprintf(stderr, "_svkEngine_CreateIndexBuffer failed!\n");
        return;
    }

    svkVector_PushBack(engine->scene->drawables, (void*)drawable);
}

void svkScene_Destroy(struct _svkEngineCore* core, struct _svkEngineScene* scene)
{
    if (!scene)
        return;

    if (!scene->drawables)
    {
        SVK_FREE(scene);
        return;
    }

    if (scene->camera)
        SVK_FREE(scene->camera);

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

void svkScene_Render(svkEngine* engine, VkCommandBuffer commandBuffer)
{
    struct _svkEngineScene* scene = engine->scene;
    struct _svkEngineCore* core = &engine->core;

    VkDeviceSize offsets[] = { 0 };

    for (size_t i = 0; i < scene->drawables->size; i++)
    {
        svkDrawable* drawable = (svkDrawable*)scene->drawables->data[i];
        VkBuffer buffer[1] = { drawable->buffers->vertexBuffer };
        vkCmdBindVertexBuffers(commandBuffer, i, 1, buffer, offsets);

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