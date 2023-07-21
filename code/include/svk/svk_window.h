#ifndef SVK_WINDOW_H
#define SVK_WINDOW_H

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include "svk/svk.h"

typedef struct svkEngine svkEngine;
typedef SDL_Event svkEvent;

typedef struct svkWindow
{
    SDL_Window* window;
    svkVec2 size;

    svkEngine* engine;
    svkInputManager inputManager;

    const char* title;
    bool shouldClose;
} svkWindow;

// Functions
//------------------------------------------------------------------------
svkWindow* svkWindow_Create(
    svkEngine*  engine,
    const char* title,
    svkVec2     size,
    svkVec2     position,
    u32         flags);

void svkWindow_SetCallback(
    svkWindow*  window,
    u32         callbackType,
    void*       cbfn,
    void*       userData);

bool svkWindow_Update(
    svkWindow*  window,
    SDL_Event*  event);

void svkWindow_LockMouse(SDL_Window* window, const bool lock);
void svkWindow_Destroy(svkWindow* window);

#endif