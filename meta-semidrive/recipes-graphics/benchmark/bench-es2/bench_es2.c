#include <unistd.h>
#include <sys/time.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

typedef unsigned char uint8;    /*<!  8 bit unsigned integer. */
typedef unsigned short uint16;  /*<! 16 bit unsigned integer. */
typedef unsigned int uint32;    /*<! 32 bit unsigned integer. */
typedef unsigned long long uint64;      /*<! 64 bit unsigned integer. */
typedef signed char int8;       /*<!  8 bit signed integer. */
typedef signed short int16;     /*<! 16 bit signed integer. */
typedef signed int int32;       /*<! 32 bit signed integer. */
typedef signed long long int64; /*<! 64 bit signed integer. */
typedef float float32;          /*<! 32 bit floating point value. */
typedef struct OSHelperTimerInfo
{
    long long timeMicroseconds; /*!< Stores the timer start time or stop time. */
} OSHelperTimerInfo;
static OSHelperTimerInfo iStart, iEnd;

void OsHelperGetTime( OSHelperTimerInfo *curTime )
{
    struct timeval t;
    gettimeofday( &t, NULL );
    curTime->timeMicroseconds = t.tv_sec * 1000000 + t.tv_usec;
}

float OsHelperDurationSec(OSHelperTimerInfo *start, OSHelperTimerInfo *stop)
{
	return (stop->timeMicroseconds - start->timeMicroseconds) * 0.000001f;
}

#define WINDOW_WIDTH    1024
#define WINDOW_HEIGHT   768
#define BPP             16

#define PRINT_HALF_TEXT 1    // 0 to just print result numbers, 1 to print descriptions as well
#define HALF_WIDTH      ((GLfloat)(WINDOW_WIDTH/2))
#define HALF_HEIGHT     ((GLfloat)(WINDOW_HEIGHT/2))

#define DISPLAY_WIDTH      1024
#define DISPLAY_HEIGHT     768
#define DISPLAY_COLOR_DEPTH     32
#define DISPLAY_SCREEN_REFRESH  60
#define DISPLAY_INTERLACED      FALSE
#define DISPLAY_SCREEN_HEAD     OUTPUT_HEAD_DEFAULT
#define DISPLAY_OUTPUT_TYPE     OUTPUT_TYPE_DEFAULT
#define DISPLAY_CARD_ID         0

static EGLDisplay      f_dpy;               /* Native display */
static EGLContext      f_eglContext;        /* EGL Context */
static EGLConfig       f_config;            /* EGL Config */
static EGLSurface      f_surface;           /* EGL Surface */

static GLuint shaderProgram, shaderPointsProgram, shaderProgramHalf, shaderPointsProgramHalf;
static double secs, calibrate = 1.0;

static GLboolean half = GL_FALSE;
static enum RenderMode {ARRAY, ELEMENTS, VBO_ARRAY, VBO_ELEMENTS, ARRAY_VBO_ELEMENTS, VBO_ARRAY_ELEMENTS, MAX_RENDER_MODE} mode = ARRAY;
static int polygonCount, arrayCount, vertCount;
static GLfloat vertices[512][2];
static uint16 hVertices[512][2];
static GLushort indices[512];
static struct
{
    GLint first;
    GLint count;
} arrayList[512];

static char pointData[128],
            line1Data[128],    lineStrip1Data[128], lineLoop1Data[128],
            line50Data[128], lineStrip50Data[128], lineLoop50Data[128],
            tri1Data[128], triStrip1Data[128], triFan1Data[128],
            tri50Data[128], triStrip50Data[128], triFan50Data[128],
            tri10000Data[128], triStrip10000Data[128], triFan10000Data[128],
            clearData[128], bitmapData[128], drawPixelData[128],
            dispFormat[128],
            *txtPtr;


static void InitGL(void);
static uint16 S_UtilFP32ToFP16(float32 in);

// for clears, bitmaps, etc
static void RunTest(void Draw(), int loop)
{
    int j;
    polygonCount = 0;
    arrayCount = 0;
    vertCount = 0;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glFinish();

    OsHelperGetTime(&iStart);
    for (j = 0; j < loop; j++)
    {
         Draw();
    }
    glFinish();
    OsHelperGetTime(&iEnd);

    eglSwapBuffers(f_dpy, f_surface);
    secs = OsHelperDurationSec(&iStart, &iEnd);
}

// for vertices
static void RunTestV(GLuint DrawSetup(), int loop)
{
    int i, j;
    GLuint vbo = 0, ibo = 0, primType = 0;
    polygonCount = 0;
    arrayCount = 0;
    vertCount = 0;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    primType = DrawSetup();
    for (i = 0; i < vertCount; i++)
    {
        vertices[i][0] = (vertices[i][0] / HALF_WIDTH)  - 1.0f;
        vertices[i][1] = (vertices[i][1] / HALF_HEIGHT) - 1.0f;
        hVertices[i][0] = S_UtilFP32ToFP16(vertices[i][0]);
        hVertices[i][1] = S_UtilFP32ToFP16(vertices[i][1]);
    }
    if (primType == GL_POINTS)
    {
        if (half)
            glUseProgram(shaderPointsProgramHalf);
        else
            glUseProgram(shaderPointsProgram);
    }
    else
    {
        if (half)
            glUseProgram(shaderProgramHalf);
        else
            glUseProgram(shaderProgram);
    }
    glFinish();
    switch (mode)
    {
    case ARRAY:
        OsHelperGetTime(&iStart);
        for (i = 0; i < loop; i++)
        {
            for (j = 0; j < arrayCount; j++)
            {
                glDrawArrays(primType, arrayList[j].first, arrayList[j].count);
            }
        }
        glFinish();
        OsHelperGetTime(&iEnd);
        polygonCount *= loop;
        break;

    case ELEMENTS:
        OsHelperGetTime(&iStart);
        for (i = 0; i < loop; i++)
        {
            for (j = 0; j < arrayCount; j++)
            {
                glDrawElements(primType, arrayList[j].count, GL_UNSIGNED_SHORT, &indices[arrayList[j].first]);
            }
        }
        glFinish();
        OsHelperGetTime(&iEnd);
        polygonCount *= loop;
        break;

    case VBO_ARRAY:
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        if (half)
        {
            glBufferData(GL_ARRAY_BUFFER, 2 * vertCount * sizeof(uint16), hVertices, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 2, GL_HALF_FLOAT_OES, GL_FALSE, 0, NULL);
        }
        else
        {
            glBufferData(GL_ARRAY_BUFFER, 2 * vertCount * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        }
        glFinish();
        OsHelperGetTime(&iStart);
        for (i = 0; i < loop; i++)
        {
            for (j = 0; j < arrayCount; j++)
            {
                glDrawArrays(primType, arrayList[j].first, arrayList[j].count);
            }
        }
        glFinish();
        OsHelperGetTime(&iEnd);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDeleteBuffers(1, &vbo);
        polygonCount *= loop;
        break;

    case VBO_ELEMENTS:
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ibo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        if (half)
        {
            glBufferData(GL_ARRAY_BUFFER, 2 * vertCount * sizeof(uint16), hVertices, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 2, GL_HALF_FLOAT_OES, GL_FALSE, 0, NULL);
        }
        else
        {
            glBufferData(GL_ARRAY_BUFFER, 2 * vertCount * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        }
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertCount * sizeof(GLushort), indices, GL_STATIC_DRAW);
        glFinish();
        OsHelperGetTime(&iStart);
        for (i = 0; i < loop; i++)
        {
            for (j = 0; j < arrayCount; j++)
            {
                glDrawElements(primType, arrayList[j].count, GL_UNSIGNED_SHORT, (const void*)(arrayList[j].first * sizeof(GLushort)));
            }
        }
        glFinish();
        OsHelperGetTime(&iEnd);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ibo);
        polygonCount *= loop;
        break;

    case ARRAY_VBO_ELEMENTS:

        glGenBuffers(1, &ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

        glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertCount * sizeof(GLushort), indices, GL_STATIC_DRAW);
        glFinish();
        OsHelperGetTime(&iStart);
        for (i = 0; i < loop; i++)
        {
            for (j = 0; j < arrayCount; j++)
            {
                glDrawElements(primType, arrayList[j].count, GL_UNSIGNED_SHORT, (const void*)(arrayList[j].first * sizeof(GLushort)));
            }
        }
        glFinish();
        OsHelperGetTime(&iEnd);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        glDeleteBuffers(1, &ibo);
        polygonCount *= loop;
        break;

    case VBO_ARRAY_ELEMENTS:
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        if (half)
        {
            glBufferData(GL_ARRAY_BUFFER, 2 * vertCount * sizeof(uint16), hVertices, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 2, GL_HALF_FLOAT_OES, GL_FALSE, 0, NULL);
        }
        else
        {
            glBufferData(GL_ARRAY_BUFFER, 2 * vertCount * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        }

        glFinish();
        OsHelperGetTime(&iStart);
        for (i = 0; i < loop; i++)
        {
            for (j = 0; j < arrayCount; j++)
            {
                glDrawElements(primType, arrayList[j].count, GL_UNSIGNED_SHORT, &indices[arrayList[j].first]);
            }
        }
        glFinish();
        OsHelperGetTime(&iEnd);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDeleteBuffers(1, &vbo);
        polygonCount *= loop;
        break;

    }



    eglSwapBuffers(f_dpy, f_surface);
    secs = OsHelperDurationSec(&iStart, &iEnd);
}

static void DrawClear()
{
#if (BPP == 16)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#else
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
#endif
    polygonCount++;
}

static GLuint DrawPoint()
{
    float i;
    arrayList[0].first = 0;
    arrayList[0].count = 0;
    for (i = 20.0f; i < 620.0f; i += 3.0f)
    {
        vertices[vertCount][0] = i; vertices[vertCount][1] = 242.0f; vertCount++;
        vertices[vertCount][0] = i; vertices[vertCount][1] = 244.0f; vertCount++;
        polygonCount++;
        polygonCount++;
    }
    arrayList[0].count = vertCount;
    arrayCount = 1;
    return GL_POINTS;
}

static GLuint DrawLine1()
{
    float i;
    for (i = 20.0f; i < 620.0f; i += 60.0f)
    {
        arrayList[arrayCount].first = vertCount;
        vertices[vertCount][0] = i;      vertices[vertCount][1] = 240.0f; vertCount++;
        vertices[vertCount][0] = i+1.0f; vertices[vertCount][1] = 240.0f; vertCount++;
        vertices[vertCount][0] = i+1.0f; vertices[vertCount][1] = 240.0f; vertCount++;
        vertices[vertCount][0] = i+2.0f; vertices[vertCount][1] = 240.0f; vertCount++;
        vertices[vertCount][0] = i+2.0f; vertices[vertCount][1] = 240.0f; vertCount++;
        vertices[vertCount][0] = i+3.0f; vertices[vertCount][1] = 240.0f; vertCount++;
        vertices[vertCount][0] = i+3.0f; vertices[vertCount][1] = 240.0f; vertCount++;
        vertices[vertCount][0] = i+4.0f; vertices[vertCount][1] = 240.0f; vertCount++;
        vertices[vertCount][0] = i+4.0f; vertices[vertCount][1] = 240.0f; vertCount++;
        vertices[vertCount][0] = i+5.0f; vertices[vertCount][1] = 240.0f; vertCount++;
        vertices[vertCount][0] = i;      vertices[vertCount][1] = 240.0f; vertCount++;
        vertices[vertCount][0] = i;      vertices[vertCount][1] = 241.0f; vertCount++;
        vertices[vertCount][0] = i;      vertices[vertCount][1] = 241.0f; vertCount++;
        vertices[vertCount][0] = i;      vertices[vertCount][1] = 242.0f; vertCount++;
        vertices[vertCount][0] = i;      vertices[vertCount][1] = 242.0f; vertCount++;
        vertices[vertCount][0] = i;      vertices[vertCount][1] = 243.0f; vertCount++;
        vertices[vertCount][0] = i;      vertices[vertCount][1] = 243.0f; vertCount++;
        vertices[vertCount][0] = i;      vertices[vertCount][1] = 244.0f; vertCount++;
        vertices[vertCount][0] = i;      vertices[vertCount][1] = 244.0f; vertCount++;
        vertices[vertCount][0] = i;      vertices[vertCount][1] = 245.0f; vertCount++;
        vertices[vertCount][0] = i;      vertices[vertCount][1] = 240.0f; vertCount++;
        vertices[vertCount][0] = i;      vertices[vertCount][1] = 239.0f; vertCount++;
        vertices[vertCount][0] = i;      vertices[vertCount][1] = 239.0f; vertCount++;
        vertices[vertCount][0] = i;      vertices[vertCount][1] = 238.0f; vertCount++;
        vertices[vertCount][0] = i;      vertices[vertCount][1] = 238.0f; vertCount++;
        vertices[vertCount][0] = i;      vertices[vertCount][1] = 237.0f; vertCount++;
        vertices[vertCount][0] = i;      vertices[vertCount][1] = 237.0f; vertCount++;
        vertices[vertCount][0] = i;      vertices[vertCount][1] = 236.0f; vertCount++;
        vertices[vertCount][0] = i;      vertices[vertCount][1] = 236.0f; vertCount++;
        vertices[vertCount][0] = i;      vertices[vertCount][1] = 235.0f; vertCount++;
        vertices[vertCount][0] = i;      vertices[vertCount][1] = 240.0f; vertCount++;
        vertices[vertCount][0] = i+1.0f; vertices[vertCount][1] = 241.0f; vertCount++;
        vertices[vertCount][0] = i+1.0f; vertices[vertCount][1] = 241.0f; vertCount++;
        vertices[vertCount][0] = i+2.0f; vertices[vertCount][1] = 242.0f; vertCount++;
        vertices[vertCount][0] = i+2.0f; vertices[vertCount][1] = 242.0f; vertCount++;
        vertices[vertCount][0] = i+3.0f; vertices[vertCount][1] = 243.0f; vertCount++;
        vertices[vertCount][0] = i+3.0f; vertices[vertCount][1] = 243.0f; vertCount++;
        vertices[vertCount][0] = i+4.0f; vertices[vertCount][1] = 244.0f; vertCount++;
        vertices[vertCount][0] = i+4.0f; vertices[vertCount][1] = 244.0f; vertCount++;
        vertices[vertCount][0] = i+5.0f; vertices[vertCount][1] = 245.0f; vertCount++;
        vertices[vertCount][0] = i;      vertices[vertCount][1] = 240.0f; vertCount++;
        vertices[vertCount][0] = i+1.0f; vertices[vertCount][1] = 239.0f; vertCount++;
        vertices[vertCount][0] = i+1.0f; vertices[vertCount][1] = 239.0f; vertCount++;
        vertices[vertCount][0] = i+2.0f; vertices[vertCount][1] = 238.0f; vertCount++;
        vertices[vertCount][0] = i+2.0f; vertices[vertCount][1] = 238.0f; vertCount++;
        vertices[vertCount][0] = i+3.0f; vertices[vertCount][1] = 237.0f; vertCount++;
        vertices[vertCount][0] = i+3.0f; vertices[vertCount][1] = 237.0f; vertCount++;
        vertices[vertCount][0] = i+4.0f; vertices[vertCount][1] = 236.0f; vertCount++;
        vertices[vertCount][0] = i+4.0f; vertices[vertCount][1] = 236.0f; vertCount++;
        vertices[vertCount][0] = i+5.0f; vertices[vertCount][1] = 235.0f; vertCount++;
        polygonCount += 25;
        arrayList[arrayCount].count = 50;
        arrayCount++;
    }
    return GL_LINES;
}

static GLuint DrawLine50()
{
    float i;
    arrayList[0].first = 0;
    arrayList[0].count = 0;
    for (i = 20.0f; i < 620.0f; i += 50.0f)
    {
        vertices[vertCount][0] = i;       vertices[vertCount][1] = 240.0f; vertCount++;
        vertices[vertCount][0] = i+50.0f; vertices[vertCount][1] = 240.0f; vertCount++;
        vertices[vertCount][0] = i;       vertices[vertCount][1] = 240.0f; vertCount++;
        vertices[vertCount][0] = i;       vertices[vertCount][1] = 290.0f; vertCount++;
        vertices[vertCount][0] = i;       vertices[vertCount][1] = 240.0f; vertCount++;
        vertices[vertCount][0] = i;       vertices[vertCount][1] = 190.0f; vertCount++;
        vertices[vertCount][0] = i;       vertices[vertCount][1] = 240.0f; vertCount++;
        vertices[vertCount][0] = i+35.4f; vertices[vertCount][1] = 275.4f; vertCount++;
        vertices[vertCount][0] = i;       vertices[vertCount][1] = 240.0f; vertCount++;
        vertices[vertCount][0] = i+35.4f; vertices[vertCount][1] = 204.6f; vertCount++;
        polygonCount += 5;
    }
    arrayList[0].count = vertCount;
    arrayCount = 1;
    return GL_LINES;
}

static GLuint DrawLineStrip1()
{
    float i, j;
    int vertCountPerArray = 0;
    for (j = 225.0f; j < 265.0f; j += 10.0f)
    {
        arrayList[arrayCount].first = vertCount;
        for (i = 260.0f; i < 380.0f; i+=8)
        {
            vertices[vertCount][0] = i;      vertices[vertCount][1] = j-2.0f; vertCount++;
            vertices[vertCount][0] = i+1.0f; vertices[vertCount][1] = j-1.0f; vertCount++;
            vertices[vertCount][0] = i+2.0f; vertices[vertCount][1] = j;      vertCount++;
            vertices[vertCount][0] = i+3.0f; vertices[vertCount][1] = j+1.0f; vertCount++;
            vertices[vertCount][0] = i+4.0f; vertices[vertCount][1] = j+2.0f; vertCount++;
            vertices[vertCount][0] = i+5.0f; vertices[vertCount][1] = j+1.0f; vertCount++;
            vertices[vertCount][0] = i+6.0f; vertices[vertCount][1] = j;      vertCount++;
            vertices[vertCount][0] = i+7.0f; vertices[vertCount][1] = j-1.0f; vertCount++;
        }
        vertices[vertCount][0] = 380.0f; vertices[vertCount][1] = j-2.0f; vertCount++;
        if (vertCountPerArray == 0) vertCountPerArray = vertCount;
        polygonCount += vertCountPerArray-1;
        arrayList[arrayCount].count = vertCountPerArray;
        arrayCount++;
    }
    return GL_LINE_STRIP;
}

static GLuint DrawLineStrip50()
{
    float i;
    for (i = 20.0f; i < 500.0f; i += 60.0f)
    {
        arrayList[arrayCount].first = vertCount;
        vertices[vertCount][0] = i;        vertices[vertCount][1] = 240.0f; vertCount++;
        vertices[vertCount][0] = i+ 50.0f; vertices[vertCount][1] = 240.0f; vertCount++;
        vertices[vertCount][0] = i+ 90.0f; vertices[vertCount][1] = 270.0f; vertCount++;
        vertices[vertCount][0] = i+120.0f; vertices[vertCount][1] = 310.0f; vertCount++;
        vertices[vertCount][0] = i+120.0f; vertices[vertCount][1] = 360.0f; vertCount++;
        vertices[vertCount][0] = i+120.0f; vertices[vertCount][1] = 410.0f; vertCount++;
        vertices[vertCount][0] = i+120.0f; vertices[vertCount][1] = 460.0f; vertCount++;
        vertices[vertCount][0] = i+ 90.0f; vertices[vertCount][1] = 420.0f; vertCount++;
        vertices[vertCount][0] = i+ 90.0f; vertices[vertCount][1] = 370.0f; vertCount++;
        vertices[vertCount][0] = i+ 90.0f; vertices[vertCount][1] = 320.0f; vertCount++;
        vertices[vertCount][0] = i+ 60.0f; vertices[vertCount][1] = 280.0f; vertCount++;
        vertices[vertCount][0] = i+ 30.0f; vertices[vertCount][1] = 240.0f; vertCount++;
        polygonCount += 11;
        arrayList[arrayCount].count = 12;
        arrayCount++;
    }
    return GL_LINE_STRIP;
}

static GLuint DrawLineLoop1()
{
    float i, j;
    int vertCountPerArray = 0;
    for (j = 100.0f; j <= 500.0f; j += 200.0f)
    {
        arrayList[arrayCount].first = vertCount;
        for (i = 0.0f; i < 40.0f; i+=8)
        {
            vertices[vertCount][0] = j+i;      vertices[vertCount][1] = 238.0f; vertCount++;
            vertices[vertCount][0] = j+i+1.0f; vertices[vertCount][1] = 239.0f; vertCount++;
            vertices[vertCount][0] = j+i+2.0f; vertices[vertCount][1] = 240.0f; vertCount++;
            vertices[vertCount][0] = j+i+3.0f; vertices[vertCount][1] = 241.0f; vertCount++;
            vertices[vertCount][0] = j+i+4.0f; vertices[vertCount][1] = 242.0f; vertCount++;
            vertices[vertCount][0] = j+i+5.0f; vertices[vertCount][1] = 241.0f; vertCount++;
            vertices[vertCount][0] = j+i+6.0f; vertices[vertCount][1] = 240.0f; vertCount++;
            vertices[vertCount][0] = j+i+7.0f; vertices[vertCount][1] = 239.0f; vertCount++;
        }
        for (i = 0.0f; i < 40.0f; i+=8)
        {
            vertices[vertCount][0] = j+40.0f; vertices[vertCount][1] = 238.0f-i; vertCount++;
            vertices[vertCount][0] = j+41.0f; vertices[vertCount][1] = 237.0f-i; vertCount++;
            vertices[vertCount][0] = j+42.0f; vertices[vertCount][1] = 236.0f-i; vertCount++;
            vertices[vertCount][0] = j+43.0f; vertices[vertCount][1] = 235.0f-i; vertCount++;
            vertices[vertCount][0] = j+44.0f; vertices[vertCount][1] = 234.0f-i; vertCount++;
            vertices[vertCount][0] = j+43.0f; vertices[vertCount][1] = 233.0f-i; vertCount++;
            vertices[vertCount][0] = j+42.0f; vertices[vertCount][1] = 232.0f-i; vertCount++;
            vertices[vertCount][0] = j+41.0f; vertices[vertCount][1] = 231.0f-i; vertCount++;
        }
        for (i = 40.0f; i > 0.0f; i-=8)
        {
            vertices[vertCount][0] = j+i;      vertices[vertCount][1] = 198.0f; vertCount++;
            vertices[vertCount][0] = j+i-1.0f; vertices[vertCount][1] = 197.0f; vertCount++;
            vertices[vertCount][0] = j+i-2.0f; vertices[vertCount][1] = 196.0f; vertCount++;
            vertices[vertCount][0] = j+i-3.0f; vertices[vertCount][1] = 195.0f; vertCount++;
            vertices[vertCount][0] = j+i-4.0f; vertices[vertCount][1] = 194.0f; vertCount++;
            vertices[vertCount][0] = j+i-5.0f; vertices[vertCount][1] = 195.0f; vertCount++;
            vertices[vertCount][0] = j+i-6.0f; vertices[vertCount][1] = 196.0f; vertCount++;
            vertices[vertCount][0] = j+i-7.0f; vertices[vertCount][1] = 197.0f; vertCount++;
        }
        for (i = 0.0f; i < 40.0f; i+=8)
        {
            vertices[vertCount][0] = j;      vertices[vertCount][1] = 198.0f+i; vertCount++;
            vertices[vertCount][0] = j-1.0f; vertices[vertCount][1] = 199.0f+i; vertCount++;
            vertices[vertCount][0] = j-2.0f; vertices[vertCount][1] = 200.0f+i; vertCount++;
            vertices[vertCount][0] = j-3.0f; vertices[vertCount][1] = 201.0f+i; vertCount++;
            vertices[vertCount][0] = j-4.0f; vertices[vertCount][1] = 202.0f+i; vertCount++;
            vertices[vertCount][0] = j-3.0f; vertices[vertCount][1] = 203.0f+i; vertCount++;
            vertices[vertCount][0] = j-2.0f; vertices[vertCount][1] = 204.0f+i; vertCount++;
            vertices[vertCount][0] = j-1.0f; vertices[vertCount][1] = 205.0f+i; vertCount++;
        }
        if (vertCountPerArray == 0) vertCountPerArray = vertCount;
        polygonCount += vertCountPerArray;
        arrayList[arrayCount].count = vertCountPerArray;
        arrayCount++;
    }
    return GL_LINE_LOOP;
}

static GLuint DrawLineLoop50()
{
    float i;
    for (i = 20.0f; i <= 460.0f; i += 110.0f)
    {
        arrayList[arrayCount].first = vertCount;
        vertices[vertCount][0] = i;        vertices[vertCount][1] = 240.0f; vertCount++;
        vertices[vertCount][0] = i+ 50.0f; vertices[vertCount][1] = 240.0f; vertCount++;
        vertices[vertCount][0] = i+ 80.0f; vertices[vertCount][1] = 280.0f; vertCount++;
        vertices[vertCount][0] = i+110.0f; vertices[vertCount][1] = 320.0f; vertCount++;
        vertices[vertCount][0] = i+140.0f; vertices[vertCount][1] = 360.0f; vertCount++;
        vertices[vertCount][0] = i+140.0f; vertices[vertCount][1] = 410.0f; vertCount++;
        vertices[vertCount][0] = i+140.0f; vertices[vertCount][1] = 460.0f; vertCount++;
        vertices[vertCount][0] = i+110.0f; vertices[vertCount][1] = 420.0f; vertCount++;
        vertices[vertCount][0] = i+ 80.0f; vertices[vertCount][1] = 380.0f; vertCount++;
        vertices[vertCount][0] = i+ 50.0f; vertices[vertCount][1] = 340.0f; vertCount++;
        vertices[vertCount][0] = i+ 50.0f; vertices[vertCount][1] = 290.0f; vertCount++;
        vertices[vertCount][0] = i;        vertices[vertCount][1] = 290.0f; vertCount++;
        polygonCount += 12;
        arrayList[arrayCount].count = 12;
        arrayCount++;
    }
    return GL_LINE_LOOP;
}

static GLuint DrawTriangle1()
{
    float i, j;
    int vertCountPerArray = 0;
    for (j = 230.0f; j < 250.0f; j += 5.0f)
    {
        arrayList[arrayCount].first = vertCount;
        for (i = 220.0f; i < 420.0f; i += 5.0f)
        {
            vertices[vertCount][0] = i;      vertices[vertCount][1] = j;      vertCount++;
            vertices[vertCount][0] = i;      vertices[vertCount][1] = j-1.0f; vertCount++;
            vertices[vertCount][0] = i+1.0f; vertices[vertCount][1] = j-1.0f; vertCount++;
            polygonCount++;
        }
        if (vertCountPerArray == 0) vertCountPerArray = vertCount;
        arrayList[arrayCount].count = vertCountPerArray;
        arrayCount++;
    }
    return GL_TRIANGLES;
}

static GLuint DrawTriangle50()
{
    float i;
    arrayList[0].first = 0;
    arrayList[0].count = 0;
    for (i = 20.0f; i < 620.0f; i += 12.0f)
    {
        vertices[vertCount][0] = i;       vertices[vertCount][1] = 240.0f; vertCount++;
        vertices[vertCount][0] = i;       vertices[vertCount][1] = 230.0f; vertCount++;
        vertices[vertCount][0] = i+10.0f; vertices[vertCount][1] = 230.0f; vertCount++;
        polygonCount++;
    }
    arrayList[0].count = vertCount;
    arrayCount = 1;
    return GL_TRIANGLES;
}

static GLuint DrawTriangle10000()
{
    float i;
    arrayList[0].first = 0;
    arrayList[0].count = 0;
    for (i = 20.0f; i < 620.0f; i += 100.0f)
    {
        vertices[vertCount][0] = i;        vertices[vertCount][1] = 340.0f; vertCount++;
        vertices[vertCount][0] = i;        vertices[vertCount][1] = 140.0f; vertCount++;
        vertices[vertCount][0] = i+100.0f; vertices[vertCount][1] = 140.0f; vertCount++;
        polygonCount++;
    }
    arrayList[0].count = vertCount;
    arrayCount = 1;
    return GL_TRIANGLES;
}

static GLuint DrawTriangleStrip1()
{
    float i, j;
    int vertCountPerArray = 0;
    for (j = 225.0f; j < 265.0f; j += 10.0f)
    {
        arrayList[arrayCount].first = vertCount;
        vertices[vertCount][0] = 299.0f; vertices[vertCount][1] = j;      vertCount++;
        vertices[vertCount][0] = 299.0f; vertices[vertCount][1] = j-2.0f; vertCount++;
        for (i = 300.0f; i < 350.0f; i += 1.0f)
        {
            vertices[vertCount][0] = i; vertices[vertCount][1] = j;      vertCount++;
            vertices[vertCount][0] = i; vertices[vertCount][1] = j-2.0f; vertCount++;
            polygonCount += 2;
        }
        if (vertCountPerArray == 0) vertCountPerArray = vertCount;
        arrayList[arrayCount].count = vertCountPerArray;
        arrayCount++;
    }
    return GL_TRIANGLE_STRIP;
}

static GLuint DrawTriangleStrip50()
{
    float i;
    arrayList[0].first = 0;
    arrayList[0].count = 0;
    vertices[vertCount][0] = 20.0f; vertices[vertCount][1] = 240.0f; vertCount++;
    vertices[vertCount][0] = 20.0f; vertices[vertCount][1] = 230.0f; vertCount++;
    for (i = 30.0f; i <= 620.0f; i += 10.0f)
    {
        vertices[vertCount][0] = i; vertices[vertCount][1] = 240.0f; vertCount++;
        vertices[vertCount][0] = i; vertices[vertCount][1] = 230.0f; vertCount++;
        polygonCount += 2;
    }
    arrayList[0].count = vertCount;
    arrayCount = 1;
    return GL_TRIANGLE_STRIP;
}

static GLuint DrawTriangleStrip10000()
{
    float i;
    arrayList[0].first = 0;
    arrayList[0].count = 0;
    vertices[vertCount][0] = 20.0f; vertices[vertCount][1] = 340.0f; vertCount++;
    vertices[vertCount][0] = 20.0f; vertices[vertCount][1] = 140.0f; vertCount++;
    for (i = 120.0f; i <= 620.0f; i += 100.0f)
    {
        vertices[vertCount][0] = i; vertices[vertCount][1] = 340.0f; vertCount++;
        vertices[vertCount][0] = i; vertices[vertCount][1] = 140.0f; vertCount++;
        polygonCount += 2;
    }
    arrayList[0].count = vertCount;
    arrayCount = 1;
    return GL_TRIANGLE_STRIP;
}

static GLuint DrawTriangleFan1()
{
    float i;
    for (i = 20.0f; i <= 620.0f; i += 60.0f)
    {
        arrayList[arrayCount].first = vertCount;
        vertices[vertCount][0] = i;      vertices[vertCount][1] = 240.0f; vertCount++;
        vertices[vertCount][0] = i+5.0f; vertices[vertCount][1] = 240.0f; vertCount++;
        vertices[vertCount][0] = i+5.0f; vertices[vertCount][1] = 241.0f; vertCount++;
        vertices[vertCount][0] = i+5.0f; vertices[vertCount][1] = 242.0f; vertCount++;
        vertices[vertCount][0] = i+5.0f; vertices[vertCount][1] = 243.0f; vertCount++;
        vertices[vertCount][0] = i+5.0f; vertices[vertCount][1] = 244.0f; vertCount++;
        vertices[vertCount][0] = i+5.0f; vertices[vertCount][1] = 245.0f; vertCount++;
        vertices[vertCount][0] = i+4.0f; vertices[vertCount][1] = 245.0f; vertCount++;
        vertices[vertCount][0] = i+3.0f; vertices[vertCount][1] = 245.0f; vertCount++;
        vertices[vertCount][0] = i+2.0f; vertices[vertCount][1] = 245.0f; vertCount++;
        vertices[vertCount][0] = i+1.0f; vertices[vertCount][1] = 245.0f; vertCount++;
        vertices[vertCount][0] = i     ; vertices[vertCount][1] = 245.0f; vertCount++;
        vertices[vertCount][0] = i-1.0f; vertices[vertCount][1] = 245.0f; vertCount++;
        vertices[vertCount][0] = i-2.0f; vertices[vertCount][1] = 245.0f; vertCount++;
        vertices[vertCount][0] = i-3.0f; vertices[vertCount][1] = 245.0f; vertCount++;
        vertices[vertCount][0] = i-4.0f; vertices[vertCount][1] = 245.0f; vertCount++;
        vertices[vertCount][0] = i-5.0f; vertices[vertCount][1] = 245.0f; vertCount++;
        vertices[vertCount][0] = i-5.0f; vertices[vertCount][1] = 244.0f; vertCount++;
        vertices[vertCount][0] = i-5.0f; vertices[vertCount][1] = 243.0f; vertCount++;
        vertices[vertCount][0] = i-5.0f; vertices[vertCount][1] = 242.0f; vertCount++;
        vertices[vertCount][0] = i-5.0f; vertices[vertCount][1] = 241.0f; vertCount++;
        vertices[vertCount][0] = i-5.0f; vertices[vertCount][1] = 240.0f; vertCount++;
        vertices[vertCount][0] = i-5.0f; vertices[vertCount][1] = 239.0f; vertCount++;
        vertices[vertCount][0] = i-5.0f; vertices[vertCount][1] = 238.0f; vertCount++;
        vertices[vertCount][0] = i-5.0f; vertices[vertCount][1] = 237.0f; vertCount++;
        vertices[vertCount][0] = i-5.0f; vertices[vertCount][1] = 236.0f; vertCount++;
        vertices[vertCount][0] = i-5.0f; vertices[vertCount][1] = 235.0f; vertCount++;
        vertices[vertCount][0] = i-4.0f; vertices[vertCount][1] = 235.0f; vertCount++;
        vertices[vertCount][0] = i-3.0f; vertices[vertCount][1] = 235.0f; vertCount++;
        vertices[vertCount][0] = i-2.0f; vertices[vertCount][1] = 235.0f; vertCount++;
        vertices[vertCount][0] = i-1.0f; vertices[vertCount][1] = 235.0f; vertCount++;
        vertices[vertCount][0] = i     ; vertices[vertCount][1] = 235.0f; vertCount++;
        vertices[vertCount][0] = i+1.0f; vertices[vertCount][1] = 235.0f; vertCount++;
        vertices[vertCount][0] = i+2.0f; vertices[vertCount][1] = 235.0f; vertCount++;
        vertices[vertCount][0] = i+3.0f; vertices[vertCount][1] = 235.0f; vertCount++;
        vertices[vertCount][0] = i+4.0f; vertices[vertCount][1] = 235.0f; vertCount++;
        vertices[vertCount][0] = i+5.0f; vertices[vertCount][1] = 235.0f; vertCount++;
        vertices[vertCount][0] = i+5.0f; vertices[vertCount][1] = 236.0f; vertCount++;
        vertices[vertCount][0] = i+5.0f; vertices[vertCount][1] = 237.0f; vertCount++;
        vertices[vertCount][0] = i+5.0f; vertices[vertCount][1] = 238.0f; vertCount++;
        vertices[vertCount][0] = i+5.0f; vertices[vertCount][1] = 239.0f; vertCount++;
        vertices[vertCount][0] = i+5.0f; vertices[vertCount][1] = 240.0f; vertCount++;
        arrayList[arrayCount].count = 42;
        arrayCount++;
        polygonCount += 40;
    }
    return GL_TRIANGLE_FAN;
}

static GLuint DrawTriangleFan50()
{
    float i;
    for (i = 20.0f; i <= 620.0f; i += 40.0f)
    {
        arrayList[arrayCount].first = vertCount;
        vertices[vertCount][0] = i;       vertices[vertCount][1] = 240.0f; vertCount++;
        vertices[vertCount][0] = i+10.0f; vertices[vertCount][1] = 240.0f; vertCount++;
        vertices[vertCount][0] = i+10.0f; vertices[vertCount][1] = 250.0f; vertCount++;
        vertices[vertCount][0] = i;       vertices[vertCount][1] = 250.0f; vertCount++;
        vertices[vertCount][0] = i-10.0f; vertices[vertCount][1] = 250.0f; vertCount++;
        vertices[vertCount][0] = i-10.0f; vertices[vertCount][1] = 240.0f; vertCount++;
        vertices[vertCount][0] = i-10.0f; vertices[vertCount][1] = 230.0f; vertCount++;
        vertices[vertCount][0] = i;       vertices[vertCount][1] = 230.0f; vertCount++;
        vertices[vertCount][0] = i+10.0f; vertices[vertCount][1] = 230.0f; vertCount++;
        vertices[vertCount][0] = i+10.0f; vertices[vertCount][1] = 240.0f; vertCount++;
        arrayList[arrayCount].count = 10;
        arrayCount++;
        polygonCount += 8;
    }
    return GL_TRIANGLE_FAN;
}

static GLuint DrawTriangleFan10000()
{
    float i;
    for (i = 170.0f; i <= 470.0f; i += 300.0f)
    {
        arrayList[arrayCount].first = vertCount;
        vertices[vertCount][0] = i;        vertices[vertCount][1] = 240.0f; vertCount++;
        vertices[vertCount][0] = i+100.0f; vertices[vertCount][1] = 240.0f; vertCount++;
        vertices[vertCount][0] = i+100.0f; vertices[vertCount][1] = 440.0f; vertCount++;
        vertices[vertCount][0] = i;        vertices[vertCount][1] = 440.0f; vertCount++;
        vertices[vertCount][0] = i-100.0f; vertices[vertCount][1] = 440.0f; vertCount++;
        vertices[vertCount][0] = i-100.0f; vertices[vertCount][1] = 240.0f; vertCount++;
        vertices[vertCount][0] = i-100.0f; vertices[vertCount][1] =  40.0f; vertCount++;
        vertices[vertCount][0] = i;        vertices[vertCount][1] =  40.0f; vertCount++;
        vertices[vertCount][0] = i+100.0f; vertices[vertCount][1] =  40.0f; vertCount++;
        vertices[vertCount][0] = i+100.0f; vertices[vertCount][1] = 240.0f; vertCount++;
        arrayList[arrayCount].count = 10;
        arrayCount++;
        polygonCount += 8;
    }
    return GL_TRIANGLE_FAN;
}

// Determine vertex speed with small triangle strips
static unsigned int CalibrateVertexSpeed()
{
    unsigned int t = 1;
    secs = 0.0;

    // Keep increasing number of tristrip tests until it takes 1.0 seconds
    while (secs < calibrate)
    {
        t <<= 1;
        RunTestV(DrawTriangleStrip1, t);
        if (t == 0) return 0xffffffff;    // super-fast hardware?
    }
    return t;
}

// Determine fillrate speed with large tristrips
static unsigned int CalibrateFillrateSpeed()
{
    unsigned int t = 1;
    secs = 0.0;

    // Keep increasing number of tristrip tests until it takes 1.0 seconds
    while (secs < calibrate)
    {
        t <<= 1;
        RunTestV(DrawTriangleStrip10000, t);
        if (t == 0) return 0xffffffff;    // super-fast hardware?
    }
    return t;
}

static void FormatResults(char *data, char *desc, char *polyType, char *units, int polyCount, double secs)
{
#if (PRINT_HALF_TEXT == 1)
    sprintf(data,         "%s %9d %sin %2.2fs = %9.0f %s", desc, polyCount, polyType, secs, (double)polyCount / secs, units);
#else
    sprintf(data, "   %9.0f", (double)polyCount / secs);
#endif
}

static void DrawBench(void)
{
    unsigned int i, numVTests = 1, numFTests = 1, numVFTests = 1;

    //glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    eglSwapBuffers(f_dpy, f_surface);

    // Set up arrays
    memset(vertices, 0x00, sizeof(vertices));
    for (i = 0; i < sizeof(indices) / sizeof(indices[0]); i++)
    {
        indices[i] = (GLushort)i;
    }
    glBindAttribLocation(shaderProgram, 0, "vertex");
    glBindAttribLocation(shaderPointsProgram, 0, "vertex");
    glEnableVertexAttribArray(0);
    if (half)
        glVertexAttribPointer(0, 2, GL_HALF_FLOAT_OES, GL_FALSE, 0, hVertices);
    else
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, vertices);

    // Auto-detect number of tests to run
    printf("Calibrating.");
    numVTests = CalibrateVertexSpeed();
    printf(".");
    numFTests = CalibrateFillrateSpeed();
    numVFTests = numVTests/2 + numFTests/2;
    printf(".Done!\n");

    RunTestV(DrawPoint, numVTests);
    FormatResults(pointData,       "   1 Pixel Points:          ", "points    ", "pps", polygonCount, secs);
    printf("%s", pointData);            printf("\n");    printf("\n");

    RunTestV(DrawLine1, numVTests);
    FormatResults(line1Data,       "   1 Pixel Lines:           ", "lines     ", "lps", polygonCount, secs);
    printf("%s",line1Data);            printf("\n");

    RunTestV(DrawLineStrip1, numVTests);
    FormatResults(lineStrip1Data,  "   1 Pixel Line Strips:     ", "lines     ", "lps", polygonCount, secs);
    printf("%s",lineStrip1Data);        printf("\n");

    RunTestV(DrawLineLoop1, numVTests);
    FormatResults(lineLoop1Data,   "   1 Pixel Line Loops:      ", "lines     ", "lps", polygonCount, secs);
    printf("%s",lineLoop1Data);        printf("\n");

    RunTestV(DrawLine50, numFTests*4);
    FormatResults(line50Data,      "  50 Pixel Lines:           ", "lines     ", "lps", polygonCount, secs);
    printf("%s",line50Data);            printf("\n");

    RunTestV(DrawLineStrip50, numFTests*4);
    FormatResults(lineStrip50Data, "  50 Pixel Line Strips:     ", "lines     ", "lps", polygonCount, secs);
    printf("%s",lineStrip50Data);    printf("\n");

    RunTestV(DrawLineLoop50, numFTests*4);
    FormatResults(lineLoop50Data,  "  50 Pixel Line Loops:      ", "lines     ", "lps", polygonCount, secs);
    printf("%s",lineLoop50Data);        printf("\n");    printf("\n");

    RunTestV(DrawTriangle1, numVTests/2);
    FormatResults(tri1Data,          "    1 Pixel Triangles:      ", "triangles ", "tps", polygonCount, secs);
    printf("%s",tri1Data);            printf("\n");

    RunTestV(DrawTriangleStrip1, numVTests);
    FormatResults(triStrip1Data,     "    1 Pixel Triangle Strips:", "triangles ", "tps", polygonCount, secs);
    printf("%s",triStrip1Data);        printf("\n");

    RunTestV(DrawTriangleFan1, numVTests);
    FormatResults(triFan1Data,       "    1 Pixel Triangle Fans:  ", "triangles ", "tps", polygonCount, secs);
    printf("%s",triFan1Data);        printf("\n");

    RunTestV(DrawTriangle50, numVFTests);
    FormatResults(tri50Data,         "   50 Pixel Triangles:      ", "triangles ", "tps", polygonCount, secs);
    printf("%s",tri50Data);            printf("\n");

    RunTestV(DrawTriangleStrip50, numVFTests);
    FormatResults(triStrip50Data,    "   50 Pixel Triangle Strips:", "triangles ", "tps", polygonCount, secs);
    printf("%s",triStrip50Data);        printf("\n");

    RunTestV(DrawTriangleFan50, numVFTests/2);
    FormatResults(triFan50Data,      "   50 Pixel Triangle Fans:  ", "triangles ", "tps", polygonCount, secs);
    printf("%s",triFan50Data);        printf("\n");

    RunTestV(DrawTriangle10000, numFTests*2);
    FormatResults(tri10000Data,      "10000 Pixel Triangles:      ", "triangles ", "tps", polygonCount, secs);
    printf("%s",tri10000Data);        printf("\n");

    RunTestV(DrawTriangleStrip10000, numFTests);
    FormatResults(triStrip10000Data, "10000 Pixel Triangle Strips:", "triangles ", "tps", polygonCount, secs);
    printf("%s",triStrip10000Data);    printf("\n");

    RunTestV(DrawTriangleFan10000, numFTests);
    FormatResults(triFan10000Data,   "10000 Pixel Triangle Fans:  ", "triangles ", "tps", polygonCount, secs);
    printf("%s",triFan10000Data);    printf("\n");    printf("\n");

    // Vertex arrays don't affect non-vertex tests
    if ((mode == ARRAY) && (half == GL_FALSE))
    {
        RunTest(DrawClear, numFTests/4);
        FormatResults(clearData,     "      Clears:               ", "clears    ", "cps", polygonCount, secs);
        printf("%s",clearData);            printf("\n");
    }
}

//add by syy
const GLchar* vsbench =
	"attribute vec4 vertex;"
	"void main (void)"
	"{"
	"	gl_Position = vertex;"
	"}";
const GLchar* fsbench =
	"void main()"
	"{"
	"    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);"
	"}";

const GLchar* vsbench_half =
	"attribute vec4 vertex;"
	"void main (void)"
	"{"
	"	gl_Position = vertex;"
	"}";
const GLchar* fsbench_half =
	"void main()"
	"{"
	"    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);"
	"}";

const GLchar* vsbench_point_half =
	"attribute vec4 vertex;"
	"void main (void)"
	"{"
	"	gl_PointSize = 1.0;"
	"	gl_Position = vertex;"
	"}";
const GLchar* fsbench_point_half =
	"void main()"
	"{"
	"    gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);"
	"}";

const GLchar* vsbench_point =
	"attribute vec4 vertex;"
	"void main (void)"
	"{"
	"	gl_PointSize = 1.0;"
	"	gl_Position = vertex;"
	"}";
const GLchar* fsbench_point =
	"void main()"
	"{"
	"    gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);"
	"}";

int drawmain(void) {
    int i;
    GLint   linkStatus  = GL_FALSE;
    GLuint shaders[2], pointsShaders[2], shadersHalf[2], pointsShadersHalf[2];
    GLint pixelFormat[4];

    InitGL();

    /* Create our shader objects */
    shaders[0] = glCreateShader(GL_VERTEX_SHADER);
    shaders[1] = glCreateShader(GL_FRAGMENT_SHADER);
    pointsShaders[0] = glCreateShader(GL_VERTEX_SHADER);
    pointsShaders[1] = glCreateShader(GL_FRAGMENT_SHADER);
    shadersHalf[0] = glCreateShader(GL_VERTEX_SHADER);
    shadersHalf[1] = glCreateShader(GL_FRAGMENT_SHADER);
    pointsShadersHalf[0] = glCreateShader(GL_VERTEX_SHADER);
    pointsShadersHalf[1] = glCreateShader(GL_FRAGMENT_SHADER);

    /* Create our program object */
    shaderProgram = glCreateProgram();
    shaderPointsProgram = glCreateProgram();
    shaderProgramHalf = glCreateProgram();
    shaderPointsProgramHalf = glCreateProgram();

    /* Attach our shaders to the program object */
    glAttachShader(shaderProgram, shaders[0]);
    glAttachShader(shaderProgram, shaders[1]);
    glAttachShader(shaderPointsProgram, pointsShaders[0]);
    glAttachShader(shaderPointsProgram, pointsShaders[1]);
    glAttachShader(shaderProgramHalf, shadersHalf[0]);
    glAttachShader(shaderProgramHalf, shadersHalf[1]);
    glAttachShader(shaderPointsProgramHalf, pointsShadersHalf[0]);
    glAttachShader(shaderPointsProgramHalf, pointsShadersHalf[1]);
    //********************************************************************
    GLint status;
    glShaderSource(shaders[0],1,&vsbench,NULL);
    glCompileShader(shaders[0]);
    glGetShaderiv(shaders[0], GL_COMPILE_STATUS, &status);
	if (status == 0)
	{
		printf("Failed to create a vsbench shader.\n");
		glDeleteShader(shaders[0]);
		return GL_FALSE;
	}

    glShaderSource(shaders[1],1,&fsbench,NULL);
    glCompileShader(shaders[1]);
    glGetShaderiv(shaders[0], GL_COMPILE_STATUS, &status);
	if (status == 0)
	{
		printf("Failed to create a fsbench shader.\n");
		glDeleteShader(shaders[1]);
		return GL_FALSE;
	}

    glShaderSource(pointsShaders[0],1,&vsbench_point,NULL);
    glCompileShader(pointsShaders[0]);
    glGetShaderiv(pointsShaders[0], GL_COMPILE_STATUS, &status);
	if (status == 0)
	{
		printf("Failed to create a vsbench_point shader.\n");
		glDeleteShader(pointsShaders[0]);
		return GL_FALSE;
	}

    glShaderSource(pointsShaders[1],1,&fsbench_point,NULL);
    glCompileShader(pointsShaders[1]);
    glGetShaderiv(pointsShaders[1], GL_COMPILE_STATUS, &status);
	if (status == 0)
	{
		printf("Failed to create a fsbench_point shader.\n");
		glDeleteShader(pointsShaders[1]);
		return GL_FALSE;
	}

    glShaderSource(shadersHalf[0],1,&vsbench_half,NULL);
    glCompileShader(shadersHalf[0]);
    glGetShaderiv(shadersHalf[0], GL_COMPILE_STATUS, &status);
	if (status == 0)
	{
		printf("Failed to create a vsbench_half shader.\n");
		glDeleteShader(shadersHalf[0]);
		return GL_FALSE;
	}

    glShaderSource(shadersHalf[1],1,&fsbench_half,NULL);
    glCompileShader(shadersHalf[1]);
    glGetShaderiv(shadersHalf[1], GL_COMPILE_STATUS, &status);
	if (status == 0)
	{
		printf("Failed to create a fsbench_half shader.\n");
		glDeleteShader(shadersHalf[1]);
		return GL_FALSE;
	}

    glShaderSource(pointsShadersHalf[0],1,&vsbench_point_half,NULL);
    glCompileShader(pointsShadersHalf[0]);
    glGetShaderiv(pointsShadersHalf[0], GL_COMPILE_STATUS, &status);
	if (status == 0)
	{
		printf("Failed to create a vsbench_point_half shader.\n");
		glDeleteShader(pointsShadersHalf[0]);
		return GL_FALSE;
	}

    glShaderSource(pointsShadersHalf[1],1,&fsbench_point_half,NULL);
    glCompileShader(pointsShadersHalf[1]);
    glGetShaderiv(pointsShadersHalf[1], GL_COMPILE_STATUS, &status);
	if (status == 0)
	{
		printf("Failed to create a fsbench_point_half shader.\n");
		glDeleteShader(pointsShadersHalf[1]);
		return GL_FALSE;
	}
    //********************************************************************

    /* Load the shader binaries into the driver */
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linkStatus);
    if (linkStatus != GL_TRUE) {
        GLsizei length = 0;
        char logBuf[ 1024 ];
        glGetProgramInfoLog(shaderProgram, 1024, &length, logBuf);
        printf("glLinkProgram()1 failed! Info log:\n--Start Info Log---\n %s \n--End Info Log---\n", logBuf);
        return -1;
    }

    glLinkProgram(shaderPointsProgram);
    glGetProgramiv(shaderPointsProgram, GL_LINK_STATUS, &linkStatus);
    if (linkStatus != GL_TRUE) {
        GLsizei length = 0;
        char logBuf[ 1024 ];
        glGetProgramInfoLog(shaderPointsProgram, 1024, &length, logBuf);
        printf("glLinkProgram()2 failed! Info log:\n--Start Info Log---\n %s \n--End Info Log---\n", logBuf);
        return -1;
    }

    glLinkProgram(shaderProgramHalf);
    glGetProgramiv(shaderProgramHalf, GL_LINK_STATUS, &linkStatus);
    if (linkStatus != GL_TRUE) {
        GLsizei length = 0;
        char logBuf[ 1024 ];
        glGetProgramInfoLog(shaderProgramHalf, 1024, &length, logBuf);
        printf("glLinkProgram()3 failed! Info log:\n--Start Info Log---\n %s \n--End Info Log---\n", logBuf);
        return -1;
    }

    glLinkProgram(shaderPointsProgramHalf);
    glGetProgramiv(shaderPointsProgramHalf, GL_LINK_STATUS, &linkStatus);
    if (linkStatus != GL_TRUE) {
        GLsizei length = 0;
        char logBuf[ 1024 ];
        glGetProgramInfoLog(shaderPointsProgramHalf, 1024, &length, logBuf);
        printf("glLinkProgram()4 failed! Info log:\n--Start Info Log---\n %s \n--End Info Log---\n", logBuf);
        return -1;
    }

    // Get Driver Details:
    printf("OpenGL Vendor : %s\n", glGetString(GL_VENDOR));
    printf("OpenGL Version: %s\n", glGetString(GL_VERSION));
    glGetIntegerv(GL_RED_BITS,   &pixelFormat[0]);
    glGetIntegerv(GL_GREEN_BITS, &pixelFormat[1]);
    glGetIntegerv(GL_BLUE_BITS,  &pixelFormat[2]);
    glGetIntegerv(GL_ALPHA_BITS, &pixelFormat[3]);
    if (pixelFormat[3] > 0)
    {
        sprintf(dispFormat, "Display Format: RGBA %d-%d-%d-%d", pixelFormat[0], pixelFormat[1], pixelFormat[2], pixelFormat[3]);
    }
    else
    {
        sprintf(dispFormat, "Display Format: RGB %d-%d-%d", pixelFormat[0], pixelFormat[1], pixelFormat[2]);
    }
    printf("%s", dispFormat);
    glGetIntegerv(GL_DEPTH_BITS, &pixelFormat[0]);
    if (pixelFormat[0] > 0)
    {
        sprintf(dispFormat, ", Depth %d", pixelFormat[0]);
	    printf("%s", dispFormat);
    }
    glGetIntegerv(GL_STENCIL_BITS, &pixelFormat[0]);
    if (pixelFormat[0] > 0)
    {
        sprintf(dispFormat, ", Stencil %d", pixelFormat[0]);
	    printf("%s\n", dispFormat);
    }
	else
	{
		printf("\n");
	}

    printf("\nMAX_RENDER_MODE*2 : %d\n",MAX_RENDER_MODE*2);
    //logMsg ("\nMAX_RENDER_MODE*2 : %d\n",MAX_RENDER_MODE*2,0,0, 0, 0, 0);

    for (i = 0; i < MAX_RENDER_MODE*2; i++)
    {
    	printf("\nloop : %d\n",i);
    	//logMsg ("\nloop : %d\n",i,0,0, 0, 0, 0);
        switch (i)
        {
        case 0:  half = GL_FALSE; mode = ARRAY;        break;
        case 1:  half = GL_FALSE; mode = ELEMENTS;     break;
        case 2:  half = GL_FALSE; mode = VBO_ARRAY;    break;
        case 3:  half = GL_FALSE; mode = VBO_ELEMENTS; break;
        case 4:  half = GL_FALSE; mode = ARRAY_VBO_ELEMENTS; break;
        case 5:  half = GL_FALSE; mode = VBO_ARRAY_ELEMENTS; break;

        case 6:  half = GL_TRUE;  mode = ARRAY;        break;
        case 7:  half = GL_TRUE;  mode = ELEMENTS;     break;
        case 8:  half = GL_TRUE;  mode = VBO_ARRAY;    break;
        case 9:  half = GL_TRUE;  mode = VBO_ELEMENTS; break;
        case 10: half = GL_TRUE;  mode = ARRAY_VBO_ELEMENTS; break;
        case 11: half = GL_TRUE;  mode = VBO_ARRAY_ELEMENTS; break;

        default: continue;
        }

        // * Support vertex buffers
        printf("\n");
        printf("Vertex Data:    ");
        if (half)
            printf("FP16\n");
        else
            printf("FP32\n");

        printf("Rendering Mode: ");
        switch (mode)
        {
        case ARRAY:        printf("DrawArrays\n\n"); break;
        case ELEMENTS:     printf("DrawElements\n\n"); break;
        case VBO_ARRAY:    printf("DrawArrays (Vertices in VBO)\n\n"); break;
        case VBO_ELEMENTS: printf("DrawElements (Vertices and Indices in VBOs)\n\n"); break;
        case ARRAY_VBO_ELEMENTS: printf("DrawElements (Indices in VBO)\n\n"); break;
        case VBO_ARRAY_ELEMENTS: printf("DrawElements (Vertices in VBO)\n\n"); break;
        }

        DrawBench();
    }

    /* Delete our shaders */
    glDeleteShader(shaders[0]);
    glDeleteShader(shaders[1]);
    glDeleteShader(pointsShaders[0]);
    glDeleteShader(pointsShaders[1]);
    glDeleteShader(shadersHalf[0]);
    glDeleteShader(shadersHalf[1]);
    glDeleteShader(pointsShadersHalf[0]);
    glDeleteShader(pointsShadersHalf[1]);

    /* Delete our programs */
    glDeleteProgram(shaderProgram);
    glDeleteProgram(shaderPointsProgram);
    glDeleteProgram(shaderProgramHalf);
    glDeleteProgram(shaderPointsProgramHalf);

    /* Detach the current program from this context */
    glUseProgram(0);

    /* Release the contexts and surfaces so they can be deleted */
    return 0;
}

static void InitGL(void) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
}


static uint16 S_UtilFP32ToFP16(float32 in)
{
    uint16 retVal;
    float32 fval = (float32)in;
    uint32 ival = *(uint32 *)(void *)(&(fval));
    int32 e = ((ival & 0x7f800000) >> 23) - 127 + 15;
    uint32 s = ival & 0x80000000U;
    uint32 f = ival & 0x007fffffU;

    if (ival == 0)
    {
        retVal = 0;
    }
    else
    {
        if (e < 0)
        {
            retVal = 0;
        }
        else if (e >= 31)
        {
            retVal = (uint16)(((s >> 16) & 0x8000U) | 0x7c00U | ((f >> 13) & 0x03ffU) | (uint32)(!(!(f & 0x1FFF))));
        }
        else
        {
            retVal = (uint16)(((s >> 16) & 0x8000U) | ((uint32)((e << 10) & 0x7c00)) | ((f >> 13) & 0x03ffU));
        }
    }
    return (retVal);
}

int test_glttf(const char * device, unsigned int screenid) {
	EGLint aEGLAttributes[] = { EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8,
			EGL_BLUE_SIZE, 8, EGL_DEPTH_SIZE, 16, EGL_RENDERABLE_TYPE,
			EGL_OPENGL_ES2_BIT, EGL_NONE };

	EGLint aEGLContextAttributes[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };

	EGLint nConfigs;
	printf("enter test_glttf\n");

	InitGL();

	f_dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);

	eglInitialize(f_dpy, NULL, NULL);

	if (!eglBindAPI(EGL_OPENGL_ES_API)) {
		printf("error eglBindAPI, exit!\n");
		return 0;
	}

	eglChooseConfig(f_dpy, aEGLAttributes, &f_config, 1, &nConfigs);

	printf("EGLConfig = %p\n", f_config);

	f_surface = eglCreateWindowSurface(f_dpy, f_config,
			(NativeWindowType) 0, 0);
	f_eglContext = eglCreateContext(f_dpy, f_config, EGL_NO_CONTEXT,
			aEGLContextAttributes);

	printf("EGLContext = %p\n", f_eglContext);

	eglMakeCurrent(f_dpy, f_surface, f_surface, f_eglContext);
	eglSwapInterval(f_dpy, 1);

	//**********************************
	drawmain();
	//**********************************

	/* Exit and clean up */
	eglMakeCurrent(f_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglDestroyContext(f_dpy, f_eglContext);
	eglDestroySurface(f_dpy, f_surface);
	eglTerminate(f_dpy);
	return 0;
}

void main ()
{
	test_glttf(0,0);
}
