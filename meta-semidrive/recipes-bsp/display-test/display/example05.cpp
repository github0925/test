#include <stdbool.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <GLES/gl.h>
#include <GLES/glext.h>

#include <xf86drm.h>
#include <drm_fourcc.h>

#include <sys/time.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <pthread.h>

#define _STRINGIFY(x) # x
#define STRINGIFY(x) _STRINGIFY(x)
#define ERRSTR strerror(errno)

#define BYE_ON(cond, ...) \
do { \
	if (cond) { \
		int errsv = errno; \
		fprintf(stderr, "ERROR(%s:%d) : ", \
			__FILE__, __LINE__); \
		errno = errsv; \
		fprintf(stderr,  __VA_ARGS__); \
		abort(); \
	} \
} while(0)
#define PRINTF printf

PFNEGLCREATEIMAGEKHRPROC pfneglCreateImageKHR = NULL;
PFNEGLDESTROYIMAGEKHRPROC pfneglDestroyImageKHR = NULL;
PFNGLEGLIMAGETARGETTEXTURE2DOESPROC pfnglEGLImageTargetTexture2DOES = NULL;
pthread_mutex_t mutex_camera = PTHREAD_MUTEX_INITIALIZER;

#define MAX_CONFIGS 64
void set_swap_interval_egl(void *data, unsigned int interval)
{
	eglSwapInterval((EGLDisplay)data, interval);
}

const char * GetEGLErrorString(EGLint error_code)
{
	switch(error_code)
	{
#define X(error) case error: return #error;
		X(EGL_SUCCESS)
		X(EGL_NOT_INITIALIZED)
		X(EGL_BAD_ACCESS)
		X(EGL_BAD_ALLOC)
		X(EGL_BAD_ATTRIBUTE)
		X(EGL_BAD_CONFIG)
		X(EGL_BAD_CONTEXT)
		X(EGL_BAD_CURRENT_SURFACE)
		X(EGL_BAD_DISPLAY)
		X(EGL_BAD_MATCH)
		X(EGL_BAD_NATIVE_PIXMAP)
		X(EGL_BAD_NATIVE_WINDOW)
		X(EGL_BAD_PARAMETER)
		X(EGL_BAD_SURFACE)
		X(EGL_CONTEXT_LOST)
#undef X
		default: return "unknown";
	}
}

static void handle_egl_error(const char *name)
{
	EGLint error_code = eglGetError();

	fprintf(stderr, "'%s' returned egl error '%s' (0x%llx)\n",
		name,
		GetEGLErrorString(error_code),
		(unsigned long long)error_code);
	exit(EXIT_FAILURE);
}


#define IMAGE_WIDTH (1280)
#define IMAGE_HEIGHT (720)

struct format_setup
{
	int num_planes;
	int width;
	int height;
	int format;
	int bpp[4];
	int drm_fd;
};
struct format_setup g_yuyv_format = {
	1,
	1280,
	720,
	DRM_FORMAT_YUYV,
	{32, 0, 0},
	-1,
};

static GLfloat triangles[] =
{
	-1.0f, -1.0f, 0.0f,
	1.0f, -1.0f, 0.0f,
	1.0f, 1.0f, 0.0f,
	1.0f, 1.0f, 0.0f,
	-1.0f, 1.0f, 0.0f,
	-1.0f, -1.0f, 0.0f
};
static GLfloat texCoords[] =
{
	0.0f, 1.0f,
	1.0f, 1.0f,
	1.0f, 0.0f,
	1.0f, 0.0f,
	0.0f, 0.0f,
	0.0f, 1.0f,
};


static EGLBoolean initGL(void)
{
	//glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, triangles);
	glTexCoordPointer(2, GL_FLOAT, 0, texCoords);

	return EGL_TRUE;
}

static void drawWindow(void)
{
	static int frameCount = 0;
	GLenum eError;

	frameCount++;

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//glEnable(GL_TEXTURE_EXTERNAL_OES);
	glLoadIdentity();
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	//glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f);
	glRotatef((GLfloat)frameCount, 0.0f, 1.0f, 0.0f);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	eError = glGetError();
	while (eError != GL_NO_ERROR)
	{
		printf("GL_ERROR: 0x%x\n", eError);
		eError =glGetError();
	}
}

static bool bQuitRequested = false;
void signal_handler(int signum)
{
	if (signum == SIGINT) {
		bQuitRequested = true;
		printf("break..\n");
	}
}

int fps(void)
{
	uint64_t now = 0;
	static int fps = 0;
	static uint64_t last_time = 0;
	static int frame_count = 0;
	struct timeval tv;
    gettimeofday(&tv,NULL);

	frame_count++;
    now = tv.tv_sec*1000 + tv.tv_usec/1000;
    if (now - last_time > 1000) // 取固定时间间隔为1秒
    {
        fps = frame_count;
        frame_count = 0;
        last_time = now;
    }
    return fps;
}

void dumpFps(int div) {
	static int fps_value = 0;
	static int cnt = 0;

	fps_value = fps();

	if (cnt++ % div == 0) {
		printf("camera update fps: %d\n", fps_value);
		cnt = 0;
	}
}



int main(int argc, char **argv)
{
	unsigned i, iFrameStop = 0;
	EGLint iMajor, iMinor, iNumConfigs;
    const char *eglexts;
	EGLDisplay sEGLDisplay;
	EGLSurface sEGLSurface;
	EGLContext sEGLContext;

	EGLint aiAttribList[] =
		{
			EGL_NATIVE_VISUAL_ID, 1,
			EGL_BUFFER_SIZE, EGL_DONT_CARE,
			EGL_RED_SIZE,   8,
			EGL_GREEN_SIZE, 8,
			EGL_BLUE_SIZE,  8,
			EGL_ALPHA_SIZE, 8,
			EGL_DEPTH_SIZE, 8,
			EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT,
			EGL_NONE
		};
	EGLint aiContextAttribs[] =
		{
			EGL_CONTEXT_PRIORITY_LEVEL_IMG, EGL_CONTEXT_PRIORITY_HIGH_IMG,
			EGL_NONE
		};
	EGLConfig asConfigs[MAX_CONFIGS];

	pfneglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");
	if (!pfneglCreateImageKHR)
	{
		handle_egl_error("eglGetProcAddress");
	}
	pfneglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR");
	if (!pfneglDestroyImageKHR)
	{
		handle_egl_error("eglGetProcAddress");
	}

	pfnglEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");
	if (!pfnglEGLImageTargetTexture2DOES)
	{
		handle_egl_error("eglGetProcAddress");
	}

	sEGLDisplay = eglGetDisplay(1);
	if (sEGLDisplay == EGL_NO_DISPLAY || eglGetError() != EGL_SUCCESS) {
		// Unable to open connection to local windowing system
		handle_egl_error("eglGetDisplay");
	}
	if (!eglInitialize(sEGLDisplay, &iMajor, &iMinor)) {
		// Unable to initialize EGL. Handle and recover
		handle_egl_error("eglInitialize");
	}

	eglexts = eglQueryString(sEGLDisplay, EGL_EXTENSIONS);
	if (strstr(eglexts, "EGL_EXT_image_dma_buf_import") == NULL)
	{
		fprintf(stderr, "EGL_EXT_image_dma_buf_import extension not supported\n");
		return EXIT_FAILURE;
	}

	if (eglChooseConfig(sEGLDisplay, aiAttribList, asConfigs, MAX_CONFIGS, &iNumConfigs) != EGL_TRUE)
	{
		handle_egl_error("eglChooseConfig");
	}

	if (!iNumConfigs)
	{
		fprintf(stderr, "eglChooseConfig didn't return any config matching our request\n");

		return EXIT_FAILURE;
	}

	sEGLSurface = eglCreateWindowSurface(sEGLDisplay, asConfigs[0], (EGLNativeWindowType) NULL, NULL);
	if (sEGLSurface == EGL_NO_SURFACE)
	{
		handle_egl_error("eglCreateWindowSurface");
	}

	sEGLContext = eglCreateContext(sEGLDisplay, asConfigs[0], EGL_NO_CONTEXT, aiContextAttribs);
	if (sEGLContext == EGL_NO_CONTEXT)
	{
		handle_egl_error("eglCreateContext");
	}

	if (!eglMakeCurrent(sEGLDisplay, sEGLSurface, sEGLSurface, sEGLContext))
	{
		handle_egl_error("eglMakeCurrent");
	}

	eglSwapInterval(sEGLDisplay, (GLint)1);

	if (!initGL())
	{
		fprintf(stderr, "Error setting up GL\n");
		bQuitRequested = true;
	}
	signal(SIGINT, signal_handler);

    pthread_mutex_init(&mutex_camera, NULL);

	for (i = 0; !bQuitRequested && (!iFrameStop || (i < iFrameStop)); i++)
	{
		pthread_mutex_lock(&mutex_camera);
		if (!eglMakeCurrent(sEGLDisplay, sEGLSurface, sEGLSurface, sEGLContext))
    	{
    		handle_egl_error("eglMakeCurrent");
    	}
        drawWindow();

    	eglSwapBuffers(sEGLDisplay, sEGLSurface);
        if (!eglMakeCurrent(sEGLDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT))
    	{
    		handle_egl_error("eglMakeCurrent");
    	}
        pthread_mutex_unlock(&mutex_camera);
	}

	if (!eglMakeCurrent(sEGLDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT))
	{
		handle_egl_error("eglMakeCurrent");
	}

	eglDestroyContext(sEGLDisplay, sEGLContext);
	eglDestroySurface(sEGLDisplay, sEGLSurface);
	eglTerminate(sEGLDisplay);

	return 0;
}