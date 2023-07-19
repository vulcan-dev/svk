#include "svk/svk_window.h"
#include "svk/engine/svk_renderer.h"

// Internal Function Prototypes
//------------------------------------------------------------------------
internal void _svkWindow_HandleEvents(svkWindow* svkw, SDL_Event event);

// SVKWindow Functions
//------------------------------------------------------------------------
svkWindow* svkWindow_Create(svkEngine* svke, const char* title, svkVec2 size, svkVec2 position, u32 flags)
{
    SVK_LogInfo("Creating window...");
    svkWindow* svkw = SVK_ZMSTRUCT(svkWindow, 1);
    svkw->title = title;

    svkw->window = SDL_CreateWindow(title, size.x, size.y, position.x, position.y, flags);
    if (!svkw->window)
    {
        lastErrorCode = SVK_ERROR_CREATE_WINDOW;
        return NULL;
    }

    svkw->engine = svke;

    const VkResult result = _svkEngine_Initialize(svke, svkw->window);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to initialize, error code: %d\n", result);
        return NULL;
    }

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

    _svkEngine_DrawFrame(svkw->window, svke);

    char newTitle[256];
    sprintf_s(newTitle, 256, "%s (RenderTime: %fms)", svkw->title, svke->debug.gpuTime);
    SDL_SetWindowTitle(svkw->window, newTitle);

    return true;
}

void svkWindow_Destroy(svkWindow* svkw)
{
    SVK_LogInfo("Destroying window");

    if (!svkw)
        return;

    vkDeviceWaitIdle(svkw->engine->core.device);

    SDL_DestroyWindow(svkw->window);
    SDL_Quit();

    SVK_FREE(svkw);
}

// Internal Functions
//------------------------------------------------------------------------
int mouseX, mouseY;
internal void _svkWindow_HandleEvents(svkWindow* svkw, SDL_Event event)
{
    int mx, my = 0;
    switch (event.type)
    {
        case SDL_QUIT:
            svkw->shouldClose = true;
            break;
        case SDL_MOUSEMOTION:
        {
            //SDL_GetMouseState(&mx, &my);
        }
    }
}