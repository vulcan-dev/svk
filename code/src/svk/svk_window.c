#include "svk/svk_window.h"
#include "svk/engine/svk_renderer.h"

// Internal Function Prototypes
//------------------------------------------------------------------------
internal void _svkWindow_HandleEvents(svkWindow* svkw, SDL_Event event);
internal void ProcessInput(svkEngine* engine);

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

bool svkWindow_Update(svkWindow* window, SDL_Event* event)
{
    if (window->shouldClose)
        return false;

    while (SDL_PollEvent(event))
        _svkWindow_HandleEvents(window, *event);

    svkEngine* engine = window->engine;

    ProcessInput(engine);
    _svkEngine_DrawFrame(window->window, engine);

    char newTitle[256];
    sprintf_s(newTitle, 256, "%s (RenderTime: %fms)", window->title, engine->debug.gpuTime);
    SDL_SetWindowTitle(window->window, newTitle);

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
void LockMouse(SDL_Window* window, bool lock)
{
    SDL_bool locked = lock ? SDL_TRUE : SDL_FALSE;
    SDL_ShowCursor(!locked);
    SDL_SetRelativeMouseMode(locked);
    SDL_SetWindowGrab(window, locked);
    SDL_CaptureMouse(locked);
}

bool moveForward = false;
bool moveBackward = false;
bool moveLeft = false;
bool moveRight = false; // TODO: Move flags and handle in a seperate camera file

internal void _svkWindow_HandleResize(svkCamera* camera, int newWidth, int newHeight)
{
    camera->aspectRatio = (float)newWidth / (float)newHeight;
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
            {
                _svkWindow_HandleResize(camera, event.window.data1, event.window.data2);
                break;
            }
        }
        case SDL_MOUSEMOTION:
        {
            if (!camera->mouseLocked)
                break;

            if (camera->firstMouse)
            {
                camera->lastMouseX = event.motion.x;
                camera->lastMouseY = event.motion.y;
                camera->firstMouse = false;
            }

            const float sensitivity = camera->mouseSensitivity;

            const float xOffset = event.motion.xrel * sensitivity;
            const float yOffset = event.motion.yrel * sensitivity;

            camera->yaw -= xOffset;
            camera->pitch -= yOffset;

            if (camera->pitch > 89.0f) camera->pitch = 89.0f;
            if (camera->pitch < -89.0f) camera->pitch = -89.0f;

            break;
        }
        case SDL_KEYDOWN:
        {
            if (event.key.keysym.sym == SDLK_w)
            {
                moveForward = true;
                break;
            }
            else if (event.key.keysym.sym == SDLK_s)
            {
                moveBackward = true;
                break;
            }

            if (event.key.keysym.sym == SDLK_a)
            {
                moveLeft = true;
                break;
            }
            else if (event.key.keysym.sym == SDLK_d)
            {
                moveRight = true;
                break;
            }

            if (event.key.keysym.sym == SDLK_SPACE)
            {
                camera->mouseLocked = !camera->mouseLocked;
                LockMouse(window->window, camera->mouseLocked);
            }
        }
        case SDL_KEYUP:
        {
            if (event.key.keysym.sym == SDLK_w)
            {
                moveForward = false;
                break;
            }
            else if (event.key.keysym.sym == SDLK_s)
            {
                moveBackward = false;
                break;
            }

            if (event.key.keysym.sym == SDLK_a)
            {
                moveLeft = false;
                break;
            }
            else if (event.key.keysym.sym == SDLK_d)
            {
                moveRight = false;
                break;
            }
            break;
        }
    }
}

internal void ProcessInput(svkEngine* engine)
{
    svkCamera* camera = engine->scene->camera;

    if (moveForward)
    {
        vec3 cameraFront;
        cameraFront[0] = cos(glm_rad(camera->yaw)) * cos(glm_rad(camera->pitch));
        cameraFront[1] = sin(glm_rad(camera->yaw)) * cos(glm_rad(camera->pitch));
        cameraFront[2] = sin(glm_rad(camera->pitch));

        glm_normalize_to(cameraFront, cameraFront);
        glm_vec3_scale(cameraFront, camera->speed, cameraFront);
        glm_vec3_add(camera->pos, cameraFront, camera->pos);
    } else if (moveBackward)
    {
        vec3 cameraBackward;
        cameraBackward[0] = -cos(glm_rad(camera->yaw)) * cos(glm_rad(camera->pitch));
        cameraBackward[1] = -sin(glm_rad(camera->yaw)) * cos(glm_rad(camera->pitch));
        cameraBackward[2] = -sin(glm_rad(camera->pitch));

        glm_normalize_to(cameraBackward, cameraBackward);
        glm_vec3_scale(cameraBackward, camera->speed, cameraBackward);
        glm_vec3_add(camera->pos, cameraBackward, camera->pos);
    }

    if (moveLeft)
    {
        vec3 cameraRight;
        cameraRight[0] = cos(glm_rad(camera->yaw) + glm_rad(90.0f));
        cameraRight[1] = sin(glm_rad(camera->yaw) + glm_rad(90.0f));
        cameraRight[2] = 0.0f;

        glm_normalize_to(cameraRight, cameraRight);
        glm_vec3_scale(cameraRight, camera->speed, cameraRight);
        glm_vec3_add(camera->pos, cameraRight, camera->pos);
    } else if (moveRight)
    {
        vec3 cameraRight;
        cameraRight[0] = cos(glm_rad(camera->yaw) + glm_rad(90.0f));
        cameraRight[1] = sin(glm_rad(camera->yaw) + glm_rad(90.0f));
        cameraRight[2] = 0.0f;

        glm_normalize_to(cameraRight, cameraRight);
        glm_vec3_scale(cameraRight, camera->speed, cameraRight);
        glm_vec3_sub(camera->pos, cameraRight, camera->pos);
    }
}