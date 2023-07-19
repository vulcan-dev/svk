#include "svk/svk.h"
#include <time.h>
#include "cglm/cglm.h"

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

const svkVertex cubeVertices[8] = {
    // Vertex positions          // Vertex colors
    {{ -0.5f, -0.5f, -0.5f }, { 0.02f, 0.99f, 0.99f }},
    {{  0.5f, -0.5f, -0.5f }, { 0.99f, 0.99f, 0.02f }},
    {{  0.5f,  0.5f, -0.5f }, { 0.99f, 0.02f, 0.99f }},
    {{ -0.5f,  0.5f, -0.5f }, { 0.02f, 0.56f, 0.99f }},
    {{ -0.5f, -0.5f,  0.5f }, { 0.02f, 0.99f, 0.99f }},
    {{  0.5f, -0.5f,  0.5f }, { 0.99f, 0.99f, 0.02f }},
    {{  0.5f,  0.5f,  0.5f }, { 0.99f, 0.02f, 0.99f }},
    {{ -0.5f,  0.5f,  0.5f }, { 0.02f, 0.56f, 0.99f }}
};

const uint16_t cubeIndices[36] = {
    // Front face
    0, 1, 2,
    2, 3, 0,

    // Back face
    4, 5, 6,
    6, 7, 4,

    // Left face
    7, 3, 0,
    0, 4, 7,

    // Right face
    1, 5, 6,
    6, 2, 1,

    // Top face
    3, 2, 6,
    6, 7, 3,

    // Bottom face
    0, 1, 5,
    5, 4, 0
};

const uint16_t rectIndices[6] = { 0, 1, 2, 2, 3, 0 };

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

    srand((unsigned int)time(NULL));

    SDL_ShowWindow(svkw->window);
    
    // Init Scene
    svkDrawable* cube = svkDrawable_Create(cubeVertices, 8, cubeIndices, 36);
    svkScene_AddDrawable(svke, cube);

    // Update
    svkEvent event;
    while (svkWindow_Update(svkw, &event))
    {
        float currentTime = (float)clock() / CLOCKS_PER_SEC;

        float rotationSpeed = 35.0f;
        float rotationAngle = rotationSpeed * currentTime;
        vec3 rotation = { rotationAngle, rotationAngle, rotationAngle };

        const uint32_t frame = svke->core.currentFrame;

        svkScene_RotateObject(cube, rotation, frame);
    }

    svkWindow_Destroy(svkw);
    svkEngine_Destroy(svke);

    return 0;
}