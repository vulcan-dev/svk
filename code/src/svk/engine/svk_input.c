#include "svk/engine/svk_input.h"
#include "svk/util/svk_logger.h"

#include <SDL2/SDL.h>

bool svkInput_Process(svkInputManager* input, SDL_Event event)
{
    const SDL_Scancode scancode = event.key.keysym.scancode;
    const Uint8 mbutton = event.button.button;

    switch (event.type)
    {
        case SDL_MOUSEMOTION:
        {
            input->mouseMoved = true;
            input->mouseX = event.motion.x;
            input->mouseY = event.motion.y;
            input->mouseRelX = event.motion.xrel;
            input->mouseRelY = event.motion.yrel;
            break;
        }
        case SDL_MOUSEWHEEL:
        {
            input->mouseWheelX = event.wheel.x;
            input->mouseWheelY = event.wheel.y;
        }
        case SDL_KEYDOWN:
        {
            input->keyDown[scancode] = true;
            input->keyUp[scancode] = false;
            break;
        }
        case SDL_KEYUP:
        {
            input->keyUp[scancode] = true;
            input->keyDown[scancode] = false;
            break;
        }
        case SDL_MOUSEBUTTONDOWN:
        {
            input->mouseDown[mbutton] = true;
            input->mouseUp[mbutton] = false;
            break;
        }
        case SDL_MOUSEBUTTONUP:
        {
            input->mouseUp[mbutton] = true;
            input->mouseDown[mbutton] = false;
        }
        default:
            return false; // Not handled
    }

    return true; // Handled
}

void svkInput_Update(svkInputManager* input) // Call at the end of the update loop
{
    memcpy(input->prevKeyDown, input->keyDown, SDL_NUM_SCANCODES * sizeof(bool));
    memcpy(input->prevKeyUp, input->keyUp, SDL_NUM_SCANCODES * sizeof(bool));
    input->mouseMoved = false;
    input->mouseWheelX = 0;
    input->mouseWheelY = 0;
}

bool svkInput_IsKeyPressed(svkInputManager* input, SDL_Scancode scancode)
{
    return input->prevKeyUp[scancode] && !input->keyUp[scancode];
}

bool svkInput_IsKeyJustDown(svkInputManager* input, SDL_Scancode scancode)
{
    return input->keyDown[scancode] && !input->prevKeyDown[scancode];
}

bool svkInput_IsKeyJustUp(svkInputManager* input, SDL_Scancode scancode)
{
    return input->prevKeyDown[scancode] && !input->keyDown[scancode];
}