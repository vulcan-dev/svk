#include "svk/svk_window.h"
#include "svk/engine/svk_renderer.h"
#include "svk/util/svk_logger.h"

// Internal Function
//------------------------------------------------------------------------
internal void _svkWindow_HandleEvents(svkWindow* window, SDL_Event event);

void SDL_LogCallback(void *userdata, int category, SDL_LogPriority priority, const char *message)
{
    char msgInfo[10];
    sprintf_s(msgInfo, 10, "[SDL (%d)]", category);

    switch (priority)
    {
        case SDL_LOG_PRIORITY_DEBUG:
            SVK_LogDebug("%s: %s", msgInfo, message);
            break;
        case SDL_LOG_PRIORITY_INFO:
            SVK_LogInfo("%s: %s", msgInfo, message);
            break;
        case SDL_LOG_PRIORITY_WARN:
            SVK_LogWarn("%s: %s", msgInfo, message);
            break;
        case SDL_LOG_PRIORITY_ERROR:
            SVK_LogError("%s: %s", msgInfo, message);
            break;
        default:
            break;
    }
}

// SVKWindow Functions
//------------------------------------------------------------------------
svkWindow* svkWindow_Create(svkEngine* engine, const char* title, svkVec2 size, svkVec2 position, u32 flags)
{
    SVK_LogInfo("Creating window...");
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);
    SDL_LogSetOutputFunction(SDL_LogCallback, NULL);

    svkWindow* window = SVK_ZMSTRUCT(svkWindow, 1);
    window->title = title;
    window->size = size;
    window->inputManager = SVK_ZMSTRUCT2(svkInputManager);

    window->window = SDL_CreateWindow(title, size.x, size.y, position.x, position.y, flags);
    if (!window->window)
    {
        lastErrorCode = SVK_ERROR_CREATE_WINDOW;
        return NULL;
    }

    window->engine = engine;

    const VkResult result = _svkEngine_Initialize(engine, window->window);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to initialize, error code: %d\n", result);
        return NULL;
    }

    return window;
}

void svkWindow_Destroy(svkWindow* window)
{
    SVK_LogInfo("Destroying window");

    if (!window)
        return;

    vkDeviceWaitIdle(window->engine->core.device);

    SDL_DestroyWindow(window->window);
    SDL_Quit();

    SVK_FREE(window);
}

void svkWindow_SetCallback(svkWindow* window, u32 callbackType, void* cbfn, void* userData)
{}

bool svkWindow_Update(svkWindow* window, SDL_Event* event)
{
    if (window->shouldClose)
        return false;

    while (SDL_PollEvent(event))
    {
        if (!svkInput_Process(&window->inputManager, *event))
            _svkWindow_HandleEvents(window, *event);
    }

    svkEngine* engine = window->engine;

    if ((window->size.x > 0 && window->size.y > 0) && !(SDL_GetWindowFlags(window->window) & SDL_WINDOW_MINIMIZED))
        _svkEngine_DrawFrame(window->window, engine);

    char newTitle[256];
    sprintf_s(newTitle, 256, "%s (GPUTime: %fms)", window->title, engine->debug.gpuTime);
    SDL_SetWindowTitle(window->window, newTitle);

    return true;
}

void svkWindow_LockMouse(SDL_Window* window, const bool lock)
{
    SDL_bool locked = lock ? SDL_TRUE : SDL_FALSE;
    SDL_ShowCursor(!locked);
    SDL_SetRelativeMouseMode(locked);
    SDL_SetWindowGrab(window, locked);
    SDL_CaptureMouse(locked);
}

// Internal Functions
//------------------------------------------------------------------------
internal void _svkWindow_HandleResize(svkWindow* window, svkCamera* camera, int newWidth, int newHeight)
{
    camera->aspectRatio = (float)newWidth / (float)newHeight;
    window->size.x = newWidth;
    window->size.y = newHeight;
}

internal void _svkWindow_HandleEvents(svkWindow* window, SDL_Event event)
{
    svkEngine* engine = window->engine;
    svkCamera* camera = engine->scene->camera; // Pray that scene exists

    int mx, my = 0;
    switch (event.type)
    {
        case SDL_QUIT:
            window->shouldClose = true;
            break;
        case SDL_WINDOWEVENT:
        {
            if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                _svkWindow_HandleResize(window, camera, event.window.data1, event.window.data2);
            break;
        }
    }
}