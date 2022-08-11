/*************************************************************************/ /*!
@File
@Title          GL ES 3.0 extensions.
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef __drv_gl3ext_h__
#define __drv_gl3ext_h__

/* We want all the prototypes */
#define GL_GLEXT_PROTOTYPES

/* Normally GL extensions with GL_GLEXT_PROTOTYPES will get default
 * visibility with GL_API, but we want to override that. The old
 * SGX headers used GL_API_EXT to achieve this.
 */
#if defined(__linux__) || defined(__QNXNTO__)
#undef GL_API_EXT
#define GL_API_EXT __attribute__((visibility("hidden")))
#elif defined (_WIN32)
#undef GL_API_EXT
#define GL_API_EXT __declspec(dllexport)
#elif defined(INTEGRITY_OS)
#define GL_API_EXT
#endif

/* NOTE: If you want to override an extension, you could put it here,
 * before we include the Khronos header, so the khronos header doesn't
 * define it first.
 */

#if defined _MSC_VER
#pragma push_macro("GL_APICALL")
#endif
#undef GL_APICALL
#define GL_APICALL GL_API_EXT

#define GL_APIENTRYP GL_APIENTRY*

/* Include the Khronos header first */
#include <GLES2/gl2ext.h>
//#include <GLES3/gl3ext.h>

/* NOTE: Extensions following might be overridden by the platform's
 * version of gl3ext.h.
 */

/*------------------------------------------------------------------------*
 * KHR extension tokens
 *------------------------------------------------------------------------*/

/*------------------------------------------------------------------------*
 * ARB extension tokens
 *------------------------------------------------------------------------*/

/*------------------------------------------------------------------------*
 * OES extension tokens
 *------------------------------------------------------------------------*/

 /*------------------------------------------------------------------------*
 * EXT extension tokens
 *------------------------------------------------------------------------*/
 
/*------------------------------------------------------------------------*
 * IMG extension tokens
 *------------------------------------------------------------------------*/

/* GL_IMG_shader_binary */
//#ifndef GL_IMG_shader_binary
#define GL_RGX_BINARY_IMG										0x8C0A
//#endif

/* GL_IMG_strip_rendering */
#ifndef GL_IMG_strip_rendering
#define GL_IMG_strip_rendering
#define GL_FRAMEBUFFER_STRIP_INDEX_IMG							0x8EA3
#define GL_FRAMEBUFFER_STRIP_BUFFER_INDEX_IMG					0x8EA4
#define GL_FRAMEBUFFER_STRIP_MODE_IMG							0x8EA5
#endif

/* GL_IMG_program_binary */
//#ifndef GL_IMG_program_binary
#define GL_RGX_PROGRAM_BINARY_IMG								0x9130
//#endif


/* GL_IMG_texture_filter_cubic */
#ifndef GL_IMG_texture_filter_cubic
#define GL_IMG_texture_filter_cubic 1
#define GL_CUBIC_IMG											0x9139
#define GL_CUBIC_MIPMAP_NEAREST_IMG								0x913A
#define GL_CUBIC_MIPMAP_LINEAR_IMG								0x913B
#endif

/* GL_IMG_framebuffer_downsample */
#ifndef GL_IMG_framebuffer_downsample
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_AND_DOWNSAMPLE_IMG    0x913C  
#define GL_NUM_DOWNSAMPLE_SCALES_IMG                                0x913D
#define GL_DOWNSAMPLE_SCALES_IMG                                    0x913E
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_SCALE_IMG                 0x913F
#endif

/* GL_EXT_polygon_offset_clamp */
#ifndef GL_EXT_polygon_offset_clamp
#define GL_EXT_polygon_offset_clamp 1
#define GL_POLYGON_OFFSET_CLAMP_EXT								0x8E1B
typedef void (GL_APIENTRYP PFNGLPOLYGONOFFSETCLAMPEXTPROC) (GLfloat factor, GLfloat units, GLfloat clamp);
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glPolygonOffsetClampEXT (GLfloat factor, GLfloat units, GLfloat clamp);
#endif
#endif	/* GL_EXT_polygon_offset_clamp */


//GL_IMG_ray_tracing
#define GL_RAY_SHADER_IMG                               0x9653
#define GL_FRAME_SHADER_IMG                             0x9654
#define GL_FRAME_SHADER_BIT_IMG                         0x00000040
#define GL_REFERENCED_BY_RAY_SHADER_IMG                 0x9655
#define GL_REFERENCED_BY_FRAME_SHADER_IMG               0x9656
#define GL_MAX_RAY_UNIFORM_BLOCKS_IMG                   0x966D
#define GL_MAX_RAY_TEXTURE_IMAGE_UNITS_IMG              0x9670
#define GL_MAX_RAY_IMAGE_UNIFORMS_IMG                   0x9673
#define GL_MAX_RAY_UNIFORM_COMPONENTS_IMG               0x966C
#define GL_MAX_RAY_ATOMIC_COUNTER_BUFFERS_IMG           0x9671
#define GL_MAX_RAY_ATOMIC_COUNTERS_IMG                  0x9672
#define GL_MAX_COMBINED_RAY_UNIFORM_COMPONENTS_IMG      0x966E
#define GL_MAX_RAY_INPUT_COMPONENTS_IMG                 0x966F
#define GL_MAX_FRAME_UNIFORM_BLOCKS_IMG                 0x9676
#define GL_MAX_FRAME_TEXTURE_IMAGE_UNITS_IMG            0x9678
#define GL_MAX_FRAME_IMAGE_UNIFORMS_IMG                 0x967B
#define GL_MAX_FRAME_UNIFORM_COMPONENTS_IMG             0x9675
#define GL_MAX_FRAME_ATOMIC_COUNTER_BUFFERS_IMG         0x9679
#define GL_MAX_FRAME_ATOMIC_COUNTERS_IMG                0x967A
#define GL_MAX_COMBINED_FRAME_UNIFORM_COMPONENTS_IMG    0x9677
#define GL_MAX_RAY_BLOCKS_IMG                           0x967F
#define GL_MAX_RAY_EMITS_IMG                            0x967E
#define GL_MAX_RAY_BLOCK_SIZE_IMG                       0x9680
#define GL_SCENE_ARRAY_IMG                              0x9688
#define GL_COMPONENT_GROUP_IMG                          0x9687
#define GL_COMPONENT_IMG                                0x9686
#define GL_RAY_BOUNCE_LIMIT_IMG                         0x966B
#define GL_MAX_RAY_BOUNCE_LIMIT_IMG                     0x9681
#define GL_SCENE_ARRAY_BINDING_IMG                      0x966A
#define GL_NO_WAIT_BIT_IMG                              0x00000001
#define GL_MAX_COMPONENT_GROUP_BUILD_SIZE_IMG           0x9683
#define GL_MAX_COMPONENT_GROUP_MERGE_SOURCES_IMG        0x9684
//Gap
#define GL_MAX_SCENE_ARRAY_COMPONENT_GROUP_BINDINGS_IMG 0x9685
#define GL_COMPONENT_RAY_VISIBILITY_IMG                 0x965F
#define GL_COMPONENT_RAY_OCCLUDER_IMG                   0x9660
#define GL_COMPONENT_VISIBLE_FACE_IMG                   0x9661
#define GL_SYNC_RTU_COMMANDS_COMPLETE_IMG               0x9658
#define GL_SYNC_SHG_COMMANDS_COMPLETE_IMG               0x9659
#define GL_MAX_RAY_PROGRAM_HANDLES_IMG                  0x9682
//Gap
#define GL_MAX_DISPATCH_RAYS_ID_IMG                     0x967D
//Gap
#define GL_ACCUMULATE_ONLY_IMG                          0x965A
#define GL_RAY_PROGRAM_HANDLE_IMG                       0x965B
//Gap
#define GL_SCENE_ARRAY_COMPONENT_GROUP_BINDING_IMG      0x9668
#define GL_SCENE_ARRAY_RAY_BLOCK_SIZE_IMG               0x9669
//Gap
#define GL_RAY_BLOCK_IMG                                0x965C
#define GL_RAY_VARIABLE_IMG                             0x965D
//Gap
#define GL_RAY_BLOCK_SIZE_IMG                           0x965E
#define GL_COMPONENT_GEOMETRY_INDEXED_IMG               0x9662
#define GL_COMPONENT_GEOMETRY_MODE_IMG                  0x9663
#define GL_COMPONENT_GEOMETRY_COUNT_IMG                 0x9664
#define GL_COMPONENT_GEOMETRY_BASE_VERTEX_IMG           0x9665
#define GL_COMPONENT_GEOMETRY_INDEX_TYPE_IMG            0x9666
#define GL_COMPONENT_GEOMETRY_INDEX_OFFSET_IMG          0x9667
#define GL_MAX_RAY_SHADER_STORAGE_BLOCKS_IMG            0x9674
#define GL_MAX_FRAME_SHADER_STORAGE_BLOCKS_IMG          0x967C

typedef void (* PFNGLRAYBOUNCELIMITIMGPROC ) (GLuint limit);
typedef GLuint (* PFNGLCREATESCENEARRAYIMGPROC ) (void);
typedef void (* PFNGLDELETESCENEARRAYIMGPROC ) (GLuint sceneArray);
typedef GLboolean (* PFNGLISSCENEARRAYIMGPROC ) (GLuint sceneArray);
typedef void (* PFNGLBINDSCENEARRAYIMGPROC ) (GLuint sceneArray);
typedef void (* PFNGLSCENEARRAYRAYBLOCKSIZEIMGPROC) (GLuint sceneArray, GLuint rayTypeIndex, GLsizei size);
typedef void (* PFNGLGETSCENEARRAYINFOIIMGPROC) (GLuint sceneArray, GLenum pname, GLuint index, GLint* params);

typedef void (* PFNGLDISPATCHRAYSIMGPROC ) (GLuint id, GLuint x, GLuint y, GLuint width, GLuint height, GLbitfield flags);

typedef void (* PFNGLCREATECOMPONENTSIMGPROC ) (GLsizei n, GLuint *components);
typedef void (* PFNGLDELETECOMPONENTSIMGPROC ) (GLsizei n, const GLuint* components);
typedef void (* PFNGLCOMPONENTOCCLUDERIMGPROC ) (GLuint component, GLboolean occluder);
typedef void (* PFNGLCOMPONENTRAYVISIBILITYIMGPROC ) (GLuint component, GLuint rayTypeIndex, GLboolean visible);
typedef void (* PFNGLCOMPONENTFRONTFACEIMGPROC ) (GLuint component, GLenum mode);
typedef void (* PFNGLCOMPONENTVISIBLEFACEIMGPROC ) (GLuint component, GLenum mode);
typedef void (* PFNGLCOMPONENTVERTEXARRAYIMGPROC ) (GLuint component, GLuint vertexArray);
typedef void (* PFNGLCOMPONENTGEOMETRYIMGPROC ) (GLuint component, GLenum mode, GLint first, GLsizei count);
typedef void (* PFNGLCOMPONENTINDEXEDGEOMETRYIMGPROC ) (GLuint component,
                                                       GLenum mode,
                                                       GLsizei count,
                                                       GLenum type,
                                                       const GLintptr indices,
                                                       GLint baseVertex);
typedef void (* PFNGLCOMPONENTBUFFERRANGEIMGPROC ) (GLuint component, GLenum target, GLuint index,
                                                    GLuint buffer, GLintptr offset, GLsizeiptr size);
typedef GLboolean (* PFNGLISCOMPONENTIMGPROC ) (GLuint component);
typedef void (* PFNGLGETCOMPONENTINFOIVIMGPROC ) (GLuint component, GLenum pname, GLuint* params);
typedef void (* PFNGLGETCOMPONENTINFOI_VIMGPROC ) (GLuint component, GLenum pname, GLuint index, GLuint* params);
typedef void (* PFNGLPROGRAMMAXRAYEMITSIMGPROC ) (GLuint program, GLuint inRayTypeIndex, GLuint value);
typedef GLuint64 (* PFNGLGETRAYPROGRAMHANDLEIMGPROC ) (GLuint rayProgram);
typedef void (* PFNGLRAYPROGRAMBUFFERRANGEIMGPROC ) (GLuint program, GLenum target, GLuint index,
                                          GLuint buffer, GLintptr offset, GLsizeiptr size);
typedef void (* PFNGLCREATECOMPONENTGROUPSIMGPROC ) (GLsizei n, GLuint *componentGroups);
typedef void (* PFNGLDELETECOMPONENTGROUPSIMGPROC ) (GLsizei n, const GLuint *componentGroups);
typedef void (* PFNGLISCOMPONENTGROUPIMGPROC ) (GLuint group);
typedef void (* PFNGLCOMPONENTGROUPEXTENTIMGPROC ) (GLuint componentGroup, const GLfloat* min, const GLfloat* max);
typedef GLuint64 (* PFNGLGETCOMPONENTGROUPHANDLEIMGPROC ) (GLuint componentGroup);
typedef void (* PFNGLBUILDCOMPONENTGROUPIMGPROC ) (GLuint id, GLuint componentGroup, GLsizei numComponents, const void* components);
typedef void (* PFNGLBUILDCOMPONENTGROUPBINARYIMGPROC ) (GLuint id, GLuint componentGroup, GLsizei numComponents, const void* components, const void* binary);
typedef void (* PFNGLMERGECOMPONENTGROUPSIMGPROC ) (GLuint componentGroupDestination, GLsizei numSourceComponentGroups, const void* componentGroups, GLenum quality);
typedef void (* PFNGLDUMPCOMPONENTGROUPIMGPROC ) (GLuint componentGroup, void** binary, GLsizei* size);
typedef void (* PFNGLBINDSCENEARRAYCOMPONENTGROUPIMGPROC ) (GLuint sceneArray, GLuint index, GLuint componentGroup);
typedef void (* PFNGLGETCOMPONENTGROUPEXTENTIMGPROC ) (GLuint componentGroup, GLfloat* min, GLfloat* max);
typedef void (* PFNGLGETCOMPONENTGROUPEXTENTPRECISIONIMGPROC ) (GLint* range, GLint* precision);
typedef GLuint64 (* PFNGLGETCOMPONENTPROGRAMHANDLEIMGPROC ) (GLuint component, GLuint program);
typedef GLuint64 (* PFNGLGETCOMPONENTPROGRAMSTAGESHANDLEIMGPROC ) (GLuint component, GLsizei numPrograms, const GLenum* stages, const GLuint* programs);
typedef void (* PFNGLGETPROGRAMI_VIMGPROC ) (GLuint program, GLenum pname, GLuint index, GLint *params);

#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glRayBounceLimitIMG(GLuint limit);
GL_APICALL GLuint GL_APIENTRY glCreateSceneArrayIMG(void);
GL_APICALL void GL_APIENTRY glDeleteSceneArrayIMG(GLuint sceneArray);
GL_APICALL GLboolean GL_APIENTRY glIsSceneArrayIMG(GLuint sceneArray);
GL_APICALL void GL_APIENTRY glBindSceneArrayIMG(GLuint sceneArray);
GL_APICALL void GL_APIENTRY glSceneArrayRayBlockSizeIMG(GLuint sceneArray, GLuint rayTypeIndex, GLsizei size);
GL_APICALL void GL_APIENTRY glGetSceneArrayInfoiIMG(GLuint sceneArray, GLenum pname, GLuint index, GLint* params);
GL_APICALL void GL_APIENTRY glDispatchRaysIMG(GLuint id, GLuint x, GLuint y, GLuint width, GLuint height, GLbitfield flags);

GL_APICALL void GL_APIENTRY glCreateComponentsIMG(GLsizei n, GLuint *components);
GL_APICALL void GL_APIENTRY glDeleteComponentsIMG(GLsizei n, const GLuint* components);
GL_APICALL void GL_APIENTRY glComponentOccluderIMG(GLuint component, GLboolean occluder);
GL_APICALL void GL_APIENTRY glComponentRayVisibilityIMG(GLuint component, GLuint rayTypeIndex, GLboolean visible);
GL_APICALL void GL_APIENTRY glComponentFrontFaceIMG(GLuint component, GLenum mode);
GL_APICALL void GL_APIENTRY glComponentVisibleFaceIMG(GLuint component, GLenum mode);
GL_APICALL void GL_APIENTRY glComponentVertexArrayIMG(GLuint component, GLuint vertexArray);
GL_APICALL void GL_APIENTRY glComponentGeometryIMG(GLuint component, GLenum mode, GLint first, GLsizei count);
GL_APICALL void GL_APIENTRY glComponentIndexedGeometryIMG(GLuint component,
                                                       GLenum mode,
                                                       GLsizei count,
                                                       GLenum type,
                                                       const GLintptr indices,
                                                       GLint baseVertex);
GL_APICALL void GL_APIENTRY glComponentBufferRangeIMG(GLuint component, GLenum target, GLuint index,
                                        GLuint buffer, GLintptr offset, GLsizeiptr size);
GL_APICALL GLboolean GL_APIENTRY glIsComponentIMG(GLuint component);
GL_APICALL void GL_APIENTRY glGetComponentInfoivIMG(GLuint component, GLenum pname, GLuint* params);
GL_APICALL void GL_APIENTRY glGetComponentInfoi_vIMG(GLuint component, GLenum pname, GLuint index, GLuint* params);
GL_APICALL void GL_APIENTRY glProgramMaxRayEmitsIMG(GLuint program,
    GLuint inRayTypeIndex,
    GLuint value);
GL_APICALL GLuint64 GL_APIENTRY glGetRayProgramHandleIMG(GLuint rayProgram);
GL_APICALL void GL_APIENTRY glRayProgramBufferRangeIMG(GLuint program, GLenum target, GLuint index,
                                                GLuint buffer, GLintptr offset, GLsizeiptr size);
GL_APICALL void GL_APIENTRY glCreateComponentGroupsIMG(GLsizei n, GLuint *componentGroups);
GL_APICALL void GL_APIENTRY glDeleteComponentGroupsIMG(GLsizei n, const GLuint *componentGroups);
GL_APICALL GLboolean GL_APIENTRY glIsComponentGroupIMG(GLuint group);
GL_APICALL void GL_APIENTRY glComponentGroupExtentIMG(GLuint componentGroup, const GLfloat* min, const GLfloat* max);
GL_APICALL GLuint64 GL_APIENTRY glGetComponentGroupHandleIMG(GLuint componentGroup);
GL_APICALL void GL_APIENTRY glBuildComponentGroupIMG(GLuint id, GLuint componentGroup, GLsizei numComponents, const void* components);
GL_APICALL void GL_APIENTRY glBuildComponentGroupBinaryIMG(GLuint id, GLuint componentGroup, GLsizei numComponents, const void* components, const void* binary);
GL_APICALL void GL_APIENTRY glMergeComponentGroupsIMG(GLuint componentGroupDestination, GLsizei numSourceComponentGroups, const void* componentGroups, GLenum quality);
GL_APICALL void GL_APIENTRY glDumpComponentGroupIMG(GLuint componentGroup, void** binary, GLsizei *size);
GL_APICALL void GL_APIENTRY glBindSceneArrayComponentGroupIMG(GLuint sceneArray, GLuint index, GLuint componentGroup);
GL_APICALL void GL_APIENTRY glGetComponentGroupExtentIMG(GLuint componentGroup, GLfloat* min, GLfloat* max);
GL_APICALL void GL_APIENTRY glGetComponentGroupExtentPrecisionIMG(GLint* range, GLint* precision);
GL_APICALL GLuint64 GL_APIENTRY glGetComponentProgramHandleIMG(GLuint component, GLuint program);
GL_APICALL GLuint64 GL_APIENTRY glGetComponentProgramStagesHandleIMG(GLuint component, GLsizei numPrograms, const GLenum* stages, const GLuint* programs); 
GL_APICALL void GL_APIENTRY glGetProgrami_vIMG(GLuint program, GLenum pname, GLuint index, GLint *params);
#endif


/*------------------------------------------------------------------------*
 * XXX extension tokens
 *------------------------------------------------------------------------*/

/*------------------------------------------------------------------------*
 * OES extension functions
 *------------------------------------------------------------------------*/

/*------------------------------------------------------------------------*
 * KHR extension functions
 *------------------------------------------------------------------------*/

 /*------------------------------------------------------------------------*
 * IMG extension functions
 *------------------------------------------------------------------------*/
 
/* GL_IMG_framebuffer_downsample */
#ifndef GL_IMG_framebuffer_downsample
#define GL_IMG_framebuffer_downsample 1
#ifdef GL_GLEXT_PROTOTYPES
GL_API_EXT void GL_APIENTRY glFramebufferTexture2DDownsampleIMG(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLuint xscale, GLuint yscale);
GL_API_EXT void GL_APIENTRY glFramebufferTextureLayerDownsampleIMG(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer, GLuint xscale, GLuint yscale);
#endif
#endif

#undef GL_APICALL
#define GL_APICALL KHRONOS_APICALL
#if defined _MSC_VER
#pragma pop_macro("GL_APICALL")
#endif

#endif /* __drv_gl3ext_h__ */
