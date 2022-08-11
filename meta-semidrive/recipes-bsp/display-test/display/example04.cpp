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
EGLDisplay sEGLDisplay;
EGLSurface sEGLSurface;
EGLContext sEGLContext;

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
#define CAMERA_MAX_NUMBER 8

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
	GLuint textures[10];
    pthread_t tidp;
    int cam_fd;
    BUFFER *psBuffer;
    int numPlanes;
    int width;
    int height;
	int vb_ptr[10];
	int vb_len[10];
    int fds[4];
    int offsets[4];
    int pitches[4];
    int drm_format;
    struct v4l2_format vformat;
    EGLDisplay sEGLDisplay;
    EGLImageKHR sEGLImage;
	int dmabufs[10];
	BUFFER *psBuffers[10];
	int num_buf;
	int current;
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
const char *camera_names[] = {
    "/dev/video-evs0",
    "/dev/video-evs1",
	"/dev/video-evs2",
	"/dev/video-evs3",
	"/dev/video-dvr4",
	"/dev/video-dvr5",
	"/dev/video-dvr6",
	"/dev/video-dvr7",
	"/dev/video8",
};

int drm_ioctl_open_priv(void)
{
    static int fd = drmOpen("semidrive", NULL);

    if (fd < 0)
        printf("open semidrive drm failed: %d(%s)", errno, strerror(errno));

    return fd;
}

int open_cam(const char *video_name)
{
    printf("cam_open(%s)\n", video_name);
    int fd = open(video_name, O_RDWR);

    if (fd < 0) {
        printf("open %s failed,erro=%s\n", video_name, strerror(errno));
    }

    return fd;
}

int cam_set_format(int cam_fd, int pixelformat, int width, int height, struct v4l2_format *out_format)
{
    int ret;

    fprintf(stderr, "width:height=%dx%d, pixelformat=0x%x, fd=%d\n", width, height,
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
        printf("ioctl(VIDIOC_S_FMT) failed %d(%s)\n", errno, strerror(errno));
        return ret;
    }
{
	struct v4l2_format fmt;
	int v4lfd = cam_fd;

	memset(&fmt, 0, sizeof fmt);
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;

	ret = ioctl(v4lfd, VIDIOC_G_FMT, &fmt);
	printf("G_FMT(start): width = %u, height = %u, 4cc = %.4s, pitches = %d\n",
		fmt.fmt.pix.width, fmt.fmt.pix.height,
		(char*)&fmt.fmt.pix.pixelformat, fmt.fmt.pix.bytesperline);
	fmt.fmt.pix.bytesperline = fmt.fmt.pix.width * 2;
	out_format->fmt.pix.bytesperline = fmt.fmt.pix.bytesperline;
	ret = ioctl(v4lfd, VIDIOC_S_FMT, &fmt);

	ret = ioctl(v4lfd, VIDIOC_G_FMT, &fmt);
	printf("G_FMT(final): width = %u, height = %u, 4cc = %.4s, pitches = %d\n",
		fmt.fmt.pix.width, fmt.fmt.pix.height,
		(char*)&fmt.fmt.pix.pixelformat, fmt.fmt.pix.bytesperline);

}
    return ret;
}

int cam_set_stream(int cam_fd, int on)
{
    int ret, cmd;
    enum v4l2_buf_type buffer_type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    printf("%s(%d)\n", __func__, on);
    cmd = (on) ? VIDIOC_STREAMON : VIDIOC_STREAMOFF;
    printf("set stream %s\n", (on ? "on" : "off"));
    ret = ioctl(cam_fd, cmd, &buffer_type);

    if (ret < 0) {
        printf("cam_set_stream failed %d(%s)", errno, strerror(errno));
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

int cam_get_frame(int cam_fd, uint8_t *out_index)
{
    int ret;
    int length = 0;
    struct v4l2_buffer buffer;

    memset(&buffer, 0, sizeof(struct v4l2_buffer));
    struct v4l2_plane planes = {0};
    memset(&planes, 0, sizeof(planes));
    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    buffer.memory = V4L2_MEMORY_MMAP;
    buffer.m.planes = &planes;
    buffer.length = 1;
    buffer.reserved = 0;
    ret = ioctl(cam_fd, VIDIOC_DQBUF, &buffer);

    if (ret < 0) {
        printf("ioctl(VIDIOC_DQBUF) failed %d(%s)\n", errno, strerror(errno));
        return ret;
    }

    if (buffer.index >= 4) {
        printf("invalid buffer index: %d\n", buffer.index);
        return ret;
    }

    *out_index = buffer.index;
    length = buffer.m.planes[0].length;

    ret = ioctl(cam_fd, VIDIOC_QBUF, &buffer);

    if (ret < 0) {
        printf("ioctl(VIDIOC_QBUF) failed %d(%s)\n", errno, strerror(errno));
        return ret;
    }
    // printf("cam %d got frame [%d] length = %d\n", cam_fd, *out_index, length);
    return length;
}


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

static GLfloat cam_positions[] =
{
    -3.0f, 1.0f, 0.0f,
    -1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 0.0f,
    3.0f, 1.0f, 0.0f,

    -3.0f, -1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,
    3.0f, -1.0f, 0.0f,
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

static void AllocPlane(int drmfd, BUFFER_PLANE *psPlane, uint32_t uiSize)
{
	struct drm_mode_create_dumb create;
	struct drm_prime_handle prime;
	int err;

	memset(&create, 0, sizeof(create));
	create.bpp = 8;
	create.width = 1;
	create.height = uiSize;
	err = drmIoctl(drmfd, DRM_IOCTL_MODE_CREATE_DUMB, &create);
	BYE_ON(err, "Failed to allocate buffer (err=%d)\n", errno);


	memset(&prime, 0, sizeof(prime));
	prime.handle = create.handle;

	err = drmIoctl(drmfd, DRM_IOCTL_PRIME_HANDLE_TO_FD, &prime);
	BYE_ON(err, "Failed to export fd (err=%d)\n", errno);

	psPlane->uiHandle = create.handle;
	psPlane->iFd = prime.fd;
	BYE_ON(uiSize != create.size, "create size failed");
}

static BUFFER *CreateBuffer(struct format_setup *s, int width, int height)
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

	AllocPlane(s->drm_fd, &psBuffer->sPlanes[0], uiTotalSize);
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
	int fd = -1234;
	for (int i = 0; i < psBuffer->iNumPlanes; i++) {
		if (fd == psBuffer->sPlanes[i].iFd) {
			struct drm_gem_close gem_close;

			memset(&gem_close, 0, sizeof(gem_close));
			gem_close.handle = psBuffer->sPlanes[i].uiHandle;

			(void) drmIoctl(psBuffer->drm_fd, DRM_IOCTL_GEM_CLOSE, &gem_close);
		}
		fd = psBuffer->sPlanes[i].iFd;
	}

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
	win->num_buf = 4;

	int v4lfd = open_cam(camera_names[index]);
	BYE_ON(v4lfd < 0, "open cam %s failed, ret = %d\n", camera_names[index], v4lfd);
	win->cam_fd = v4lfd;

	struct v4l2_format fmt;
	memset(&fmt, 0, sizeof fmt);
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;

	ret = ioctl(v4lfd, VIDIOC_G_FMT, &fmt);
	BYE_ON(ret < 0, "VIDIOC_G_FMT failed: %s\n", ERRSTR);
	printf("G_FMT(start): width = %u, height = %u, 4cc = %.4s\n",
		fmt.fmt.pix.width, fmt.fmt.pix.height,
		(char*)&fmt.fmt.pix.pixelformat);
#if 1

		fmt.fmt.pix.width = win->width;
		fmt.fmt.pix.height = win->height;
		//fmt.fmt.pix.pixelformat = win->drm_format;
#endif

	ret = ioctl(v4lfd, VIDIOC_S_FMT, &fmt);
	BYE_ON(ret < 0, "VIDIOC_S_FMT failed: %s\n", ERRSTR);

	ret = ioctl(v4lfd, VIDIOC_G_FMT, &fmt);
	BYE_ON(ret < 0, "VIDIOC_G_FMT failed: %s\n", ERRSTR);
	printf("G_FMT(final): width = %u, height = %u, 4cc = %.4s\n",
		fmt.fmt.pix.width, fmt.fmt.pix.height,
		(char*)&fmt.fmt.pix.pixelformat);

	struct v4l2_requestbuffers rqbufs;
	memset(&rqbufs, 0, sizeof(rqbufs));
	rqbufs.count = win->num_buf;
	rqbufs.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	rqbufs.memory = V4L2_MEMORY_DMABUF;

	ret = ioctl(v4lfd, VIDIOC_REQBUFS, &rqbufs);
	BYE_ON(ret < 0, "VIDIOC_REQBUFS failed: %s\n", ERRSTR);
	BYE_ON(rqbufs.count < (unsigned int)win->num_buf, "video node allocated only "
		"%u of %u buffers\n", rqbufs.count, win->num_buf);
	printf("VIDIOC_REQBUFS: ok, rqbufs.count = %d %dx%d\n", rqbufs.count, fmt.fmt.pix.width, fmt.fmt.pix.height);

	win->drm_format = fmt.fmt.pix.pixelformat;
	win->width = fmt.fmt.pix.width;
	win->height = fmt.fmt.pix.height;
	// printf("init cam sss %d x %d %0.4s\n", win->width, win->height, win->drm_format);

	for (int i = 0; i < win->num_buf; i++) {
		psBuffer = CreateBuffer(&g_yuyv_format, win->width, win->height);
		if (!psBuffer)
		{
			fprintf(stderr, "Failed to create buffer\n");
			return EXIT_FAILURE;
		}

		win->psBuffers[i] = psBuffer;
	}

    printf("Switching to format \"%c%c%c%c\"\n",
        (win->drm_format >> 0) & 0xff, (win->drm_format >> 8) & 0xff,
        (win->drm_format >> 16) & 0xff, (win->drm_format >> 24) & 0xff);

	win->psBuffer = win->psBuffers[0];

	for (int i = 0; i < win->num_buf; ++i) {
		struct v4l2_buffer buf;
		struct v4l2_plane planes = {0};
		memset(&buf, 0, sizeof buf);
		memset(&planes, 0, sizeof(planes));
		printf("ok check %d\n", i);
		buf.index = i;
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		buf.memory = V4L2_MEMORY_DMABUF;
		buf.m.planes = &planes;
	    buf.flags = 0;
		buf.length = 1;
		//buf.m.fd = buffer[i].dbuf_fd;
		buf.m.planes[0].m.fd = win->psBuffers[i]->sPlanes[0].iFd;
		ret = ioctl(v4lfd, VIDIOC_QBUF, &buf);
		BYE_ON(ret < 0, "VIDIOC_QBUF for buffer %d failed: %s (fd %u)\n",
			buf.index, ERRSTR, buf.m.planes[0].m.fd);
	}

	return 0;
}


static int captureCamWindow(struct CamSubWindow *win, int test)
{
    int ret = 0;
	BUFFER *psBuffer = test? win->psBuffer: win->psBuffers[win->current];

    if (!win) {
        printf("error: win not inited.\n");
        return -1;
    }

    pthread_mutex_lock(&mutex_camera);
	win->tex = win->textures[win->current];
	if (!eglMakeCurrent(sEGLDisplay, sEGLSurface, sEGLSurface, sEGLContext))
	{
		handle_egl_error("eglMakeCurrent");
	}

    /* First destroy anything from previous image */
    if (win->tex)
    {
        glDeleteTextures(1, &win->tex);
        win->tex = 0;
    }

    if (win->sEGLImage != EGL_NO_IMAGE_KHR)
    {
        pfneglDestroyImageKHR(win->sEGLDisplay, win->sEGLImage);
        win->sEGLImage = EGL_NO_IMAGE_KHR;
    }

    if (win->tex == 0)
    {
        glGenTextures(1, &win->tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, win->tex);
    }
    else
    {
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, win->tex);
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
                EGL_NONE
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
        if (!win->sEGLImage)
        {
            fprintf(stderr, "Failed to create image\n");
            ret = -1;
			goto OUT;
        }

        pfnglEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, win->sEGLImage);
    }
	win->textures[win->current] = win->tex;

    ret = 0;
OUT:
    if (!eglMakeCurrent(sEGLDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT))
	{
		handle_egl_error("eglMakeCurrent");
	}
    pthread_mutex_unlock(&mutex_camera);
    return ret;
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

static void *camera_update_thread(void *arg)
{
    struct CamSubWindow *cam = (struct CamSubWindow*)arg;
	sleep(cam->id);
	printf("\n---------- start camera%d %s---------\n", cam->id, camera_names[cam->id]);
	v4l2_init_buffers(cam, cam->id);
	cam_set_stream(cam->cam_fd, 1);

    while (1) {
		uint8_t index;
		cam_get_frame(cam->cam_fd, &index);
		cam->current = index;
		captureCamWindow(cam, false);
		if (cam->id == 0) {
			dumpFps(60);
		}
    }
}

static void drawSubWindow(struct CamSubWindow *cam, int frameCount)
{
    int i = cam->id * 3;
    glLoadIdentity();
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, cam->textures[cam->current]);
	glScalef(0.25f, 0.5f, 1.0f);
    glTranslatef(cam_positions[i], cam_positions[i+1], cam_positions[i+2]);
    // glRotatef((GLfloat)frameCount, 0.0f, 1.0f, 0.0f);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

int main(int argc, char **argv)
{
	unsigned i, iFrameStop = 0;
	EGLint iMajor, iMinor, iNumConfigs;
    const char *eglexts;

	EGLint aiAttribList[] =
		{
			EGL_NATIVE_VISUAL_ID, 0,
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

	g_yuyv_format.drm_fd = drm_ioctl_open_priv();


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

	static struct CamSubWindow  cameras[9] = {0};

    pthread_mutex_init(&mutex_camera, NULL);

    for (int i = 0; i< CAMERA_MAX_NUMBER;i++) {
        struct CamSubWindow *cam = &cameras[i];
        cam->id = i;
        cam->sEGLDisplay = sEGLDisplay;

         if ((pthread_create(&cam->tidp, NULL, camera_update_thread, (void*)cam)) == -1) {
            printf("create error!\n");
         }
    }
	for (i = 0; !bQuitRequested && (!iFrameStop || (i < iFrameStop)); i++)
	{
		pthread_mutex_lock(&mutex_camera);
		if (!eglMakeCurrent(sEGLDisplay, sEGLSurface, sEGLSurface, sEGLContext))
    	{
    		handle_egl_error("eglMakeCurrent");
    	}
#if 0
        drawWindow();
#else
        GLenum eError;
        static int frameCount = 0;
    	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    	glEnable(GL_TEXTURE_EXTERNAL_OES);
        for (int i = 0; i< CAMERA_MAX_NUMBER; i++)
            drawSubWindow(&cameras[i], frameCount);
        frameCount++;
        eError = glGetError();
        while (eError != GL_NO_ERROR)
        {
            printf("GL_ERROR: 0x%x\n", eError);
            eError =glGetError();
        }
#endif

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
        for (int i = 0; i< CAMERA_MAX_NUMBER; i++) {
			struct CamSubWindow *cam = &cameras[i];
			for (int j = 0; j < cam->num_buf; j++)
				FreeBuffer(cam->psBuffers[i]);
		}
	return 0;
}