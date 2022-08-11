#ifndef _FASTAVM_CONFIG_H_
#define _FASTAVM_CONFIG_H_

#define FASTAVM_DBG_ENABLE    0
/* print time for every step */
#define FASTAVM_DBG_PROFILE   0

#if (FASTAVM_DBG_ENABLE == 1)
#define APP_DBG(...) printf(__VA_ARGS__)
#else
#define APP_DBG(...) do{} while(0)
#endif

#if (FASTAVM_DBG_PROFILE == 1)
#define APP_PRF(...) printf(__VA_ARGS__)
#else
#define APP_PRF(...) do{} while(0)
#endif

#define PLAYER_USE_DISPLAY   1
#define PLAYER_USE_FPS_TIMER 0
/* avm module is fixed @25fps */
#define ANIMATION_FPS 25

#define AVM_X             0
#define AVM_Y             0
#define AVM_WIDTH         640
#define AVM_HEIGHT        800
#define IMG_WIDTH         1280
#define IMG_HEIGHT        720
#define SINGLE_IMG_X      640
#define SINGLE_IMG_Y      0
#define SINGLE_IMG_WIDTH  640
#define SINGLE_IMG_HEIGHT 360
#define CAMERA_WIDTH      1280
#define CAMERA_HEIGHT     720

#ifndef singlecameraid 
#define singlecameraid 2
#endif


/* temply show 2 layers, 1 for vdsp transformed,
 * 1 for original camera output, will be redone with G2D */
#define PLAYER_LAYERS     2
#endif
