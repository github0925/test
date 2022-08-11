#ifndef _ANIMATION_CONFIG_H_
#define _ANIMATION_CONFIG_H_



#define ANIMATION_FPS 25

#if ENABLE_SERDES
#define BA_PATH "early_app/BootAnimation/splash_5760x720_high_quality_3s.mjpeg"
// #define BA_PATH "early_app/BootAnimation/splash_5760x720_high_quality.mjpeg"
#else
// #define BA_PATH "early_app/BootAnimation/splash_2560x800.mjpeg"
#define BA_PATH "early_app/BootAnimation/splash_5760x720_high_quality_3s.mjpeg"
#endif
#define FA_PATH "early_app/BootAnimation/fast_audio_pcm.bin"
#define TU_PATH "early_app/BootAnimation/turn_pcm.bin"
#define WA_PATH "early_app/BootAnimation/warning_pcm.bin"
#endif
