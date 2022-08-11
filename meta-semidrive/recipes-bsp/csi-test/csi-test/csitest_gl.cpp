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

#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>

#include <xf86drm.h>
#include <drm_fourcc.h>

#include <sys/time.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <pthread.h>

#define _STRINGIFY(x) # x
#define STRINGIFY(x) _STRINGIFY(x)
#define ERRSTR strerror(errno)
#define DEBUG 1
#if DEBUG
	#define PRINTF printf
#else
	#define PRINTF(...)
#endif
#define BYE_ON(cond, ...) \
do { \
	if (cond) { \
		int errsv = errno; \
		PRINTF( "ERROR(%s:%d) : ", \
			__FILE__, __LINE__); \
		errno = errsv; \
		PRINTF(  __VA_ARGS__); \
		abort(); \
	} \
} while(0)

PFNEGLCREATEIMAGEKHRPROC pfneglCreateImageKHR = NULL;
PFNEGLDESTROYIMAGEKHRPROC pfneglDestroyImageKHR = NULL;
PFNGLEGLIMAGETARGETTEXTURE2DOESPROC pfnglEGLImageTargetTexture2DOES = NULL;
pthread_mutex_t mutex_camera = PTHREAD_MUTEX_INITIALIZER;
EGLDisplay sEGLDisplay;
EGLSurface sEGLSurface;
EGLContext sEGLContext;

static int camera_num = 8;
static int max_texture_num = 0;

#define MAX_CONFIGS 64

#define MAX_DRM_BUFFER     3 //16

#define SCREEN_WIDTH (1920)
#define IMAGE_WIDTH (1280)
#define IMAGE_HEIGHT (720)
#define CAMERA_MAX_NUMBER 8
#define CLAP_COEF (SCREEN_WIDTH - IMAGE_WIDTH)/(SCREEN_WIDTH*2)

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

	PRINTF("'%s' returned egl error '%s' (0x%llx)\n",
		name,
		GetEGLErrorString(error_code),
		(unsigned long long)error_code);
	exit(EXIT_FAILURE);
}

static void checkGlError(const char* op) {
    for (GLint error = glGetError(); error; error
            = glGetError()) {
        PRINTF("after %s() glError (0x%x)\n", op, error);
    }
}

typedef struct
{
	union
	{
		uint32_t uiHandle;
	};
	uint32_t uiPitch;
	uint32_t uiOffset;
	size_t uiSize;
	int iFd;
	void *pvPtr;
} BUFFER_PLANE;

typedef struct BUFFER_TAG
{
	int drm_fd;
	uint32_t uiWidth;
	uint32_t uiHeight;
	uint32_t uiPixelFormat;
	int iNumPlanes;
	BUFFER_PLANE sPlanes[3];
} BUFFER;

struct CamSubWindow {
    int id;
    GLuint tex;

    pthread_t tidp;
    int cam_fd;
    BUFFER *psBuffer;
    int numPlanes;
    int width;
    int height;
	void * vb_ptr[10];
	int vb_len[10];
    int fds[4];
    int offsets[4];
    int pitches[4];
    int drm_format;
    struct v4l2_format vformat;
    EGLDisplay sEGLDisplay;
    EGLImageKHR sEGLImage;
	int dmabufs[10];
	BUFFER *psBuffers[16];
	int num_buf;
	int current;
	pthread_mutex_t mutex_camera;
};

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
static char **camera_names = NULL;
char *camera_sideA_names[] = {
    	"/dev/video-evs0",
    	"/dev/video-evs1",
	"/dev/video-evs2",
	"/dev/video-evs3",
	"/dev/video4",
};

char *camera_sideB_names[] = {
    "/dev/video-evs8",
    "/dev/video-evs9",
	"/dev/video-evs10",
	"/dev/video-evs11",

	"/dev/video-dvr12",
	"/dev/video-dvr13",
	"/dev/video-dvr14",
	"/dev/video-dvr15",
};

int open_cam(const char *video_name)
{
    PRINTF("cam_open(%s)\n", video_name);
    int fd = open(video_name, O_RDWR);

    if (fd < 0) {
        PRINTF("open %s failed,erro=%s\n", video_name, strerror(errno));
    }

    return fd;
}

int cam_set_format(int cam_fd, int pixelformat, int width, int height, struct v4l2_format *out_format)
{
    int ret;

    PRINTF("width:height=%dx%d, pixelformat=0x%x, fd=%d\n", width, height,
           pixelformat, cam_fd);
    memset(out_format, 0, sizeof(struct v4l2_format));

    out_format->type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    out_format->fmt.pix.width = width;
    out_format->fmt.pix.height = height;
    out_format->fmt.pix.field = V4L2_FIELD_ANY;
    out_format->fmt.pix.pixelformat = pixelformat;
    out_format->fmt.pix.priv = 0x5a5afefe;

    ret = ioctl(cam_fd, VIDIOC_S_FMT, out_format);

    if (ret < 0) {
        PRINTF("ioctl(VIDIOC_S_FMT) failed %d(%s)\n", errno, strerror(errno));
        return ret;
    }
{
	struct v4l2_format fmt;
	int v4lfd = cam_fd;

	memset(&fmt, 0, sizeof fmt);
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;

	ret = ioctl(v4lfd, VIDIOC_G_FMT, &fmt);
	PRINTF("G_FMT(start): width = %u, height = %u, 4cc = %.4s, pitches = %d\n",
		fmt.fmt.pix.width, fmt.fmt.pix.height,
		(char*)&fmt.fmt.pix.pixelformat, fmt.fmt.pix.bytesperline);
	fmt.fmt.pix.bytesperline = fmt.fmt.pix.width * 2;
	out_format->fmt.pix.bytesperline = fmt.fmt.pix.bytesperline;
	ret = ioctl(v4lfd, VIDIOC_S_FMT, &fmt);

	ret = ioctl(v4lfd, VIDIOC_G_FMT, &fmt);
	PRINTF("G_FMT(final): width = %u, height = %u, 4cc = %.4s, pitches = %d\n",
		fmt.fmt.pix.width, fmt.fmt.pix.height,
		(char*)&fmt.fmt.pix.pixelformat, fmt.fmt.pix.bytesperline);

}
    return ret;
}

int cam_set_stream(int cam_fd, int on)
{
    int ret, cmd;
    enum v4l2_buf_type buffer_type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    PRINTF("%s(%d)\n", __func__, on);
    cmd = (on) ? VIDIOC_STREAMON : VIDIOC_STREAMOFF;
    PRINTF("set stream %s\n", (on ? "on" : "off"));
    ret = ioctl(cam_fd, cmd, &buffer_type);

    if (ret < 0) {
        PRINTF("cam_set_stream failed %d(%s)", errno, strerror(errno));
    }

    return ret;
}

static int buffer_export(int v4lfd, enum v4l2_buf_type bt, uint32_t index, int *dmafd)
{
	struct v4l2_exportbuffer expbuf;

	memset(&expbuf, 0, sizeof(expbuf));
	expbuf.type = bt;
	expbuf.index = index;
	if (ioctl(v4lfd, VIDIOC_EXPBUF, &expbuf) == -1) {
		perror("VIDIOC_EXPBUF");
		return -1;
	}

	*dmafd = expbuf.fd;

	return 0;
}

extern "C" void * bionic_memcpy(void *destin, void *source, unsigned long n);

int cam_get_frame(int cam_fd, uint8_t *out_index, struct CamSubWindow *cam)
{
    int ret;
    int length = 0;
    struct v4l2_buffer buffer = {0};
	struct v4l2_plane planes = {0};

	memset(&buffer, 0, sizeof(struct v4l2_buffer));
	memset(&planes, 0, sizeof(struct v4l2_plane));

    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    buffer.memory = V4L2_MEMORY_MMAP;
    buffer.m.planes = &planes;
    buffer.length = 1;
    buffer.reserved = 0;
    ret = ioctl(cam_fd, VIDIOC_DQBUF, &buffer);

    if (ret < 0) {
        PRINTF("ioctl(VIDIOC_DQBUF) failed %d(%s)\n", errno, strerror(errno));
        return ret;
    }

    if (buffer.index >= MAX_DRM_BUFFER) {
        PRINTF("invalid buffer index: %d\n", buffer.index);
        return ret;
    }

    *out_index = buffer.index;
    length = buffer.m.planes[0].length;
	cam->current = buffer.index;

    ret = ioctl(cam_fd, VIDIOC_QBUF, &buffer);
    if (ret < 0) {
        PRINTF("ioctl(VIDIOC_QBUF) failed %d(%s)\n", errno, strerror(errno));
        return ret;
    }
    // PRINTF("cam %d got frame [%d] length = %d\n", cam_fd, *out_index, length);
    return length;
}

static bool bQuitRequested = false;
void signal_handler(int signum)
{
	if (signum == SIGINT) {
		bQuitRequested = true;
		PRINTF("break..\n");
	}
}

static void AllocPlane(int devfd, BUFFER_PLANE *psPlane, uint32_t uiSize, int index)
{
	int dma_fd = -1;
	int ret;
	//Get v4l2 buffer dma fd
	ret = buffer_export(devfd, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE, index, &dma_fd);
	if (ret < 0)
		PRINTF("ioctl VIDIOC_EXPBUF fail to get dma fd.\n");
	else
		PRINTF("success to get dma fd : %d\n", dma_fd);

	// psPlane->uiHandle = create.handle;
	psPlane->iFd = dma_fd;
}

static BUFFER *CreateBuffer(struct format_setup *s, int width, int height, int index)
{
	BUFFER *psBuffer;
	int i;
	uint32_t uiTotalSize = 0;
	uint32_t uiOffset;
	s->width = width;
	s->height = height;
	psBuffer = (BUFFER *)calloc(1, sizeof(*psBuffer));
	if (!psBuffer)
	{
		return NULL;
	}
	for (i = 0; i < s->num_planes; i++)
	{
		psBuffer->sPlanes[i].iFd = -1;
	}
	psBuffer->uiWidth       = width;
	psBuffer->uiHeight      = height;
	psBuffer->iNumPlanes    = s->num_planes;
	psBuffer->uiPixelFormat = s->format;
	psBuffer->drm_fd = s->drm_fd;

	for (i = 0; i < s->num_planes; i++)
	{
		uint32_t uiWidth  = s->width;
		uint32_t uiHeight = s->height;
		uint32_t uiBpp    = s->bpp[i] >> 3;
		uint32_t uiSize   = uiWidth * uiHeight * uiBpp;

		psBuffer->sPlanes[i].uiPitch = uiWidth * uiBpp;
		psBuffer->sPlanes[i].uiSize  = uiSize;
		uiTotalSize += uiSize;
	}

	AllocPlane(s->drm_fd, &psBuffer->sPlanes[0], uiTotalSize, index);
	for (i = 0, uiOffset = 0; i < s->num_planes; i++)
	{
		psBuffer->sPlanes[i].uiHandle = psBuffer->sPlanes[0].uiHandle;
		psBuffer->sPlanes[i].iFd      = psBuffer->sPlanes[0].iFd;
		psBuffer->sPlanes[i].uiOffset = uiOffset;

		uiOffset += psBuffer->sPlanes[i].uiSize;
	}
	return psBuffer;
}

static void FreeBuffer(BUFFER *psBuffer) {
	free(psBuffer);
}

int v4l2_init_buffers(struct CamSubWindow *win, int index) {
	int ret = -1;
	BUFFER *psBuffer = NULL;
	win->id = index;

	win->sEGLImage = EGL_NO_IMAGE_KHR;
	win->width = IMAGE_WIDTH;
	win->height = IMAGE_HEIGHT;
	win->drm_format = DRM_FORMAT_YUYV;
	win->numPlanes = 1;
	win->num_buf = MAX_DRM_BUFFER; //MAX_DRM_BUFFER, 4

	int v4lfd = open_cam(camera_names[index]);
	BYE_ON(v4lfd < 0, "open cam %s failed, ret = %d\n", camera_names[index], v4lfd);
	win->cam_fd = v4lfd;

	struct v4l2_format fmt;
	memset(&fmt, 0, sizeof fmt);
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;

	ret = ioctl(v4lfd, VIDIOC_G_FMT, &fmt);
	BYE_ON(ret < 0, "VIDIOC_G_FMT failed: %s\n", ERRSTR);
	PRINTF("G_FMT(start): width = %u, height = %u, 4cc = %.4s\n",
		fmt.fmt.pix.width, fmt.fmt.pix.height,
		(char*)&fmt.fmt.pix.pixelformat);
#if 1
		fmt.fmt.pix.field = V4L2_FIELD_ANY;
		fmt.fmt.pix.width = win->width;
		fmt.fmt.pix.height = win->height;
		fmt.fmt.pix.pixelformat = win->drm_format;
		fmt.fmt.pix.priv = 0x5a5afefe;
#endif

	ret = ioctl(v4lfd, VIDIOC_S_FMT, &fmt);
	BYE_ON(ret < 0, "VIDIOC_S_FMT failed: %s\n", ERRSTR);

	struct v4l2_requestbuffers rqbufs;
	memset(&rqbufs, 0, sizeof(rqbufs));
	rqbufs.count = win->num_buf;
	rqbufs.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	rqbufs.memory = V4L2_MEMORY_MMAP;

	ret = ioctl(v4lfd, VIDIOC_REQBUFS, &rqbufs);
	BYE_ON(ret < 0, "VIDIOC_REQBUFS failed: %s\n", ERRSTR);
	BYE_ON(rqbufs.count < (unsigned int)win->num_buf, "video node allocated only "
		"%u of %u buffers\n", rqbufs.count, win->num_buf);
	PRINTF("VIDIOC_REQBUFS: ok, rqbufs.count = %d %dx%d\n", rqbufs.count, fmt.fmt.pix.width, fmt.fmt.pix.height);

	win->drm_format = fmt.fmt.pix.pixelformat;
	win->width = fmt.fmt.pix.width;
	win->height = fmt.fmt.pix.height;
	// PRINTF("init cam sss %d x %d %0.4s\n", win->width, win->height, win->drm_format);

	PRINTF("%s: %d, init for non-drmbuffer !\n", __func__, __LINE__);
	g_yuyv_format.drm_fd = v4lfd;

	for (int i = 0; i < (MAX_DRM_BUFFER); i++) { //win->num_buf
		psBuffer = CreateBuffer(&g_yuyv_format, win->width, win->height, i);
		if (!psBuffer)
		{
			PRINTF( "Failed to create buffer\n");
			return EXIT_FAILURE;
		}

		win->psBuffers[i] = psBuffer;
	}

    PRINTF("Switching to format \"%c%c%c%c\"\n",
        (win->drm_format >> 0) & 0xff, (win->drm_format >> 8) & 0xff,
        (win->drm_format >> 16) & 0xff, (win->drm_format >> 24) & 0xff);

	win->psBuffer = win->psBuffers[0];

	for (int i = 0; i < win->num_buf; ++i) {
		struct v4l2_buffer buf;
		struct v4l2_plane planes = {0};
		memset(&buf, 0, sizeof buf);
		memset(&planes, 0, sizeof(planes));
		PRINTF("ok check %d\n", i);
		buf.index = i;
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.m.planes = &planes;
	    buf.flags = 0;
		buf.length = 1;

		ret = ioctl(v4lfd, VIDIOC_QBUF, &buf);
		BYE_ON(ret < 0, "VIDIOC_QBUF for buffer %d failed: %s (fd %u)\n",
			buf.index, ERRSTR, buf.m.planes[0].m.fd);
	}

	PRINTF("%s:%d, done \n", __func__, __LINE__);
	return 0;
}

static GLfloat triangles[] =
{
	-1.0f, -1.0f, 0.0f, 1.0f,
	 1.0f, -1.0f, 0.0f, 1.0f,
	-1.0f,  1.0f, 0.0f, 1.0f,
	 1.0f,  1.0f, 0.0f, 1.0f,
};


static GLfloat texCoords[] =
{
	0.0f, (1.0f-CLAP_COEF),
	1.0f, (1.0f-CLAP_COEF),
	0.0f, (0.0f+CLAP_COEF),
	1.0f, (0.0f+CLAP_COEF),
};

static char const *const gShaderFiles[] =
{
	"/usr/bin/shaders/gles3_fullscreen_cam_vertshader.txt",
	"/usr/bin/shaders/gles3_fullscreen_cam_fragshader.txt"
};

static EGLBoolean file_load(unsigned i, char **pcData,
							int *piLen, const char *const paszShaderFiles[],
							unsigned uiNumShaders)
{
	FILE *fpShader;
	int iLen,iGot;

	if (i >= uiNumShaders || !pcData || !piLen)
	{
		return GL_FALSE;
	}

	/* open the shader file */
	fpShader = fopen(paszShaderFiles[i], "r");

	/* Check open succeeded */
	if(!fpShader)
	{
		PRINTF("Error: Failed to open shader file '%s'!\n", paszShaderFiles[i]);
		return GL_FALSE;
	}

	/* To get size of file, seek to end, ftell, then rewind */
	fseek(fpShader, 0, SEEK_END);
	iLen = ftell(fpShader);
	fseek(fpShader, 0, SEEK_SET);

	*pcData = (char *)malloc(iLen + 1);

	if(*pcData == NULL)
	{
		PRINTF("Error: Failed to allocate %d bytes for program '%s'!\n",
			  iLen + 1, paszShaderFiles[i]);

		fclose(fpShader);

		return GL_FALSE;
	}

	/* Read the file into the buffer */
	iGot = fread(*pcData, 1, iLen, fpShader);

	if (iGot != iLen)
	{
		// Might be ASCII vs Binary
		PRINTF("Warning: Only read %u bytes of %d from '%s'!\n",
			  iGot, iLen, paszShaderFiles[i]);
	}

	/* Close the file */
	fclose(fpShader);

	/* Terminate the string */
	(*pcData)[iGot] = '\0';

	*piLen = iGot;

	return GL_TRUE;
}

static void file_unload(char *pData)
{
	free(pData);
}



GLuint loadShader(GLenum shaderType, const char* pSource)
{
	GLuint shader = glCreateShader(shaderType);
	if (shader) {
		GLint compiled = 0;

		glShaderSource(shader, 1, &pSource, NULL);
		glCompileShader(shader);
		
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
		if (GL_TRUE != compiled) {
			PRINTF( "Error: Failed to compile GLSL shader\n");

			GLint infoLen = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
			if (infoLen) {
				char* buf = (char*) malloc(infoLen);
				if (buf) {
					glGetShaderInfoLog(shader, infoLen, NULL, buf);
					PRINTF( "Could not compile shader %d:\n%s\n",
							shaderType, buf);
					free(buf);
				}
			} else {
				PRINTF( "Guessing at GL_INFO_LOG_LENGTH size\n");
				char* buf = (char*) malloc(0x1000);
				if (buf) {
					glGetShaderInfoLog(shader, 0x1000, NULL, buf);
					PRINTF( "Could not compile shader %d:\n%s\n",
							shaderType, buf);
					free(buf);
				}
			}

			glDeleteShader(shader);
			shader = 0;
		}
	}

	return shader;
}

GLuint createProgram(const char* pVertexSource, const char* pFragmentSource)
{
	GLint bufLength = 0;
	GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
	if (!vertexShader) {
    	return 0;
	}

	GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
	if (!pixelShader) {
    	return 0;
	}
	GLuint program = glCreateProgram();

	if (program) {
		GLint nShaderStatus = GL_FALSE;

    	glAttachShader(program, vertexShader);
		checkGlError("glAttachShader");
		glAttachShader(program, pixelShader);
		checkGlError("glAttachShader");

		/* Associate shader attributes with a generic index */
		glBindAttribLocation(program, 0, "position");
		glBindAttribLocation(program, 1, "texcoord");

		glLinkProgram(program);
		glGetProgramiv(program, GL_LINK_STATUS, &nShaderStatus);
		if (nShaderStatus != GL_TRUE) {
			PRINTF( "Error: Failed to link GLSL program\n");
			goto Shader_Err;
		}

		glValidateProgram(program);
		glGetProgramiv(program, GL_VALIDATE_STATUS, &nShaderStatus);
		if (nShaderStatus != GL_TRUE)
		{
			PRINTF( "Error: Failed to validate GLSL program\n");
			goto Shader_Err;
		}
		return program;
	}

Shader_Err:
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
	if (bufLength) {
		char* buf = (char*) malloc(bufLength);
		if (buf) {
			glGetProgramInfoLog(program, bufLength, NULL, buf);
			PRINTF( "Could not link/validate program:\n%s\n", buf);
			free(buf);
		}
	}
	glDeleteProgram(program);

	return GL_FALSE;
}

GLuint gProgram;
GLint gvPositionHandle;
GLint gYuvTexSamplerHandle[CAMERA_MAX_NUMBER];

static GLint gCamNumHandle;

//static int gtransformHandle;

static EGLBoolean initGL(int cam_num)
{
	int i = 0;
	int eError = 0;
	char TexSamplerName[32] = {0,};
	char *pVertexShader;
	char *pFragmentShader;
	int nProgramLength = 0;

	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_texture_num); //gles3

	eError = glGetError();
	PRINTF("\n%s-line%d, GL_MAX_TEXTURE_UNITS: %d \n", __func__, __LINE__, max_texture_num);
	if (!(max_texture_num > 0) || (GL_NO_ERROR != eError)) {
		PRINTF("\n%s, GL_MAX_TEXTURE_UNITS: %d \n",
				__func__, max_texture_num);
		PRINTF("GL_ERROR: 0x%x\n", eError);
		return EGL_FALSE;
	}

	//glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

	if (file_load(0, &pVertexShader,
				  &nProgramLength, gShaderFiles, 2) == GL_FALSE)
	{
		return EGL_FALSE;
	}

	if (file_load(1, &pFragmentShader,
				  &nProgramLength, gShaderFiles, 2) == GL_FALSE)
	{
		return EGL_FALSE;
	}

    gProgram = createProgram(pVertexShader, pFragmentShader);
    if (!gProgram) {
        return EGL_FALSE;
    }

	file_unload(pVertexShader);
	file_unload(pFragmentShader);

	eError = glGetError();
	if (eError != GL_NO_ERROR)
	{
		PRINTF("GL_ERROR: 0x%x\n", eError);
		return EGL_FALSE;
	}

	glUseProgram(gProgram);

	for (i = 0; i < cam_num; i++) {
		snprintf(TexSamplerName, 31, "yuvTexSampler[%d]", i);
		gYuvTexSamplerHandle[i] = glGetUniformLocation(gProgram, TexSamplerName);
		if (-1 == gYuvTexSamplerHandle[i]) {
			PRINTF( "Couldn't find uniform '%s' in program\n", TexSamplerName);

			eError = glGetError();
			if (eError != GL_NO_ERROR)
			{
				PRINTF("GL_ERROR: line%d 0x%x\n", __LINE__, eError);
				return EGL_FALSE;
			}
		}
	}

	gCamNumHandle = glGetUniformLocation(gProgram, "cam_num");
	glUniform1i(gCamNumHandle, cam_num);

	eError = glGetError();
	if (eError != GL_NO_ERROR)
	{
		PRINTF("GL_ERROR: line%d 0x%x\n", __LINE__, eError);
		return EGL_FALSE;
	}

//    glViewport(0, 0, IMAGE_WIDTH, IMAGE_HEIGHT);
//    checkGlError("glViewport");

	/* Enable vertex attribute arrays - disabled by default */
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	/* Associate vertex data with the shader attributes */
	glVertexAttribPointer(0, 4, GL_FLOAT, 0, 0, triangles);
	glVertexAttribPointer(1, 2, GL_FLOAT, 0, 0, texCoords);

//	glEnable(GL_BLEND);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return EGL_TRUE;
}

static int dramWindow_perCam(struct CamSubWindow *win)
{
    int ret = 0;
	unsigned int drm_index = 0;
	BUFFER *psBuffer = NULL;

    if (!win) {
        PRINTF("error: win not inited.\n");
        return -1;
    }
	drm_index = win->current;
	if (drm_index >= MAX_DRM_BUFFER || win->psBuffers[drm_index] == win->psBuffer) {
		if (win->tex == 0)
		{
			//PRINTF("debug %d\n", __LINE__);
			glGenTextures(1, &win->tex);
			PRINTF("%s: %d ActiveTexture %d\n", __func__, __LINE__, GL_TEXTURE1 + (win->id)%max_texture_num);
			glActiveTexture(GL_TEXTURE1 + win->id % max_texture_num);

			glBindTexture(GL_TEXTURE_EXTERNAL_OES, win->tex);
			glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
		else {
			//PRINTF("\n### start glBindTexture %d ###\n\n", __LINE__);
			glActiveTexture(GL_TEXTURE1 + win->id % max_texture_num);
			glBindTexture(GL_TEXTURE_EXTERNAL_OES, win->tex);
		}
	}
	else {

	psBuffer = win->psBuffers[drm_index];
	win->psBuffer = win->psBuffers[drm_index];
//	PRINTF("%s: %d start id %d\n", __func__, __LINE__, win->current);

//	PRINTF("debug %d\n", __LINE__);
    if (win->sEGLImage != EGL_NO_IMAGE_KHR)
    {
		/* Notice: need to Destroy EGLImage before next creation,
		 * Otherwise it would cause memory leak issue, as pfneglCreateImageKHR
		 * would allocate buffer each time by itself, not controlled by caller.
		 */
        pfneglDestroyImageKHR(win->sEGLDisplay, win->sEGLImage);
        win->sEGLImage = EGL_NO_IMAGE_KHR;
    }

    if (win->sEGLImage == EGL_NO_IMAGE_KHR)
    {
        EGLint aiImageAttribs[25] =
            {
                EGL_WIDTH, (EGLint)win->width,
                EGL_HEIGHT, (EGLint)win->height / 2,
                EGL_LINUX_DRM_FOURCC_EXT, (EGLint)psBuffer->uiPixelFormat,
                EGL_DMA_BUF_PLANE0_FD_EXT, (EGLint)psBuffer->sPlanes[0].iFd,
                EGL_DMA_BUF_PLANE0_OFFSET_EXT, (EGLint)psBuffer->sPlanes[0].uiOffset,
                EGL_DMA_BUF_PLANE0_PITCH_EXT, (EGLint)psBuffer->sPlanes[0].uiPitch,
                EGL_NONE, EGL_NONE,
            };

        if (win->numPlanes > 1)
        {
            aiImageAttribs[12] = EGL_DMA_BUF_PLANE1_FD_EXT;
            aiImageAttribs[13] = psBuffer->sPlanes[1].iFd;
            aiImageAttribs[14] = EGL_DMA_BUF_PLANE1_OFFSET_EXT;
            aiImageAttribs[15] =  psBuffer->sPlanes[1].uiOffset;
            aiImageAttribs[16] = EGL_DMA_BUF_PLANE1_PITCH_EXT;
            aiImageAttribs[17] =  psBuffer->sPlanes[1].uiPitch;
            aiImageAttribs[18] = EGL_NONE;

            if (win->numPlanes > 2)
            {
                aiImageAttribs[18] = EGL_DMA_BUF_PLANE2_FD_EXT;
                aiImageAttribs[19] = psBuffer->sPlanes[2].iFd;
                aiImageAttribs[20] = EGL_DMA_BUF_PLANE2_OFFSET_EXT;
                aiImageAttribs[21] =  psBuffer->sPlanes[2].uiOffset;
                aiImageAttribs[22] = EGL_DMA_BUF_PLANE2_PITCH_EXT;
                aiImageAttribs[23] = psBuffer->sPlanes[2].uiPitch;
                aiImageAttribs[24] = EGL_NONE;
            }
        }

		win->sEGLImage = pfneglCreateImageKHR(win->sEGLDisplay, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, NULL, aiImageAttribs);
        if (EGL_NO_IMAGE_KHR == win->sEGLImage)
        {
            PRINTF( "Failed to create image\n");
			PRINTF("%s: %d failed !\n", __func__, __LINE__);
            ret = -1;
			goto OUT;
		}
	}

    if (win->tex == 0)
    {
        glGenTextures(1, &win->tex);
		glActiveTexture(GL_TEXTURE1 + win->id % max_texture_num);
		glBindTexture(GL_TEXTURE_EXTERNAL_OES, win->tex);

        glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    else
    {
		glActiveTexture(GL_TEXTURE1 + (win->id % max_texture_num));
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, win->tex);
    }
	checkGlError("glBindTexture");

	/* Create an external texture backed by egl image */
	pfnglEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, win->sEGLImage);
	checkGlError("pfnglEGLImageTargetTexture2DOES");
}

	glUniform1i(gYuvTexSamplerHandle[win->id], (1 + win->id % max_texture_num)); //GL_TEXTURE1
	checkGlError("glUniform1i");

OUT:
    return ret;
}

void draw_fps()
{
	uint64_t now = 0;
	static float drawFps = 0;
	static uint64_t draw_last_time = 0;
	static int draw_frame_count = 0;
	struct timeval tv;

    gettimeofday(&tv, NULL);

	draw_frame_count++;
    now = tv.tv_sec*1000 + tv.tv_usec/1000;
    if (now - draw_last_time > 8000) // 取固定时间间隔为8秒
    {
        drawFps = draw_frame_count / 8.0f;
        draw_frame_count = 0;
        draw_last_time = now;
		PRINTF("---Draw fps: %f---\n", drawFps);
    }

}

int fps(void)
{
	uint64_t now = 0;
	static int camFps = 0;
	static uint64_t last_time = 0;
	static int frame_count = 0;
	struct timeval tv;
    gettimeofday(&tv,NULL);

	frame_count++;
    now = tv.tv_sec*1000 + tv.tv_usec/1000;
    if (now - last_time > 8000) // 取固定时间间隔为8秒
    {
        camFps = frame_count >> 3;
        frame_count = 0;
        last_time = now;
		PRINTF("cam update fps: %d\n", camFps);
    }
    return camFps;
}

static void *camera_update_thread(void *arg)
{
    struct CamSubWindow *cam = (struct CamSubWindow*)arg;
	uint8_t index;
	int length = 0;

	sleep(cam->id);
	PRINTF("\n---------- start camera%d %s---------\n", cam->id, camera_names[cam->id]);

	v4l2_init_buffers(cam, cam->id);
	cam_set_stream(cam->cam_fd, 1);

    while (!bQuitRequested) {
		length = cam_get_frame(cam->cam_fd, &index, cam);
		cam->current = index;
		if (cam->id == 0) {
			fps();
		}
    }

	return (void *)NULL;
}


static bool has_extension(const char * const extensions_list,
						  const char * const extension_searched)
{
	const char *extension = extensions_list;
	const size_t extension_searched_length = strlen(extension_searched);

	if (!extension)
	{
		return false;
	}

	if (!extension_searched_length)
	{
		return true;
	}

	while (true)
	{
		const size_t extension_length = strcspn(extension, " ");

		if (extension_length == extension_searched_length &&
			strncmp(extension, extension_searched, extension_length) == 0)
		{
			return true;
		}

		extension += extension_length;

		if (*extension == '\0')
		{
			return false;
		}

		extension += 1;
	}
}

/* Check if required extensions exist, and get the function adresses */
static bool get_extension_funcs(EGLDisplay display)
{
	const char *eglexts = eglQueryString(display, EGL_EXTENSIONS);
	const char *glexts = (const char *)glGetString(GL_EXTENSIONS);

	if (!has_extension(eglexts, "EGL_KHR_image_base"))
	{
		PRINTF( "No EGL_KHR_image_base extension\n");
		return false;
	}

	pfneglCreateImageKHR =
		(PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");
	if (pfneglCreateImageKHR == NULL)
	{
		PRINTF( "eglGetProcAddress failed for eglCreateImageKHR\n");
		return false;
	}

	pfneglDestroyImageKHR =
		(PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR");
	if (pfneglDestroyImageKHR == NULL)
	{
		PRINTF( "eglGetProcAddress failed for eglDestroyImageKHR\n");
		return false;
	}

	if (!has_extension(eglexts, "EGL_EXT_image_dma_buf_import_modifiers"))
	{
		PRINTF( "No EGL_KHR_image_base extension\n");
		return false;
	}

	if (!has_extension(glexts, "GL_OES_EGL_image"))
	{
		PRINTF( "No GL_OES_EGL_image extension\n");
		return false;
	}

	pfnglEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)
		eglGetProcAddress("glEGLImageTargetTexture2DOES");
	if (pfnglEGLImageTargetTexture2DOES == NULL)
	{
		PRINTF(
				"eglGetProcAddress failed for glEGLImageTargetTexture2DOES\n");
		return false;
	}

	if (!has_extension(glexts, "GL_OES_EGL_image_external"))
	{
		PRINTF( "No GL_OES_EGL_image_external extension\n");
		return false;
	}

	if (!has_extension(glexts, "GL_EXT_YUV_target"))
	{
		PRINTF( "No GL_EXT_YUV_target extension\n");
		return false;
	}

	return true;
}


int main(int argc, char **argv)
{
	unsigned i, iFrameStop = 0;
	EGLint iMajor, iMinor, iNumConfigs;
    const char *eglexts;
	int bypass_display = 0;
	GLenum eError;
	camera_names = camera_sideA_names;
	camera_num = 1;

	if (argc >= 2) {
		int side = atoi(argv[1]);
		camera_names = side? camera_sideB_names: camera_sideA_names;
	}
	if (argc >= 3) {
		camera_num = atoi(argv[2]);
		if (!camera_num)
			camera_num = 1;
		else if (camera_num > CAMERA_MAX_NUMBER || camera_num < 0)
			camera_num = CAMERA_MAX_NUMBER;
	}
	if (argc >= 4) {
		bypass_display = atoi(argv[3]);
	}

	PRINTF("Start to enable %d cameras \n", camera_num);

		EGLint aiAttribList[] = {
			EGL_BUFFER_SIZE,	 EGL_DONT_CARE,
			EGL_DEPTH_SIZE, 	 8, //16
			EGL_RED_SIZE,		 8, //5
			EGL_GREEN_SIZE, 	 8, //6
			EGL_BLUE_SIZE,		 8, //5
#if defined(PBUFFER)
			EGL_SURFACE_TYPE,	 EGL_PBUFFER_BIT,
#endif
			EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
			EGL_NONE
		};

		EGLint aiContextAttribs[] = {
			EGL_CONTEXT_MAJOR_VERSION_KHR, 3,
			EGL_CONTEXT_MINOR_VERSION_KHR, 0,
			EGL_NONE
		};

	EGLConfig asConfigs[MAX_CONFIGS];

	sEGLDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (sEGLDisplay == EGL_NO_DISPLAY) {
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
            PRINTF( "EGL_EXT_image_dma_buf_import extension not supported\n");
            return EXIT_FAILURE;
    }

	if (eglChooseConfig(sEGLDisplay, aiAttribList, asConfigs, MAX_CONFIGS, &iNumConfigs) != EGL_TRUE)
	{
		handle_egl_error("eglChooseConfig");
	}

	if (!iNumConfigs)
	{
		PRINTF( "eglChooseConfig didn't return any config matching our request\n");

		return EXIT_FAILURE;
	}

#if defined(PBUFFER)
	GLubyte *readbuf = NULL;

	{
		EGLint pbuf[] = {
			EGL_WIDTH, IMAGE_WIDTH,
			EGL_HEIGHT, IMAGE_HEIGHT,
			EGL_NONE
		};

		surface = eglCreatePbufferSurface (dpy, asConfigs[0], pbuf);
		if (surface == EGL_NO_SURFACE)
		{
			handle_egl_error("eglCreatePbufferSurface");
		}
	}
	readbuf = malloc(IMAGE_WIDTH * IMAGE_HEIGHT * 4);
	if (readbuf) {
		PRINTF( "Could not allocate memory for Pbuffer\n");
		return EXIUT_FAILRE;
	}
#else

	sEGLSurface = eglCreateWindowSurface(sEGLDisplay, asConfigs[0], (EGLNativeWindowType) NULL, NULL);
	if (sEGLSurface == EGL_NO_SURFACE)
	{
		handle_egl_error("eglCreateWindowSurface");
	}
#endif

	if (eglBindAPI(EGL_OPENGL_ES_API) != EGL_TRUE)
	{
		handle_egl_error("eglBindAPI");
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

    if (!get_extension_funcs(sEGLDisplay)) {
            PRINTF( "get_extension_funcs: can't get all extension funcs (exiting).\n");
            handle_egl_error("get_extension_funcs");
    }

	eglSwapInterval(sEGLDisplay, (GLint)1);

	if (!initGL(camera_num))
	{
		PRINTF( "Error setting up GL\n");
		bQuitRequested = true;
	}
	signal(SIGINT, signal_handler);

	static struct CamSubWindow  cameras[CAMERA_MAX_NUMBER] = {0};

    //pthread_mutex_init(&mutex_camera, NULL);

    for (int i = 0; i< camera_num; i++) {
        struct CamSubWindow *cam = &cameras[i];
        cam->id = i;
        cam->sEGLDisplay = sEGLDisplay;

		cam->mutex_camera = PTHREAD_MUTEX_INITIALIZER;

		pthread_mutex_init(&cam->mutex_camera, NULL);
         if ((pthread_create(&cam->tidp, NULL, camera_update_thread, (void*)cam)) == -1) {
            PRINTF("create error!\n");
         }
    }
//	sleep(camera_num + 2);
	while(!bQuitRequested && bypass_display) {sleep(4);};

   	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	for (i = 0; !bQuitRequested && (!iFrameStop || (i < iFrameStop)); i++)
	{

		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); // if no glClear, the blank area will blank !!!

    	//glEnable(GL_TEXTURE_EXTERNAL_OES);
        for (int i = 0; i< camera_num; i++) {
			dramWindow_perCam(&cameras[i]);
        }
	    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        eError = glGetError();
        while (eError != GL_NO_ERROR)
        {
            PRINTF("GL_ERROR: 0x%x\n", eError);
            eError = glGetError();
        }
		eglSwapBuffers(sEGLDisplay, sEGLSurface);
		draw_fps();
	}

	for (int i = 0; i< camera_num; i++)
		pfneglDestroyImageKHR(sEGLDisplay, cameras[i].sEGLImage);
#if 1
	if (!eglMakeCurrent(sEGLDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT))
	{
		handle_egl_error("eglMakeCurrent");
	}
#endif

	PRINTF("\n exit() \n");
	sleep(1);

	eglDestroyContext(sEGLDisplay, sEGLContext);
	eglDestroySurface(sEGLDisplay, sEGLSurface);
	eglTerminate(sEGLDisplay);
	PRINTF("\n destroy..... \n");


	for (int i = 0; i< camera_num; i++) {
		struct CamSubWindow *cam = &cameras[i];

		if (cam->psBuffers[0]->sPlanes[0].iFd > 0)
			close(cam->psBuffers[0]->sPlanes[0].iFd);
		PRINTF("\n free() %d .....\n", i);

		for (int j = 0; j < MAX_DRM_BUFFER; j++)
			FreeBuffer(cam->psBuffers[j]);

		PRINTF("\n exit() %d .....\n", i);

		if (cam->cam_fd > 0)
			close(cam->cam_fd);
	}
	return 0;
}
