#include "svk/svk.h"

int main(void)
{
    svkEngine* svke = svkEngine_Create("SVK Engine", VK_MAKE_API_VERSION(0, 1, 0, 0));
    svkWindow* svkw = svkWindow_Create(svke, "Vulkan Engine",
        (svkVec2){ SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED },
        (svkVec2){ 800, 600 },
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN | SDL_WINDOW_HIDDEN
    );

    if (svkw == NULL)
    {
        printf("Something went wrong! Last Error: %#012x\n", SVK_GetLastError());
        svkWindow_Destroy(svkw);
        svkEngine_Destroy(svke);
        return -1;
    }

    SDL_ShowWindow(svkw->window);

    svkEvent event;
    while (svkWindow_Update(svkw, &event))
    {
        
    }

    svkWindow_Destroy(svkw);
    svkEngine_Destroy(svke);

    return 0;
}