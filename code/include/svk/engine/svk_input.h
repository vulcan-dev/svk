#ifndef SVK_INPUT_H
#define SVK_INPUT_H

typedef union SDL_Event SDL_Event;
#include <SDL2/SDL_scancode.h>
#include <stdbool.h>

typedef struct svkInputManager
{
    bool keyDown[512];
    bool keyUp[512];
    bool prevKeyDown[512];
    bool prevKeyUp[512];
    bool mouseDown[12];
    bool mouseUp[12];

    int mouseX;
    int mouseY;

    int mouseRelX;
    int mouseRelY;

    int mouseWheelX;
    int mouseWheelY;

    bool mouseMoved;
} svkInputManager;

bool svkInput_Process(svkInputManager* input, SDL_Event event);
void svkInput_Update(svkInputManager* input);

bool svkInput_IsKeyJustUp(svkInputManager* input, SDL_Scancode scancode);
bool svkInput_IsKeyJustDown(svkInputManager* input, SDL_Scancode scancode);

#endif