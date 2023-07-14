#include "svk/svk_window.h"
#include "svk/engine/svk_renderer.h"

// Internal Function Prototypes
//------------------------------------------------------------------------
internal void _svkWindow_HandleEvents(svkWindow* svkw, SDL_Event event);

// SVKWindow Functions
//------------------------------------------------------------------------
svkWindow* svkWindow_Create(svkEngine* svke, const char* title, svkVec2 size, svkVec2 position, u32 flags)
{
    svkWindow* svkw = SVK_ZMSTRUCT(svkWindow, 1);

    svkw->window = SDL_CreateWindow(title, size.x, size.y, position.x, position.y, flags);
    if (!svkw->window)
    {
        lastErrorCode = SVK_ERROR_CREATE_WINDOW;
        return NULL;
    }

    svkw->engine = svke;

    if (!_svkEngine_Initialize(svke, svkw->window))
        return NULL;

    return svkw;
}

void svkWindow_SetCallback(svkWindow* svkw, u32 callbackType, void* cbfn, void* userData)
{}

bool svkWindow_Update(svkWindow* svkw, SDL_Event* event)
{
    if (svkw->shouldClose)
        return false;

    while (SDL_PollEvent(event))
        _svkWindow_HandleEvents(svkw, *event);

    svkEngine* svke = svkw->engine;

    _svkEngine_DrawFrame(
        svkw->window,
        svke->core.physicalDevice,
        svke->core.surface,
        svke->core.device,
        svke->core.commandBuffers,
        svke->core.graphicsPipeline,
        svke->core.renderPass,
        svke->swapChain,
        svke->core.queues,
        &svke->core.renderer);
    return true;
}

void svkWindow_Destroy(svkWindow* svkw)
{
    if (!svkw)
        return;

    vkDeviceWaitIdle(svkw->engine->core.device);

    SDL_DestroyWindow(svkw->window);
    SDL_Quit();
}

// Internal Functions
//------------------------------------------------------------------------
internal void _svkWindow_HandleEvents(svkWindow* svkw, SDL_Event event)
{
    switch (event.type)
    {
        case SDL_QUIT:
            svkw->shouldClose = true;
            break;
    }
}