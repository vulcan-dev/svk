#include "svk/svk.h"
#include <time.h>

const svkVertex triangleVertices[3] =
{
    {{  0.0f,  -0.15f }, { 0.99f, 0.02f, 0.99f }},
    {{  0.15f,  0.15f }, { 0.99f, 0.99f, 0.02f }},
    {{ -0.15f,  0.15f }, { 0.02f, 0.99f, 0.99f }}
};

const svkVertex triangleVertices2[3] =
{
    {{  0.7f, -0.7f }, { 0.02f, 0.99f, 0.99f }},
    {{  0.7f,  0.7f }, { 0.99f, 0.99f, 0.02f }},
    {{ -0.7f,  0.7f }, { 0.99f, 0.02f, 0.99f }}
};

const svkVertex rectVertices[4] = {
    {{ -0.5f, -0.5f }, { 0.99f, 0.02f, 0.99f }},
    {{  0.5f, -0.5f }, { 0.02f, 0.56f, 0.99f }},
    {{  0.5f,  0.5f }, { 0.02f, 0.99f, 0.99f }},
    {{ -0.5f,  0.5f }, { 0.99f, 0.99f, 0.02f }}
};

const svkVertex rectVertices2[4] = {
    {{ -0.25f, -0.25f }, { 0.02f, 0.99f, 0.99f }},
    {{  0.25f, -0.25f }, { 0.99f, 0.99f, 0.02f }},
    {{  0.25f,  0.25f }, { 0.99f, 0.02f, 0.99f }},
    {{ -0.25f,  0.25f }, { 0.02f, 0.56f, 0.99f }}
};

const uint16_t rectIndices[6] = { 0, 1, 2, 2, 3, 0 };

int main(void)
{
    // Birth
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

    srand((unsigned int)time(NULL));
    
    svkDrawable* drawable = svkDrawable_Create(rectVertices, 4, rectIndices, 6);
    svkDrawable* drawable2 = svkDrawable_Create(rectVertices2, 4, rectIndices, 6);
    svkDrawable* drawable3 = svkDrawable_Create(triangleVertices, 3, NULL, 0);
    svkScene_AddDrawable(svke, drawable);
    svkScene_AddDrawable(svke, drawable2);
    svkScene_AddDrawable(svke, drawable3);

    // Living life
    SDL_ShowWindow(svkw->window);

    svkEvent event;
    while (svkWindow_Update(svkw, &event))
    {

    }

    // Sad death
    svkWindow_Destroy(svkw);
    svkEngine_Destroy(svke);

    return 0;
}