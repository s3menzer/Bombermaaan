#include "SDL2Controller.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_gamecontroller.h>

struct BombermaaanSDL2Controller
{
    SDL_GameController* pController;
};

namespace
{
    int getButton(BombermaaanSDL2Controller* controller,
                  SDL_GameControllerButton button)
    {
        return controller != NULL &&
               controller->pController != NULL &&
               SDL_GameControllerGetButton(
                   controller->pController, button) != 0;
    }
}

extern "C"
{
int BombermaaanSDL2_InitGameController (void)
{
    return SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
}

void BombermaaanSDL2_EnableEvents (void)
{
    SDL_JoystickEventState(SDL_ENABLE);
    SDL_GameControllerEventState(SDL_ENABLE);
}

int BombermaaanSDL2_NumJoysticks (void)
{
    return SDL_NumJoysticks();
}

int BombermaaanSDL2_IsGameController (int joystickIndex)
{
    return SDL_IsGameController(joystickIndex);
}

BombermaaanSDL2Controller* BombermaaanSDL2_Open (int joystickIndex)
{
    SDL_GameController* pController =
        SDL_GameControllerOpen(joystickIndex);
    if (pController == NULL)
        return NULL;

    BombermaaanSDL2Controller* result =
        new BombermaaanSDL2Controller;
    if (result == NULL)
    {
        SDL_GameControllerClose(pController);
        return NULL;
    }

    result->pController = pController;
    return result;
}

void BombermaaanSDL2_Close (BombermaaanSDL2Controller* controller)
{
    if (controller == NULL)
        return;

    if (controller->pController != NULL)
        SDL_GameControllerClose(controller->pController);
    delete controller;
}

void BombermaaanSDL2_Update (void)
{
    SDL_GameControllerUpdate();
}

int BombermaaanSDL2_GetAttached (BombermaaanSDL2Controller* controller)
{
    return controller != NULL &&
           controller->pController != NULL &&
           SDL_GameControllerGetAttached(controller->pController);
}

int BombermaaanSDL2_GetLeftX (BombermaaanSDL2Controller* controller)
{
    return controller != NULL && controller->pController != NULL
        ? SDL_GameControllerGetAxis(
              controller->pController, SDL_CONTROLLER_AXIS_LEFTX)
        : 0;
}

int BombermaaanSDL2_GetLeftY (BombermaaanSDL2Controller* controller)
{
    return controller != NULL && controller->pController != NULL
        ? SDL_GameControllerGetAxis(
              controller->pController, SDL_CONTROLLER_AXIS_LEFTY)
        : 0;
}

int BombermaaanSDL2_GetA (BombermaaanSDL2Controller* controller)
{
    return getButton(controller, SDL_CONTROLLER_BUTTON_A);
}

int BombermaaanSDL2_GetB (BombermaaanSDL2Controller* controller)
{
    return getButton(controller, SDL_CONTROLLER_BUTTON_B);
}

int BombermaaanSDL2_GetX (BombermaaanSDL2Controller* controller)
{
    return getButton(controller, SDL_CONTROLLER_BUTTON_X);
}

int BombermaaanSDL2_GetY (BombermaaanSDL2Controller* controller)
{
    return getButton(controller, SDL_CONTROLLER_BUTTON_Y);
}

int BombermaaanSDL2_GetBack (BombermaaanSDL2Controller* controller)
{
    return getButton(controller, SDL_CONTROLLER_BUTTON_BACK);
}

int BombermaaanSDL2_GetDpadUp (BombermaaanSDL2Controller* controller)
{
    return getButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP);
}

int BombermaaanSDL2_GetDpadDown (BombermaaanSDL2Controller* controller)
{
    return getButton(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
}

int BombermaaanSDL2_GetDpadLeft (BombermaaanSDL2Controller* controller)
{
    return getButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
}

int BombermaaanSDL2_GetDpadRight (BombermaaanSDL2Controller* controller)
{
    return getButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
}
}
