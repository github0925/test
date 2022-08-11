/*************************************************************************/ /*!
@File
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <drv/GLES3/glimgext.h>
#include "slt_gpu.h"

#if !defined(GL_ES_VERSION_HALTI) && !defined(GL_ES_VERSION_3_0)
#error ("wrong header file")
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#if ((defined(LINUX) || defined(__QNXNTO__) || defined (INTEGRITY_OS)) && !defined(ANDROID))
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <errno.h>

typedef struct termios _termios;
_termios gOldTerm;

void _kbhit_init(void);
int _kbhit(void);
int _getch(void);

#if defined (INTEGRITY_OS)
void performGracefulShutdown(void);
#endif
void restoreTerminal(void);

void _kbhit_init()
{
	_termios term;
	tcgetattr(STDIN_FILENO, &gOldTerm);
	term = gOldTerm;

	term.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &term);
#if !defined(ANDROID)
	setbuf(stdin, NULL);
#endif
}

int _kbhit()
{
	int bytesWaiting;

	ioctl(STDIN_FILENO, FIONREAD, &bytesWaiting);

	if(bytesWaiting > 0)
	{
		return 1;
	}

	return 0;
}

int _getch()
{
	return (int)getchar();
}

void restoreTerminal()
{
	/* Use termios to restore original terminal attributes */
	tcsetattr(STDIN_FILENO, TCSANOW, &gOldTerm);
}

#endif

#include "eglutils.h"
#include "maths.h"

#define TEX_SIZE 32

#define WINDOW_WIDTH 200
#define WINDOW_HEIGHT 200
#define PBUFFER 1
#define CHECKFILE 1
#define APPHINT_MAX_STRING_SIZE 256

#define INFO  printf
#define ERROR printf

#define DEFAULT_FRAME_STOP 100

#define PBUFFER 1

static int frameStop;

static int mvp_pos[2];
static int hProgramHandle[2];
static int attriblocation[2];

static GLfloat projection[4][4] = {{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}};
static GLfloat modelview[4][4] = {{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}};
static GLfloat mvp[4][4] = {{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}};

#define handle_egl_error(x, y)  \
{																\
	EGLint error_code = eglGetError();							\
	ERROR("'%s' returned egl error '%s' (0x%x)\n",				\
		   x, GetEGLErrorString(error_code), error_code);	\
    if (y != NULL) {								\
        strcpy(y, GetEGLErrorString(error_code));	\
    }															\
	return EGL_ERROR;											\
}


static char const *const apszProgramFileName[] =
{
	SHADER_DIR "gles3test1_vertshader.txt",
	SHADER_DIR "gles3test1_fragshaderA.txt",
	SHADER_DIR "gles3test1_fragshaderB.txt"
};

#define NFILES (int)(sizeof(apszProgramFileName)/sizeof(apszProgramFileName[0]))

#ifdef FILES_EMBEDDED

#ifndef __unused
#define __unused
#endif

#include "gles3test1_vertshader.txt.h"
#include "gles3test1_fragshaderA.txt.h"
#include "gles3test1_fragshaderB.txt.h"

static char const * const apszFiles[NFILES]=
{
	gles3test1_vertshader,
	gles3test1_fragshaderA,
	gles3test1_fragshaderB
};

static EGLBoolean file_load(int i, char **pcData, int *piLen)
{
	if (i < 0 || i >= NFILES || !pcData || !piLen)
	{
		return GL_FALSE;
	}

	*pcData = (char *)apszFiles[i];
	*piLen = strlen(apszFiles[i]);

	return GL_TRUE;
}

static void file_unload(char __unused *pData) {}

#else /* FILES_EMBEDDED */

static EGLBoolean file_load(int i, char **pcData, int *piLen)
{
	FILE *fpShader;
	int iLen,iGot;

	if (i < 0 || i >= NFILES || !pcData || !piLen)
	{
		return GL_FALSE;
	}

	/* open the shader file */
	fpShader = fopen(apszProgramFileName[i], "r");

	/* Check open succeeded */
	if (!fpShader)
	{
		ERROR("Error: Failed to open shader file '%s'!\n", apszProgramFileName[i]);

		return GL_FALSE;
	}

	/* To get size of file, seek to end, ftell, then rewind */
	fseek(fpShader, 0, SEEK_END);
	iLen = ftell(fpShader);
	fseek(fpShader, 0, SEEK_SET);

	*pcData = (char *)malloc(iLen + 1);

	if (*pcData == NULL)
	{
		ERROR("Error: Failed to allocate %d bytes for program '%s'!\n", iLen + 1, apszProgramFileName[i]);

		fclose(fpShader);

		return GL_FALSE;
	}

	/* Read the file into the buffer */
	iGot = fread(*pcData, 1, iLen, fpShader);

	if (iGot != iLen)
	{
		// Might be ASCII vs Binary
		ERROR("Warning: Only read %u bytes of %d from '%s'!\n", iGot, iLen, apszProgramFileName[i]);
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

#endif /* FILES_EMBEDDED */

static int init(void)
{
	static GLfloat vertices[] =
	   {-0.5f,-0.5f, 0.0f, 1.0f,
		 0.0f, 0.5f, 0.0f, 1.0f,
		 0.5f,-0.5f, 0.0f, 1.0f,
	    -0.5f,-0.5f, 0.0f, 1.0f,
	     0.0f, 0.5f, 0.0f, 1.0f,
	     0.5f,-0.5f, 0.0f, 1.0f};

	static GLfloat colors[] = {0.66f, 0.66f, 0.66f,1.0f,
							   0.33f, 0.33f, 0.33f,1.0f,
							   1.0f, 1.0f, 1.0f,1.0f,
							   1.0f, 0.4f, 0.4f,1.0f,
							   0.3f, 0.4f, 1.0f,1.0f,
							   0.7f, 1.0f, 0.4f,1.0f};

	static GLfloat texcoord[] = {0.0, 0.0, 0.0, 1.0,
								 1.0, 0.0, 0.0, 1.0,
								 1.0, 1.0, 0.0, 1.0,
								 0.0, 0.0, 0.0, 1.0,
								 1.0, 0.0, 0.0, 1.0,
								 1.0, 1.0, 0.0, 1.0};

	char aszInfoLog[1024];
	int  nShaderStatus, nInfoLogLength;
	int hShaderHandle[3];
	int basetexture_pos;
	GLubyte texdata[TEX_SIZE*TEX_SIZE*4];
	GLubyte *lpTex = texdata;
	GLuint i,j;

	glClearColor (0.5, 0.0, 0.0, 1.0);

	for (j=0; j<TEX_SIZE; j++)
	{
		for (i=0; i<TEX_SIZE; i++)
		{
			if ((i ^ j) & 0x8)
			{
				lpTex[0] = lpTex[1] = lpTex[2] = 0x00;
				/* Set full alpha */
				lpTex[3] = 0xff;
			}
			else
			{
				lpTex[0] = lpTex[1] = lpTex[2] = lpTex[3] = 0xFF;
			}

			lpTex += 4;
		}
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEX_SIZE, TEX_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, texdata);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	/* Create a program object */
	hShaderHandle[0] = glCreateShader(GL_VERTEX_SHADER);
	hShaderHandle[1] = glCreateShader(GL_FRAGMENT_SHADER);
	hShaderHandle[2] = glCreateShader(GL_FRAGMENT_SHADER);
	hProgramHandle[0] = glCreateProgram();
	hProgramHandle[1] = glCreateProgram();

	for (i=0; i<NFILES ; i++)
	{
		char *pszProgramString;
		int  nProgramLength;

		if (file_load(i,&pszProgramString,&nProgramLength)==GL_FALSE)
		{
			return GL_FALSE;
		}

		snprintf(aszInfoLog, sizeof(aszInfoLog), "Compiling program '%s', %d bytes long\n", apszProgramFileName[i], nProgramLength);

		INFO("%s", aszInfoLog);

		/* Supply shader source */
		glShaderSource(hShaderHandle[i], 1, (const char **)&pszProgramString, &nProgramLength);

		/* Free the program string */
		file_unload(pszProgramString);

		/* Compile the shader */
		glCompileShader(hShaderHandle[i]);

		/* Check it compiled OK */
		glGetShaderiv(hShaderHandle[i], GL_COMPILE_STATUS, &nShaderStatus);

		if (nShaderStatus != GL_TRUE)
		{
			ERROR("Error: Failed to compile GLSL shader\n");

			glGetShaderInfoLog(hShaderHandle[i], 1024, &nInfoLogLength, aszInfoLog);

			INFO("%s", aszInfoLog);

			return GL_FALSE;
		}
	}

	/* Attach the shader to the programs */
	glAttachShader(hProgramHandle[0], hShaderHandle[0]);
	glAttachShader(hProgramHandle[0], hShaderHandle[1]);

	glAttachShader(hProgramHandle[1], hShaderHandle[0]);
	glAttachShader(hProgramHandle[1], hShaderHandle[2]);

	for (i=0; i < 2; i++)
	{
		glBindAttribLocation(hProgramHandle[i], 0, "position");
		glBindAttribLocation(hProgramHandle[i], 1, "inputcolor");

		/* Link the program */
		glLinkProgram(hProgramHandle[i]);

		/* Check it linked OK */
		glGetProgramiv(hProgramHandle[i], GL_LINK_STATUS, &nShaderStatus);

		if (nShaderStatus != GL_TRUE)
		{
			ERROR("Error: Failed to link GLSL program\n");

			glGetProgramInfoLog(hProgramHandle[i], 1024, &nInfoLogLength, aszInfoLog);

			INFO("%s", aszInfoLog);

			return GL_FALSE;
		}

		glValidateProgram(hProgramHandle[i]);

		glGetProgramiv(hProgramHandle[i], GL_VALIDATE_STATUS, &nShaderStatus);

		if (nShaderStatus != GL_TRUE)
		{
			ERROR("Error: Failed to validate GLSL program\n");

			glGetProgramInfoLog(hProgramHandle[i], 1024, &nInfoLogLength, aszInfoLog);

			INFO("%s", aszInfoLog);

			return GL_FALSE;
		}

		mvp_pos[i] = glGetUniformLocation(hProgramHandle[i], "mvp");
		basetexture_pos = glGetUniformLocation(hProgramHandle[i], "basetexture");

		glUseProgram(hProgramHandle[i]);

		glUniform1i(basetexture_pos, 0);
	}

	j = glGetError();

	if (j != GL_NO_ERROR)
	{
		ERROR("GL ERROR = %x", j);

		return GL_FALSE;
	}

	glEnableVertexAttribArray (0);
	glEnableVertexAttribArray (1);

	glVertexAttribPointer(0, 4, GL_FLOAT, 0, 0, vertices);
	glVertexAttribPointer(1, 4, GL_FLOAT, 0, 0, colors);

	/* Texturing - the right hand side triangle */
	attriblocation[1] = glGetAttribLocation(hProgramHandle[1], "inputtexcoord");
	glVertexAttribPointer(attriblocation[1], 4, GL_FLOAT, 0, 0, texcoord);
	glEnableVertexAttribArray(attriblocation[1]);

	myIdentity(projection);
	myPersp(projection,  60.0f,	1.0f, 0.1f,	100);

	return GL_TRUE;
}

static void display(void)
{
	static int framecount=0;

	glClear (GL_COLOR_BUFFER_BIT);

	glUseProgram(hProgramHandle[0]);

	myIdentity(modelview);

	myTranslate(modelview, -0.5, 0, -2.0f);

	myRotate(modelview, 0.0f, 1.0f, 0.0f, 5.0f * framecount);

	myMultMatrix(mvp, modelview, projection);

	glUniformMatrix4fv(mvp_pos[0], 1, GL_FALSE, &mvp[0][0]);

	glDrawArrays (GL_TRIANGLES, 0, 3);

	glUseProgram(hProgramHandle[1]);

	myIdentity(modelview);

	myTranslate(modelview, 0.5, 0, -2.0);

	myRotate(modelview, 0, 1, 0, -5.0f * framecount);

	myMultMatrix(mvp, modelview, projection);

	glUniformMatrix4fv(mvp_pos[1], 1, GL_FALSE, &mvp[0][0]);

	glDrawArrays (GL_TRIANGLES, 3, 3);

	framecount++;
}


/***********************************************************************************
 Function Name      : EglMainArgs
 Inputs             : eglDisplay, eglWindow
 Outputs            : None
 Returns            : Error
 Description        : EGL portion of 'main' function
************************************************************************************/
int Gles3_EglMainArgs(EGLNativeDisplayType eglDisplay, char* result_string)
{
	EGLDisplay dpy;
	EGLSurface surface;
	EGLContext context;
	EGLConfig configs[2];
	EGLBoolean eRetStatus;
	int ret = 0;
	EGLint major, minor;
	EGLint context_attribs[] = {EGL_CONTEXT_MAJOR_VERSION_KHR, 3,
		                        EGL_CONTEXT_MINOR_VERSION_KHR, 0,
		                        EGL_NONE};
	EGLint config_count;
    EGLint cfg_attribs[] = {EGL_BUFFER_SIZE,    EGL_DONT_CARE,
							EGL_DEPTH_SIZE,		16,
							EGL_RED_SIZE,       5,
							EGL_GREEN_SIZE,     6,
							EGL_BLUE_SIZE,      5,
#if defined(PBUFFER)
							EGL_SURFACE_TYPE,   EGL_PBUFFER_BIT,
#endif
							EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
							EGL_NONE};

	int i;
#if defined(PBUFFER)
	GLubyte *readbuf = NULL;
#if defined(CHECKFILE)
	GLubyte *checkbuf = NULL;
#endif
#endif

	INFO("--------------------- started ---------------------\n");

	frameStop = DEFAULT_FRAME_STOP;

	dpy = eglGetDisplay(eglDisplay);

	eRetStatus = eglInitialize(dpy, &major, &minor);
	if (eRetStatus != EGL_TRUE) {
		handle_egl_error("eglInitialize", result_string);
	}
	INFO("eglInitialize: eRetStatus = 0x%x, major = %d, minor = %d.\n",eRetStatus, major, minor);

	eRetStatus = eglChooseConfig (dpy, cfg_attribs, configs, 2, &config_count);
	if (!eRetStatus)
	{
		handle_egl_error ("eglChooseConfig", result_string);
	}
	else if (!config_count)
	{
		INFO("eglChooseConfig: no matching configs were returned by EGL (exiting).\n");
		return EGL_ERROR;
	}

#if defined(PBUFFER)
	{
		EGLint pbuf[] = {EGL_WIDTH, WINDOW_WIDTH, EGL_HEIGHT, WINDOW_HEIGHT, EGL_NONE};

		surface = eglCreatePbufferSurface (dpy, configs[0], pbuf);
		if (surface == EGL_NO_SURFACE)
		{
			handle_egl_error ("eglCreatePbufferSurface", result_string);
		}
	}
	readbuf = malloc(WINDOW_WIDTH*WINDOW_HEIGHT*4);
#if defined(CHECKFILE)
	checkbuf = malloc(WINDOW_WIDTH*WINDOW_HEIGHT*4);
#endif
#else

	surface = eglCreateWindowSurface(dpy, configs[0], eglWindow, NULL);
	if (surface == EGL_NO_SURFACE)
	{
		handle_egl_error ("eglCreateWindowSurface", result_string);
	}
#endif

	eRetStatus = eglBindAPI(EGL_OPENGL_ES_API);
	if (eRetStatus != EGL_TRUE)
	{
		handle_egl_error ("eglBindAPI", result_string);
	}

	context = eglCreateContext (dpy, configs[0], EGL_NO_CONTEXT, context_attribs);
	if (context == EGL_NO_CONTEXT)
	{
		handle_egl_error ("eglCreateContext", result_string);
	}

	eRetStatus = eglMakeCurrent (dpy, surface, surface, context);
	if( eRetStatus != EGL_TRUE )
		handle_egl_error ("eglMakeCurrent", result_string);

	if (init() == GL_FALSE) {
		ret = INIT_FAIL;
		goto term;
	}

	for (i=0; !frameStop || (i < frameStop); i++)
	{
		display();
#if defined(PBUFFER)
		glReadPixels(0,0,WINDOW_WIDTH, WINDOW_HEIGHT,GL_RGBA,GL_UNSIGNED_BYTE, readbuf);
#if defined(SAVEFILE)
		{
			char szFileName[APPHINT_MAX_STRING_SIZE + 4];
			FILE *fp;
			snprintf(szFileName, sizeof(szFileName), "/usr/local/data/gles3test1_dump_%d.dat",
									i);

			fp = fopen(szFileName, "wb");

			if(fp == NULL)
			{
					ERROR("Error: failed to open file %s for writing\n",
										szFileName);
			}
			else
			{
				INFO("Dumping buffer to raw file %s ...\n", szFileName);
				fwrite(readbuf, 1, WINDOW_WIDTH*WINDOW_HEIGHT*4, fp);
				INFO("Done\n");

				fclose(fp);
			}

		}
#endif//#if defined(SAVEFILE)
#if defined(CHECKFILE)
		{
			char szFileName[APPHINT_MAX_STRING_SIZE + 4];
			FILE *fp;
			snprintf(szFileName, sizeof(szFileName), "/usr/local/data/gles3test1_dump_%d.dat",
									i);

			fp = fopen(szFileName, "rb");

			if(fp == NULL)
			{
					ERROR("Error: failed to open file %s for checking\n",
										szFileName);
					ret = ENV_ERROR;
			}
			else
			{
				INFO("Checking buffer with raw file %s ...\n", szFileName);
				fread(checkbuf, 1, WINDOW_WIDTH*WINDOW_HEIGHT*4, fp);
				fclose(fp);
				if (memcmp(checkbuf, readbuf, WINDOW_WIDTH*WINDOW_HEIGHT*4) != 0)
				{
					ret = ERR_FRAME(i);
					INFO("check file %d failed\n", i);
					break;
				}
				INFO("Done\n");
			}

		}
#endif//#if defined(SAVEFILE)

#else
		eglSwapBuffers (dpy, surface);
#endif
	}

term:
	eglDestroyContext (dpy, context);
	eglDestroySurface (dpy, surface);
	eglMakeCurrent (dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglTerminate (dpy);

	INFO("--------------------- finished ---------------------\n");
	return ret;
}

