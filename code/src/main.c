#include "svk/svk.h"
#include <time.h>
#include "cglm/cglm.h"

int main(void)
{
    svkEngine* engine = svkEngine_Create("SVK Engine", VK_MAKE_API_VERSION(0, 1, 0, 0));
    svkWindow* window = svkWindow_Create(engine, "Vulkan Engine",
        (svkVec2){ SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED },
        (svkVec2){ 800, 600 },
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN | SDL_WINDOW_HIDDEN
    );

    if (window == NULL)
    {
        printf("Something went wrong! Last Error: %#012x\n", SVK_GetLastError());
        svkWindow_Destroy(window);
        svkEngine_Destroy(engine);
        return -1;
    }

    svkScene_Initialize(window, engine);

    SDL_ShowWindow(window->window);
    
    // Init Scene
    svkDrawable* cube0 = svkDrawable_Create(cubeVertices, 8, cubeIndices, 36);
    svkScene_AddDrawable(engine, cube0);

    svkDrawable* tri = svkDrawable_Create(triangleVertices, 3, NULL, 0);
    //svkScene_AddDrawable(engine, tri);

    // Update
    svkEvent event;

    svkCamera* camera = engine->scene->camera;

    while (svkWindow_Update(window, &event))
    {
        svkCamera_Update(camera, &window->inputManager, window->window);

        float currentTime = (float)clock() / CLOCKS_PER_SEC;

        float rotationSpeed = 35.0f;
        float rotationAngle = rotationSpeed * currentTime;
        vec3 rotation = { rotationAngle, rotationAngle, rotationAngle };

        const uint32_t frame = engine->core.currentFrame;

        svkScene_RotateObject(cube0, rotation, frame);
        svkInput_Update(&window->inputManager);
    }

    svkWindow_Destroy(window);
    svkEngine_Destroy(engine);

    return 0;
}