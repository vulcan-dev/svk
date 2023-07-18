#ifndef SVK_SCENE_H
#define SVK_SCENE_H

typedef struct _svkEngineScene _svkEngineScene;
typedef struct svkDrawable svkDrawable;
typedef struct svkEngine svkEngine;

void svkScene_AddDrawable(svkEngine* svke, svkDrawable* drawable);

#endif