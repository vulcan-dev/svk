#include "svk/svk_common.h"
#include "svk/svk_window.h"
#include "svk/util/svk_logger.h"
#include "svk/camera/svk_camera.h"
#include "SDL2/SDL.h"

// Creation & Destruction
//------------------------------------------------------------------------
void svkCamera_Initialize(svkCamera* camera, const svkVec2 windowSize)
{
    SVK_ASSERT(camera, "Could not initialize invalid camera");
    SVK_ZMPTR(camera, sizeof(svkCamera));

    camera->firstMouse = true;
    camera->aspectRatio = (float)windowSize.x / (float)windowSize.y;
    svkCamera_Reset(camera);
}

void svkCamera_Destroy(svkCamera* camera)
{
    SVK_ASSERT(camera, "Could not destroy NULL camera");
    SVK_FREE(camera);
}

// Internal Camera Functions
//------------------------------------------------------------------------
internal void _svkCamera_Move(svkCamera* camera, vec3 direction)
{
    glm_vec3_scale(direction, (float)camera->speed, direction);
    glm_vec3_add(camera->pos, direction, camera->pos);
}

internal void _svkCamera_ProcessInputs(svkCamera* camera, svkInputManager* inputManager, SDL_Window* window)
{
    if (camera->firstMouse)
    {
        camera->lastMouseX = inputManager->mouseX;
        camera->lastMouseY = inputManager->mouseY;
        camera->firstMouse = false;
    }
       
    // Mouse Movement
    if (inputManager->mouseMoved && camera->mouseEnabled)
    {
        const float sensitivity = camera->mouseSensitivity;
        const float xOffset = inputManager->mouseRelX * sensitivity;
        const float yOffset = inputManager->mouseRelY * sensitivity;

        camera->yaw -= xOffset;
        camera->pitch -= yOffset;

        if (camera->pitch > 89.0f) camera->pitch = 89.0f;
        if (camera->pitch < -89.0f) camera->pitch = -89.0f;

        camera->lastMouseX = inputManager->mouseX;
        camera->lastMouseY = inputManager->mouseY;
    }

    // Toggle Lock
    if (svkInput_IsKeyJustDown(inputManager, SDL_SCANCODE_TAB))
    {
        camera->mouseEnabled = !camera->mouseEnabled;
        camera->inputLocked = !camera->inputLocked;
        svkWindow_LockMouse(window, camera->mouseEnabled);

        if (camera->mouseEnabled)
        {
            camera->lockedPos = (svkVec2i){ inputManager->mouseX, inputManager->mouseY };
        } else
        {
            SDL_WarpMouseInWindow(window, camera->lockedPos.x, camera->lockedPos.y);
        }
    } else if (svkInput_IsKeyJustDown(inputManager, SDL_SCANCODE_R))
    {
        glm_vec3_zero(camera->pos);
    }

    if (camera->inputLocked)
        return;

    if (inputManager->mouseWheelY > 0)
    {
        camera->speed += 0.001f;
        if (camera->speed >= 0.02)
            camera->speed = 0.02;
    } else if (inputManager->mouseWheelY < 0)
    {
        camera->speed -= 0.001f;
        if (camera->speed <= 0.001)
            camera->speed = 0.001;
    }

    { // Forwards and Backwards
        vec3 cameraFront;
        svkCamera_GetFront(camera, &cameraFront);

        // Keyboard Movement
        if (inputManager->keyDown[SDL_SCANCODE_W])
        {
            _svkCamera_Move(camera, cameraFront);
        } else if (inputManager->keyDown[SDL_SCANCODE_S])
        {
            glm_vec3_negate(cameraFront);
            _svkCamera_Move(camera, cameraFront);
        }
    }

    { // Left / Right
        vec3 cameraRight;
        svkCamera_GetRight(camera, &cameraRight);

        if (inputManager->keyDown[SDL_SCANCODE_A])
        {
            _svkCamera_Move(camera, cameraRight);
        } else if (inputManager->keyDown[SDL_SCANCODE_D])
        {
            glm_vec3_negate(cameraRight);
            _svkCamera_Move(camera, cameraRight);
        }
    }

    if (inputManager->keyDown[SDL_SCANCODE_SPACE])
    {
        _svkCamera_Move(camera, VEC3_UP);
    }
    else if (inputManager->keyDown[SDL_SCANCODE_LCTRL])
    {
        vec3 downVector = { 0.0f, 0.0f, -1.0f };
        _svkCamera_Move(camera, downVector);
    }
}

// General Camera Functions
//------------------------------------------------------------------------
void svkCamera_Reset(svkCamera* camera)
{
    SVK_ASSERT(camera, "Camera is NULL");

    camera->mouseSensitivity = 0.05f;
    camera->yaw = -90.0f;
    camera->pitch = 0.0f;
    camera->speed = 0.003f;
    camera->nearClip = 0.1f;
    camera->farClip = 100.0f;
    camera->pos[0] = 0.0f;
    camera->pos[1] = 2.0f;
    camera->pos[2] = 0.0f;
}

void svkCamera_Update(svkCamera* camera, svkInputManager* inputManager, SDL_Window* window)
{
    _svkCamera_ProcessInputs(camera, inputManager, window);

    vec3 front;
    front[0] = cos(glm_rad(camera->yaw)) * cos(glm_rad(camera->pitch));
    front[1] = sin(glm_rad(camera->yaw)) * cos(glm_rad(camera->pitch));
    front[2] = sin(glm_rad(camera->pitch));
    glm_normalize_to(front, front);

    vec3 target;
    glm_vec3_add(camera->pos, front, target);

    glm_lookat(camera->pos, target, VEC3_UP, camera->view);

    glm_perspective(glm_rad(90.0f), camera->aspectRatio, camera->nearClip, camera->farClip, camera->projection);
    camera->projection[1][1] *= -1.0f;
}

void svkCamera_GetFront(svkCamera* camera, vec3* front)
{
    float cameraFront[3];
    cameraFront[0] = cos(glm_rad(camera->yaw)) * cos(glm_rad(camera->pitch));
    cameraFront[1] = sin(glm_rad(camera->yaw)) * cos(glm_rad(camera->pitch));
    cameraFront[2] = sin(glm_rad(camera->pitch));
    glm_normalize_to(cameraFront, cameraFront);
    memcpy(front, cameraFront, sizeof(vec3));
}

void svkCamera_GetRight(svkCamera* camera, vec3* right)
{
    float cameraRight[3];
    cameraRight[0] = cos(glm_rad(camera->yaw) + glm_rad(90.0f));
    cameraRight[1] = sin(glm_rad(camera->yaw) + glm_rad(90.0f));
    cameraRight[2] = 0.0f;
    glm_normalize_to(cameraRight, cameraRight);
    memcpy(right, cameraRight, sizeof(vec3));
}