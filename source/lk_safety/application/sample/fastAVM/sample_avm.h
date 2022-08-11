#ifndef _SAMPLE_AVM_
#define _SAMPLE_AVM_

#if (FASTAVM_DBG_ENABLE == 1)
#define APP_DBG(...) printf(__VA_ARGS__)
#else
#define APP_DBG(...) do{} while(0)
#endif

#define CAMERA_WIDTH    1280
#define CAMERA_HEIGHT   720
#define CAMERA_FPS      25

//AVM output
#define AVM_X             0
#define AVM_Y             0



#if ENABLE_SERDES
//original camera output
#define SUBSCREEN_IMG_X      AVM_WIDTH
#define SUBSCREEN_IMG_Y      0
#define SUBSCREEN_IMG_WIDTH  1280
#define SUBSCREEN_IMG_HEIGHT 720
#else//1280*800
//original camera output
#define SUBSCREEN_IMG_X      AVM_WIDTH
#define SUBSCREEN_IMG_Y      40
#define SUBSCREEN_IMG_WIDTH  640
#define SUBSCREEN_IMG_HEIGHT 720
#endif
#define singlecameraid 2

void fastavm_test(void);

#endif

