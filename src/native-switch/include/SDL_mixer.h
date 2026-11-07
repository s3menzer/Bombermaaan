#ifndef BOMBERMAAAN_SWITCH_SDL_MIXER_H
#define BOMBERMAAAN_SWITCH_SDL_MIXER_H

/* Minimal SDL_mixer 1.2-shaped declarations for the SDL2_mixer port library.
 * CSound only uses the common Mix_* API.  SDL2's RWops are kept distinct from
 * the SDL12-compat RWops used by the video code. */
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Mix_Chunk Mix_Chunk;
typedef struct Mix_Music Mix_Music;
typedef struct SDL2_RWops SDL2_RWops;

typedef uint16_t SDL_AudioFormat;
#ifndef AUDIO_S16LSB
#define AUDIO_S16LSB ((SDL_AudioFormat)0x8010u)
#endif
#define MIX_MAX_VOLUME 128

int Mix_OpenAudio(int frequency, SDL_AudioFormat format, int channels, int chunksize);
void Mix_CloseAudio(void);
int Mix_AllocateChannels(int numchans);

Mix_Chunk *Mix_LoadWAV_RW(SDL2_RWops *src, int freesrc);
Mix_Music *Mix_LoadMUS_RW(SDL2_RWops *src, int freesrc);
void Mix_FreeChunk(Mix_Chunk *chunk);
void Mix_FreeMusic(Mix_Music *music);

int Mix_HaltChannel(int channel);
int Mix_HaltMusic(void);
int Mix_PlayMusic(Mix_Music *music, int loops);
void Mix_PauseMusic(void);
void Mix_ResumeMusic(void);
int Mix_Volume(int channel, int volume);
int Mix_VolumeMusic(int volume);

#define Mix_GetError SDL2_GetError
#define Mix_PlayChannel(channel, chunk, loops) \
    Mix_PlayChannelTimed((channel), (chunk), (loops), -1)
int Mix_PlayChannelTimed(int channel, Mix_Chunk *chunk, int loops, int ticks);

/* The names below are the SDL2 ABI symbols.  asm labels preserve those names
 * while avoiding a collision with the SDL12-compat SDL_RWops declarations. */
#ifdef __GNUC__
#define BOMBERMAAAN_SWITCH_ASM(name) __asm__(name)
#else
#define BOMBERMAAAN_SWITCH_ASM(name)
#endif

const char *SDL2_GetError(void) BOMBERMAAAN_SWITCH_ASM("SDL_GetError");
SDL2_RWops *SDL2_RWFromMem(void *mem, int size) BOMBERMAAAN_SWITCH_ASM("SDL_RWFromMem");
void SDL2_FreeRW(SDL2_RWops *area) BOMBERMAAAN_SWITCH_ASM("SDL_FreeRW");
#ifdef __cplusplus
}
#endif

#endif
