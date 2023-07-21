#ifndef SVK_CAMERA_H
#define SVK_CAMERA_H

#include "svk/engine/svk_input.h"
#include "svk/svk_types.h"
#include "cglm/cglm.h"

#include <stdalign.h>

typedef struct SDL_Window SDL_Window;

typedef struct svkCamera
{
    vec3 pos;
    svkVec2i lockedPos;

    float yaw;
    float pitch;
    double speed;
    float mouseSensitivity;

    float lastMouseX;
    float lastMouseY;
    bool firstMouse;
    bool mouseEnabled;
    bool inputLocked;

    float nearClip;
    float farClip;

    float aspectRatio;

    alignas(16) mat4 view;
    alignas(16) mat4 projection;
} svkCamera;

// Functions
//------------------------------------------------------------------------
void svkCamera_Initialize(svkCamera* camera, const svkVec2 windowSize);
void svkCamera_Destroy(svkCamera* camera);

void svkCamera_Reset(svkCamera* camera);
void svkCamera_Update(svkCamera* camera, svkInputManager* inputManager, SDL_Window* window);

void svkCamera_GetFront(svkCamera* camera, vec3* front);
void svkCamera_GetRight(svkCamera* camera, vec3* right);

#endif