#ifndef SVK_SCENE_H
#define SVK_SCENE_H

#include "cglm/cglm.h"
#include "svk/svk_engine.h"

typedef struct _svkEngineScene _svkEngineScene;
typedef struct _svkEngineCore _svkEngineCore;
typedef struct svkDrawable svkDrawable;
typedef struct svkEngine svkEngine;
typedef struct svkWindow svkWindow;

void svkScene_RotateObject(
    svkDrawable* drawable,
    const vec3 rotation,
    const uint32_t frame);

void svkScene_MoveObject(
    svkDrawable* drawable,
    vec3 position,
    const uint32_t frame);

void svkScene_Initialize(svkWindow* window, svkEngine* engine);
void svkScene_Destroy(_svkEngineCore* core, _svkEngineScene* scene);
void svkScene_PostRender(svkEngine* engine);
void svkScene_Render(svkEngine* engine, VkCommandBuffer commandBuffer);
void svkScene_AddDrawable(svkEngine* svke, svkDrawable* drawable);

#endif