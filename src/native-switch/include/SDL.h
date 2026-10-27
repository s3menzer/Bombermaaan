#ifndef BOMBERMAAAN_SWITCH_SDL_H
#define BOMBERMAAAN_SWITCH_SDL_H

/* SDL 1.2 source-compatibility headers backed by the static SDL12-compat
 * implementation.  The implementation deliberately exports SDL12_* symbols
 * so it can coexist with the SDL2 port library used by SDL_mixer. */
#include "SDL/SDL.h"

#define SDL_Init                SDL12_Init
#define SDL_InitSubSystem       SDL12_InitSubSystem
#define SDL_QuitSubSystem       SDL12_QuitSubSystem
#define SDL_WasInit             SDL12_WasInit
#define SDL_Quit                SDL12_Quit
#define SDL_GetError            SDL12_GetError
#define SDL_SetError            SDL12_SetError
#define SDL_Delay               SDL12_Delay
#define SDL_PollEvent           SDL12_PollEvent
#define SDL_PushEvent           SDL12_PushEvent
#define SDL_ListModes           SDL12_ListModes
#define SDL_SetVideoMode        SDL12_SetVideoMode
#define SDL_Flip                SDL12_Flip
#define SDL_MapRGB              SDL12_MapRGB
#define SDL_MapRGBA             SDL12_MapRGBA
#define SDL_CreateRGBSurface    SDL12_CreateRGBSurface
#define SDL_ConvertSurface      SDL12_ConvertSurface
#define SDL_FreeSurface         SDL12_FreeSurface
#define SDL_LoadBMP_RW          SDL12_LoadBMP_RW
#define SDL_SetColorKey         SDL12_SetColorKey
#define SDL_SetAlpha            SDL12_SetAlpha
#define SDL_FillRect            SDL12_FillRect
#define SDL_DisplayFormatAlpha  SDL12_DisplayFormatAlpha
#define SDL_BlitSurface         SDL12_BlitSurface
#define SDL_ShowCursor          SDL12_ShowCursor
#define SDL_WM_SetCaption       SDL12_WM_SetCaption
#define SDL_WM_SetIcon          SDL12_WM_SetIcon
#define SDL_RWFromMem           SDL12_RWFromMem
#define SDL_FreeRW              SDL12_FreeRW
#define SDL_NumJoysticks        SDL12_NumJoysticks
#define SDL_JoystickEventState  SDL12_JoystickEventState
#define SDL_JoystickOpen        SDL12_JoystickOpen
#define SDL_JoystickClose       SDL12_JoystickClose

#endif
