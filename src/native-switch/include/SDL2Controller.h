#ifndef BOMBERMAAAN_SWITCH_SDL2_CONTROLLER_H
#define BOMBERMAAN_SWITCH_SDL2_CONTROLLER_H

/*
 * SDL2 controller bridge used by the SDL 1.2 game code.
 *
 * CSDLInput includes the SDL 1.2 compatibility headers, while the Switch
 * controller implementation includes the real SDL2 headers in
 * SDL2Controller.cpp. Keeping the SDK SDL2 types on the other side of this
 * opaque interface avoids SDL 1.2/SDL2 type collisions.
 */

#ifdef __cplusplus
extern "C" {
#endif

struct BombermaaanSDL2Controller;
typedef struct BombermaaanSDL2Controller BombermaaanSDL2Controller;

int BombermaaanSDL2_InitGameController (void);
void BombermaaanSDL2_EnableEvents (void);
int BombermaaanSDL2_NumJoysticks (void);
int BombermaaanSDL2_IsGameController (int joystickIndex);
BombermaaanSDL2Controller* BombermaaanSDL2_Open (int joystickIndex);
void BombermaaanSDL2_Close (BombermaaanSDL2Controller* controller);
void BombermaaanSDL2_Update (void);
int BombermaaanSDL2_GetAttached (BombermaaanSDL2Controller* controller);
int BombermaaanSDL2_GetLeftX (BombermaaanSDL2Controller* controller);
int BombermaaanSDL2_GetLeftY (BombermaaanSDL2Controller* controller);
int BombermaaanSDL2_GetA (BombermaaanSDL2Controller* controller);
int BombermaaanSDL2_GetB (BombermaaanSDL2Controller* controller);
int BombermaaanSDL2_GetX (BombermaaanSDL2Controller* controller);
int BombermaaanSDL2_GetY (BombermaaanSDL2Controller* controller);
int BombermaaanSDL2_GetBack (BombermaaanSDL2Controller* controller);
int BombermaaanSDL2_GetDpadUp (BombermaaanSDL2Controller* controller);
int BombermaaanSDL2_GetDpadDown (BombermaaanSDL2Controller* controller);
int BombermaaanSDL2_GetDpadLeft (BombermaaanSDL2Controller* controller);
int BombermaaanSDL2_GetDpadRight (BombermaaanSDL2Controller* controller);

#ifdef __cplusplus
}
#endif

#endif
