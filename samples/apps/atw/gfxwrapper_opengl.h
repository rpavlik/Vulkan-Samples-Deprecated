/*
================================================================================================

Description :   Convenient wrapper for the OpenGL API.
Author      :   J.M.P. van Waveren
Date        :   12/21/2014
Language    :   C99
Format      :   Real tabs with the tab size equal to 4 spaces.
Copyright   :   Copyright (c) 2016 Oculus VR, LLC. All Rights reserved.
            :   Portions copyright (c) 2016 The Brenwill Workshop Ltd. All Rights reserved.


LICENSE
=======

Copyright (c) 2016 Oculus VR, LLC.
Portions of macOS, iOS, functionality copyright (c) 2016 The Brenwill Workshop Ltd.

SPDX-License-Identifier: Apache-2.0

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.


IMPLEMENTATION
==============

The code is written in an object-oriented style with a focus on minimizing state
and side effects. The majority of the functions manipulate self-contained objects
without modifying any global state (except for OpenGL state). The types
introduced in this file have no circular dependencies, and there are no forward
declarations.

Even though an object-oriented style is used, the code is written in straight C99 for
maximum portability and readability. To further improve portability and to simplify
compilation, all source code is in a single file without any dependencies on third-
party code or non-standard libraries. The code does not use an OpenGL loading library
like GLEE, GLEW, GL3W, or an OpenGL toolkit like GLUT, FreeGLUT, GLFW, etc. Instead,
the code provides direct access to window and context creation for driver extension work.

The code is written against version 4.3 of the Core Profile OpenGL Specification,
and version 3.1 of the OpenGL ES Specification.

Supported platforms are:

        - Microsoft Windows 7 or later
        - Ubuntu Linux 14.04 or later
        - Apple macOS 10.11 or later
        - Apple iOS 9.0 or later
        - Android 5.0 or later


GRAPHICS API WRAPPER
====================

The code wraps the OpenGL API with a convenient wrapper that takes care of a
lot of the OpenGL intricacies. This wrapper does not expose the full OpenGL API
but can be easily extended to support more features. Some of the current
limitations are:

- The wrapper is setup for forward rendering with a single render pass. This
  can be easily extended if more complex rendering algorithms are desired.

- A pipeline can only use 256 bytes worth of plain integer and floating-point
  uniforms, including vectors and matrices. If more uniforms are needed then
  it is advised to use a uniform buffer, which is the preferred approach for
  exposing large amounts of data anyway.

- Graphics programs currently consist of only a vertex and fragment shader.
  This can be easily extended if there is a need for geometry shaders etc.


KNOWN ISSUES
============

OS     : Apple Mac OS X 10.9.5
GPU    : Geforce GT 750M
DRIVER : NVIDIA 310.40.55b01
-----------------------------------------------
- glGetQueryObjectui64v( query, GL_QUERY_RESULT, &time ) always returns zero for a timer query.
- glFlush() after a glFenceSync() stalls the CPU for many milliseconds.
- Creating a context fails when the share context is current on another thread.

OS     : Android 6.0.1
GPU    : Adreno (TM) 530
DRIVER : OpenGL ES 3.1 V@145.0
-----------------------------------------------
- Enabling OVR_multiview hangs the GPU.


WORK ITEMS
==========

- Implement WGL, GLX and NSOpenGL equivalents of EGL_IMG_context_priority.
- Implement an extension that provides accurate display refresh timing (WGL_NV_delay_before_swap, D3DKMTGetScanLine).
- Implement an OpenGL extension that allows rendering directly to the front buffer.
- Implement an OpenGL extension that allows a compute shader to directly write to the front/back buffer images
(WGL_AMDX_drawable_view).
- Improve GPU task switching granularity.

================================================================================================
*/

#if !defined(KSGRAPHICSWRAPPER_OPENGL_H)
#define KSGRAPHICSWRAPPER_OPENGL_H

#include "gfxwrapper_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#define OS_WINDOWS
#elif defined(__ANDROID__)
#define OS_ANDROID
#elif defined(__APPLE__)
#define OS_APPLE
#include <Availability.h>
#if __IPHONE_OS_VERSION_MAX_ALLOWED
#define OS_APPLE_IOS
#elif __MAC_OS_X_VERSION_MAX_ALLOWED
#define OS_APPLE_MACOS
#endif
#elif defined(__linux__)
#define OS_LINUX
#else
#error "unknown platform"
#endif

/*
================================
Platform headers / declarations
================================
*/

#if defined(OS_WINDOWS)

#define XR_USE_PLATFORM_WIN32 1

#if !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#if defined(_MSC_VER)
#pragma warning(disable : 4204)  // nonstandard extension used : non-constant aggregate initializer
#pragma warning(disable : 4221)  // nonstandard extension used: 'layers': cannot be initialized using address of automatic variable
                                 // 'layerProjection'
#pragma warning(disable : 4255)  // '<name>' : no function prototype given: converting '()' to '(void)'
#pragma warning(disable : 4668)  // '__cplusplus' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
#pragma warning(disable : 4710)  // 'int printf(const char *const ,...)': function not inlined
#pragma warning(disable : 4711)  // function '<name>' selected for automatic inline expansion
#pragma warning(disable : 4738)  // storing 32-bit float result in memory, possible loss of performance
#pragma warning(disable : 4820)  // '<name>' : 'X' bytes padding added after data member '<member>'
#pragma warning(disable : 4505)  // unreferenced local function has been removed
#endif

#if _MSC_VER >= 1900
#pragma warning(disable : 4464)  // relative include path contains '..'
#pragma warning(disable : 4774)  // 'printf' : format string expected in argument 1 is not a string literal
#endif

#define OPENGL_VERSION_MAJOR 4
#define OPENGL_VERSION_MINOR 3
#define GLSL_VERSION "430"
#define SPIRV_VERSION "99"
#define USE_SYNC_OBJECT 0  // 0 = GLsync, 1 = EGLSyncKHR, 2 = storage buffer

#include <windows.h>
#include <GL/gl.h>
#define GL_EXT_color_subtable
#include <GL/glext.h>
#include <GL/wglext.h>
#include <GL/gl_format.h>

#define GRAPHICS_API_OPENGL 1
#define OUTPUT_PATH ""

#define __thread __declspec(thread)

#elif defined(OS_LINUX)

#define OPENGL_VERSION_MAJOR 4
#define OPENGL_VERSION_MINOR 5
#define GLSL_VERSION "430"
#define SPIRV_VERSION "99"
#define USE_SYNC_OBJECT 0  // 0 = GLsync, 1 = EGLSyncKHR, 2 = storage buffer

#if !defined(_XOPEN_SOURCE)
#if __STDC_VERSION__ >= 199901L
#define _XOPEN_SOURCE 600
#else
#define _XOPEN_SOURCE 500
#endif
#endif

#include <time.h>      // for timespec
#include <sys/time.h>  // for gettimeofday()
#if !defined(__USE_UNIX98)
#define __USE_UNIX98 1  // for pthread_mutexattr_settype
#endif
#include <pthread.h>  // for pthread_create() etc.
#include <malloc.h>   // for memalign
#if defined(OS_LINUX_XLIB)
#define XR_USE_PLATFORM_XLIB 1

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/xf86vmode.h>  // for fullscreen video mode
#include <X11/extensions/Xrandr.h>     // for resolution changes
#include <GL/glx.h>

#elif defined(OS_LINUX_XCB) || defined(OS_LINUX_XCB_GLX)
#define XR_USE_PLATFORM_XCB 1

#include <X11/keysym.h>
#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xcb_icccm.h>
#include <xcb/randr.h>
#include <xcb/glx.h>
#include <xcb/dri2.h>
#include <GL/glx.h>

#elif defined(OS_LINUX_WAYLAND)
#define XR_USE_PLATFORM_WAYLAND 1

#include <wayland-client.h>
#include <wayland-client-protocol.h>
#include <wayland-egl.h>
#include <EGL/egl.h>
#include <EGL/eglplatform.h>
#include <GL/gl.h>
#include <linux/input.h>
#include <poll.h>
#include <unistd.h>
#include "xdg-shell-unstable-v6.h"

#endif

#include <GL/gl_format.h>

#define GRAPHICS_API_OPENGL 1
#define OUTPUT_PATH ""

#if !defined(__USE_GNU)
// These prototypes are only included when __USE_GNU is defined but that causes other compile errors.
extern int pthread_setname_np(pthread_t __target_thread, __const char *__name);
extern int pthread_setaffinity_np(pthread_t thread, size_t cpusetsize, const cpu_set_t *cpuset);
#endif  // !__USE_GNU

#pragma GCC diagnostic ignored "-Wunused-function"

#elif defined(OS_APPLE_MACOS)

// Apple is still at OpenGL 4.1
#define OPENGL_VERSION_MAJOR 4
#define OPENGL_VERSION_MINOR 1
#define GLSL_VERSION "410"
#define SPIRV_VERSION "99"
#define USE_SYNC_OBJECT 0  // 0 = GLsync, 1 = EGLSyncKHR, 2 = storage buffer
#define XR_USE_PLATFORM_MACOS 1

#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <pthread.h>
#include <Cocoa/Cocoa.h>
#define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>
#include <GL/gl_format.h>

#undef MAX
#undef MIN

#define GRAPHICS_API_OPENGL 1
#define OUTPUT_PATH ""

// Undocumented CGS and CGL
typedef void *CGSConnectionID;
typedef int CGSWindowID;
typedef int CGSSurfaceID;

CGLError CGLSetSurface(CGLContextObj ctx, CGSConnectionID cid, CGSWindowID wid, CGSSurfaceID sid);
CGLError CGLGetSurface(CGLContextObj ctx, CGSConnectionID *cid, CGSWindowID *wid, CGSSurfaceID *sid);
CGLError CGLUpdateContext(CGLContextObj ctx);

#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wunused-const-variable"

#elif defined(OS_APPLE_IOS)

// Assume iOS 7+ which is GLES 3.0
#define OPENGL_VERSION_MAJOR 3
#define OPENGL_VERSION_MINOR 0
#define GLSL_VERSION "300 es"
#define SPIRV_VERSION "99"
#define USE_SYNC_OBJECT 0  // 0 = GLsync, 1 = EGLSyncKHR, 2 = storage buffer
#define XR_USE_PLATFORM_IOS 1

#import <Foundation/Foundation.h>
#import <QuartzCore/QuartzCore.h>
#import <UIKit/UIKit.h>
#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#include <sys/sysctl.h>

#define GRAPHICS_API_OPENGL_ES 1

#elif defined(OS_ANDROID)

#define OPENGL_VERSION_MAJOR 3
#define OPENGL_VERSION_MINOR 2
#define GLSL_VERSION "320 es"
#define SPIRV_VERSION "99"
#define USE_SYNC_OBJECT 1  // 0 = GLsync, 1 = EGLSyncKHR, 2 = storage buffer

#include <time.h>
#include <unistd.h>
#include <dirent.h>  // for opendir/closedir
#include <pthread.h>
#include <malloc.h>                     // for memalign
#include <dlfcn.h>                      // for dlopen
#include <sys/prctl.h>                  // for prctl( PR_SET_NAME )
#include <sys/stat.h>                   // for gettid
#include <sys/syscall.h>                // for syscall
#include <android/log.h>                // for __android_log_print
#include <android/input.h>              // for AKEYCODE_ etc.
#include <android/window.h>             // for AWINDOW_FLAG_KEEP_SCREEN_ON
#include <android/native_window_jni.h>  // for native window JNI
#include <android_native_app_glue.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>
#if OPENGL_VERSION_MAJOR == 3
#if OPENGL_VERSION_MINOR == 1
#include <GLES3/gl31.h>
#elif OPENGL_VERSION_MINOR == 2
#include <GLES3/gl32.h>
#endif
#endif
#include <GLES3/gl3ext.h>
#include <GL/gl_format.h>

#define GRAPHICS_API_OPENGL_ES 1
#define OUTPUT_PATH "/sdcard/"

#pragma GCC diagnostic ignored "-Wunused-function"

typedef struct {
    JavaVM *vm;        // Java Virtual Machine
    JNIEnv *env;       // Thread specific environment
    jobject activity;  // Java activity object
} Java_t;

#endif

/*
================================
Common headers
================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>
#include <string.h>  // for memset
#include <errno.h>   // for EBUSY, ETIMEDOUT etc.
#include <ctype.h>   // for isspace, isdigit

#include <utils/sysinfo.h>
#include <utils/nanoseconds.h>
#include <utils/threading.h>
#include <utils/algebra.h>

/*
================================
Common defines
================================
*/

#define UNUSED_PARM(x) \
    { (void)(x); }
#define ARRAY_SIZE(a) (sizeof((a)) / sizeof((a)[0]))
#define OFFSETOF_MEMBER(type, member) (size_t) & ((type *)0)->member
#define SIZEOF_MEMBER(type, member) sizeof(((type *)0)->member)
#define BIT(x) (1 << (x))
#define ROUNDUP(x, granularity) (((x) + (granularity)-1) & ~((granularity)-1))
#ifndef MAX
#define MAX(x, y) ((x > y) ? (x) : (y))
#endif
#ifndef MIN
#define MIN(x, y) ((x < y) ? (x) : (y))
#endif
#define CLAMP(x, min, max) (((x) < (min)) ? (min) : (((x) > (max)) ? (max) : (x)))
#define STRINGIFY_EXPANDED(a) #a
#define STRINGIFY(a) STRINGIFY_EXPANDED(a)

#define APPLICATION_NAME "OpenGL SI"
#define WINDOW_TITLE "OpenGL SI"

#define PROGRAM(name) name##GLSL

#define GLSL_EXTENSIONS "#extension GL_EXT_shader_io_blocks : enable\n"
#define GL_FINISH_SYNC 1

#if defined(OS_ANDROID)
#define ES_HIGHP "highp"  // GLSL "310 es" requires a precision qualifier on a image2D
#else
#define ES_HIGHP ""  // GLSL "430" disallows a precision qualifier on a image2D
#endif

void GlInitExtensions();

/*
================================================================================================================================

OpenGL extensions.

================================================================================================================================
*/

/*
================================
Multi-view support
================================
*/

#if !defined(GL_OVR_multiview)
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_NUM_VIEWS_OVR 0x9630
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_BASE_VIEW_INDEX_OVR 0x9632
#define GL_MAX_VIEWS_OVR 0x9631

typedef void (*PFNGLFRAMEBUFFERTEXTUREMULTIVIEWOVRPROC)(GLenum target, GLenum attachment, GLuint texture, GLint level,
                                                        GLint baseViewIndex, GLsizei numViews);
#endif

/*
================================
Multi-sampling support
================================
*/

#if !defined(GL_EXT_framebuffer_multisample)
typedef void (*PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width,
                                                           GLsizei height);
#endif

#if !defined(GL_EXT_multisampled_render_to_texture)
typedef void (*PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture,
                                                            GLint level, GLsizei samples);
#endif

#if !defined(GL_OVR_multiview_multisampled_render_to_texture)
typedef void (*PFNGLFRAMEBUFFERTEXTUREMULTISAMPLEMULTIVIEWOVRPROC)(GLenum target, GLenum attachment, GLuint texture, GLint level,
                                                                   GLsizei samples, GLint baseViewIndex, GLsizei numViews);
#endif

#if defined(OS_WINDOWS) || defined(OS_LINUX)

extern PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
extern PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
extern PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
extern PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer;
extern PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
extern PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers;
extern PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
extern PFNGLISRENDERBUFFERPROC glIsRenderbuffer;
extern PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage;
extern PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC glRenderbufferStorageMultisample;
extern PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC glRenderbufferStorageMultisampleEXT;
extern PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;
extern PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
extern PFNGLFRAMEBUFFERTEXTURELAYERPROC glFramebufferTextureLayer;
extern PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC glFramebufferTexture2DMultisampleEXT;
extern PFNGLFRAMEBUFFERTEXTUREMULTIVIEWOVRPROC glFramebufferTextureMultiviewOVR;
extern PFNGLFRAMEBUFFERTEXTUREMULTISAMPLEMULTIVIEWOVRPROC glFramebufferTextureMultisampleMultiviewOVR;
extern PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
extern PFNGLCHECKNAMEDFRAMEBUFFERSTATUSPROC glCheckNamedFramebufferStatus;

extern PFNGLGENBUFFERSPROC glGenBuffers;
extern PFNGLDELETEBUFFERSPROC glDeleteBuffers;
extern PFNGLBINDBUFFERPROC glBindBuffer;
extern PFNGLBINDBUFFERBASEPROC glBindBufferBase;
extern PFNGLBUFFERDATAPROC glBufferData;
extern PFNGLBUFFERSUBDATAPROC glBufferSubData;
extern PFNGLBUFFERSTORAGEPROC glBufferStorage;
extern PFNGLMAPBUFFERPROC glMapBuffer;
extern PFNGLMAPBUFFERRANGEPROC glMapBufferRange;
extern PFNGLUNMAPBUFFERPROC glUnmapBuffer;

extern PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
extern PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
extern PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisor;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;

#if defined(OS_WINDOWS)
extern PFNGLACTIVETEXTUREPROC glActiveTexture;
extern PFNGLTEXIMAGE3DPROC glTexImage3D;
extern PFNGLCOMPRESSEDTEXIMAGE2DPROC glCompressedTexImage2D;
extern PFNGLCOMPRESSEDTEXIMAGE3DPROC glCompressedTexImage3D;
extern PFNGLTEXSUBIMAGE3DPROC glTexSubImage3D;
extern PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC glCompressedTexSubImage2D;
extern PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC glCompressedTexSubImage3D;
#endif
extern PFNGLTEXSTORAGE2DPROC glTexStorage2D;
extern PFNGLTEXSTORAGE3DPROC glTexStorage3D;
extern PFNGLTEXIMAGE2DMULTISAMPLEPROC glTexImage2DMultisample;
extern PFNGLTEXIMAGE3DMULTISAMPLEPROC glTexImage3DMultisample;
extern PFNGLTEXSTORAGE2DMULTISAMPLEPROC glTexStorage2DMultisample;
extern PFNGLTEXSTORAGE3DMULTISAMPLEPROC glTexStorage3DMultisample;
extern PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
extern PFNGLBINDIMAGETEXTUREPROC glBindImageTexture;

extern PFNGLCREATEPROGRAMPROC glCreateProgram;
extern PFNGLDELETEPROGRAMPROC glDeleteProgram;
extern PFNGLCREATESHADERPROC glCreateShader;
extern PFNGLDELETESHADERPROC glDeleteShader;
extern PFNGLSHADERSOURCEPROC glShaderSource;
extern PFNGLCOMPILESHADERPROC glCompileShader;
extern PFNGLGETSHADERIVPROC glGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
extern PFNGLUSEPROGRAMPROC glUseProgram;
extern PFNGLATTACHSHADERPROC glAttachShader;
extern PFNGLLINKPROGRAMPROC glLinkProgram;
extern PFNGLGETPROGRAMIVPROC glGetProgramiv;
extern PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
extern PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
extern PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation;
extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
extern PFNGLGETUNIFORMBLOCKINDEXPROC glGetUniformBlockIndex;
extern PFNGLGETPROGRAMRESOURCEINDEXPROC glGetProgramResourceIndex;
extern PFNGLUNIFORMBLOCKBINDINGPROC glUniformBlockBinding;
extern PFNGLSHADERSTORAGEBLOCKBINDINGPROC glShaderStorageBlockBinding;
extern PFNGLPROGRAMUNIFORM1IPROC glProgramUniform1i;
extern PFNGLUNIFORM1IPROC glUniform1i;
extern PFNGLUNIFORM1IVPROC glUniform1iv;
extern PFNGLUNIFORM2IVPROC glUniform2iv;
extern PFNGLUNIFORM3IVPROC glUniform3iv;
extern PFNGLUNIFORM4IVPROC glUniform4iv;
extern PFNGLUNIFORM1FPROC glUniform1f;
extern PFNGLUNIFORM1FVPROC glUniform1fv;
extern PFNGLUNIFORM2FVPROC glUniform2fv;
extern PFNGLUNIFORM3FVPROC glUniform3fv;
extern PFNGLUNIFORM4FVPROC glUniform4fv;
extern PFNGLUNIFORMMATRIX2FVPROC glUniformMatrix2fv;
extern PFNGLUNIFORMMATRIX2X3FVPROC glUniformMatrix2x3fv;
extern PFNGLUNIFORMMATRIX2X4FVPROC glUniformMatrix2x4fv;
extern PFNGLUNIFORMMATRIX3X2FVPROC glUniformMatrix3x2fv;
extern PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv;
extern PFNGLUNIFORMMATRIX3X4FVPROC glUniformMatrix3x4fv;
extern PFNGLUNIFORMMATRIX4X2FVPROC glUniformMatrix4x2fv;
extern PFNGLUNIFORMMATRIX4X3FVPROC glUniformMatrix4x3fv;
extern PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;

extern PFNGLDRAWELEMENTSINSTANCEDPROC glDrawElementsInstanced;
extern PFNGLDISPATCHCOMPUTEPROC glDispatchCompute;
extern PFNGLMEMORYBARRIERPROC glMemoryBarrier;

extern PFNGLGENQUERIESPROC glGenQueries;
extern PFNGLDELETEQUERIESPROC glDeleteQueries;
extern PFNGLISQUERYPROC glIsQuery;
extern PFNGLBEGINQUERYPROC glBeginQuery;
extern PFNGLENDQUERYPROC glEndQuery;
extern PFNGLQUERYCOUNTERPROC glQueryCounter;
extern PFNGLGETQUERYIVPROC glGetQueryiv;
extern PFNGLGETQUERYOBJECTIVPROC glGetQueryObjectiv;
extern PFNGLGETQUERYOBJECTUIVPROC glGetQueryObjectuiv;
extern PFNGLGETQUERYOBJECTI64VPROC glGetQueryObjecti64v;
extern PFNGLGETQUERYOBJECTUI64VPROC glGetQueryObjectui64v;

extern PFNGLFENCESYNCPROC glFenceSync;
extern PFNGLCLIENTWAITSYNCPROC glClientWaitSync;
extern PFNGLDELETESYNCPROC glDeleteSync;
extern PFNGLISSYNCPROC glIsSync;

extern PFNGLBLENDFUNCSEPARATEPROC glBlendFuncSeparate;
extern PFNGLBLENDEQUATIONSEPARATEPROC glBlendEquationSeparate;

extern PFNGLDEBUGMESSAGECONTROLPROC glDebugMessageControl;
extern PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback;

#if defined(OS_WINDOWS)
extern PFNGLBLENDCOLORPROC glBlendColor;
extern PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
extern PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
extern PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
extern PFNWGLDELAYBEFORESWAPNVPROC wglDelayBeforeSwapNV;
#elif defined(OS_LINUX) && !defined(OS_LINUX_WAYLAND)
extern PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB;
extern PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT;
extern PFNGLXDELAYBEFORESWAPNVPROC glXDelayBeforeSwapNV;
#endif

#elif defined(OS_APPLE_MACOS)

extern PFNGLFRAMEBUFFERTEXTUREMULTIVIEWOVRPROC glFramebufferTextureMultiviewOVR;
extern PFNGLFRAMEBUFFERTEXTUREMULTISAMPLEMULTIVIEWOVRPROC glFramebufferTextureMultisampleMultiviewOVR;
extern PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC glFramebufferTexture2DMultisampleEXT;
extern PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC glRenderbufferStorageMultisampleEXT;

#elif defined(OS_ANDROID)

// GL_EXT_disjoint_timer_query without _EXT
#if !defined(GL_TIMESTAMP)
#define GL_QUERY_COUNTER_BITS GL_QUERY_COUNTER_BITS_EXT
#define GL_TIME_ELAPSED GL_TIME_ELAPSED_EXT
#define GL_TIMESTAMP GL_TIMESTAMP_EXT
#define GL_GPU_DISJOINT GL_GPU_DISJOINT_EXT
#endif

// GL_EXT_buffer_storage without _EXT
#if !defined(GL_BUFFER_STORAGE_FLAGS)
#define GL_MAP_READ_BIT 0x0001                          // GL_MAP_READ_BIT_EXT
#define GL_MAP_WRITE_BIT 0x0002                         // GL_MAP_WRITE_BIT_EXT
#define GL_MAP_PERSISTENT_BIT 0x0040                    // GL_MAP_PERSISTENT_BIT_EXT
#define GL_MAP_COHERENT_BIT 0x0080                      // GL_MAP_COHERENT_BIT_EXT
#define GL_DYNAMIC_STORAGE_BIT 0x0100                   // GL_DYNAMIC_STORAGE_BIT_EXT
#define GL_CLIENT_STORAGE_BIT 0x0200                    // GL_CLIENT_STORAGE_BIT_EXT
#define GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT 0x00004000  // GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT_EXT
#define GL_BUFFER_IMMUTABLE_STORAGE 0x821F              // GL_BUFFER_IMMUTABLE_STORAGE_EXT
#define GL_BUFFER_STORAGE_FLAGS 0x8220                  // GL_BUFFER_STORAGE_FLAGS_EXT
#endif

typedef void(GL_APIENTRY *PFNGLBUFFERSTORAGEEXTPROC)(GLenum target, GLsizeiptr size, const void *data, GLbitfield flags);
typedef void(GL_APIENTRY *PFNGLTEXSTORAGE3DMULTISAMPLEPROC)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width,
                                                            GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);

// EGL_KHR_fence_sync, GL_OES_EGL_sync, VG_KHR_EGL_sync
extern PFNEGLCREATESYNCKHRPROC eglCreateSyncKHR;
extern PFNEGLDESTROYSYNCKHRPROC eglDestroySyncKHR;
extern PFNEGLCLIENTWAITSYNCKHRPROC eglClientWaitSyncKHR;
extern PFNEGLGETSYNCATTRIBKHRPROC eglGetSyncAttribKHR;

// GL_EXT_disjoint_timer_query
extern PFNGLQUERYCOUNTEREXTPROC glQueryCounter;
extern PFNGLGETQUERYOBJECTI64VEXTPROC glGetQueryObjecti64v;
extern PFNGLGETQUERYOBJECTUI64VEXTPROC glGetQueryObjectui64v;

// GL_EXT_buffer_storage
extern PFNGLBUFFERSTORAGEEXTPROC glBufferStorage;

// GL_OVR_multiview
extern PFNGLFRAMEBUFFERTEXTUREMULTIVIEWOVRPROC glFramebufferTextureMultiviewOVR;

// GL_EXT_multisampled_render_to_texture
extern PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC glRenderbufferStorageMultisampleEXT;
extern PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC glFramebufferTexture2DMultisampleEXT;

// GL_OVR_multiview_multisampled_render_to_texture
extern PFNGLFRAMEBUFFERTEXTUREMULTISAMPLEMULTIVIEWOVRPROC glFramebufferTextureMultisampleMultiviewOVR;

#ifndef GL_ES_VERSION_3_2
extern PFNGLTEXSTORAGE3DMULTISAMPLEPROC glTexStorage3DMultisample;
#endif

#if !defined(EGL_OPENGL_ES3_BIT)
#define EGL_OPENGL_ES3_BIT 0x0040
#endif

// GL_EXT_texture_cube_map_array
#if !defined(GL_TEXTURE_CUBE_MAP_ARRAY)
#define GL_TEXTURE_CUBE_MAP_ARRAY 0x9009
#endif

// GL_EXT_texture_filter_anisotropic
#if !defined(GL_TEXTURE_MAX_ANISOTROPY_EXT)
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif

// GL_EXT_texture_border_clamp or GL_OES_texture_border_clamp
#if !defined(GL_CLAMP_TO_BORDER)
#define GL_CLAMP_TO_BORDER 0x812D
#endif

// No 1D textures in OpenGL ES.
#if !defined(GL_TEXTURE_1D)
#define GL_TEXTURE_1D 0x0DE0
#endif

// No 1D texture arrays in OpenGL ES.
#if !defined(GL_TEXTURE_1D_ARRAY)
#define GL_TEXTURE_1D_ARRAY 0x8C18
#endif

// No multi-sampled texture arrays in OpenGL ES.
#if !defined(GL_TEXTURE_2D_MULTISAMPLE_ARRAY)
#define GL_TEXTURE_2D_MULTISAMPLE_ARRAY 0x9102
#endif

#endif

/*
================================================================================================================================

Driver Instance.

ksDriverInstance

bool ksDriverInstance_Create( ksDriverInstance * instance );
void ksDriverInstance_Destroy( ksDriverInstance * instance );

================================================================================================================================
*/

typedef struct {
    int placeholder;
} ksDriverInstance;

bool ksDriverInstance_Create(ksDriverInstance *instance);
void ksDriverInstance_Destroy(ksDriverInstance *instance);

/*
================================================================================================================================

GPU device.

ksGpuQueueProperty
ksGpuQueuePriority
ksGpuQueueInfo
ksGpuDevice

bool ksGpuDevice_Create( ksGpuDevice * device, ksDriverInstance * instance, const ksGpuQueueInfo * queueInfo );
void ksGpuDevice_Destroy( ksGpuDevice * device );

================================================================================================================================
*/

typedef enum {
    KS_GPU_QUEUE_PROPERTY_GRAPHICS = BIT(0),
    KS_GPU_QUEUE_PROPERTY_COMPUTE = BIT(1),
    KS_GPU_QUEUE_PROPERTY_TRANSFER = BIT(2)
} ksGpuQueueProperty;

typedef enum { KS_GPU_QUEUE_PRIORITY_LOW, KS_GPU_QUEUE_PRIORITY_MEDIUM, KS_GPU_QUEUE_PRIORITY_HIGH } ksGpuQueuePriority;

#define MAX_QUEUES 16

typedef struct {
    int queueCount;                                  // number of queues
    ksGpuQueueProperty queueProperties;              // desired queue family properties
    ksGpuQueuePriority queuePriorities[MAX_QUEUES];  // individual queue priorities
} ksGpuQueueInfo;

typedef struct {
    ksDriverInstance *instance;
    ksGpuQueueInfo queueInfo;
} ksGpuDevice;

bool ksGpuDevice_Create(ksGpuDevice *device, ksDriverInstance *instance, const ksGpuQueueInfo *queueInfo);
void ksGpuDevice_Destroy(ksGpuDevice *device);

/*
================================================================================================================================

GPU context.

A context encapsulates a queue that is used to submit command buffers.
A context can only be used by a single thread.
For optimal performance a context should only be created at load time, not at runtime.

ksGpuContext
ksGpuSurfaceColorFormat
ksGpuSurfaceDepthFormat
ksGpuSampleCount

bool ksGpuContext_CreateShared( ksGpuContext * context, const ksGpuContext * other, const int queueIndex );
void ksGpuContext_Destroy( ksGpuContext * context );
void ksGpuContext_WaitIdle( ksGpuContext * context );
void ksGpuContext_SetCurrent( ksGpuContext * context );
void ksGpuContext_UnsetCurrent( ksGpuContext * context );
bool ksGpuContext_CheckCurrent( ksGpuContext * context );

bool ksGpuContext_CreateForSurface( ksGpuContext * context, const ksGpuDevice * device, const int queueIndex,
                                                                                const ksGpuSurfaceColorFormat colorFormat,
                                                                                const ksGpuSurfaceDepthFormat depthFormat,
                                                                                const ksGpuSampleCount sampleCount,
                                                                                ... );

================================================================================================================================
*/

typedef enum {
    KS_GPU_SURFACE_COLOR_FORMAT_R5G6B5,
    KS_GPU_SURFACE_COLOR_FORMAT_B5G6R5,
    KS_GPU_SURFACE_COLOR_FORMAT_R8G8B8A8,
    KS_GPU_SURFACE_COLOR_FORMAT_B8G8R8A8,
    KS_GPU_SURFACE_COLOR_FORMAT_MAX
} ksGpuSurfaceColorFormat;

typedef enum {
    KS_GPU_SURFACE_DEPTH_FORMAT_NONE,
    KS_GPU_SURFACE_DEPTH_FORMAT_D16,
    KS_GPU_SURFACE_DEPTH_FORMAT_D24,
    KS_GPU_SURFACE_DEPTH_FORMAT_MAX
} ksGpuSurfaceDepthFormat;

typedef enum {
    KS_GPU_SAMPLE_COUNT_1 = 1,
    KS_GPU_SAMPLE_COUNT_2 = 2,
    KS_GPU_SAMPLE_COUNT_4 = 4,
    KS_GPU_SAMPLE_COUNT_8 = 8,
    KS_GPU_SAMPLE_COUNT_16 = 16,
    KS_GPU_SAMPLE_COUNT_32 = 32,
    KS_GPU_SAMPLE_COUNT_64 = 64,
} ksGpuSampleCount;

typedef struct ksGpuLimits {
    size_t maxPushConstantsSize;
    int maxSamples;
} ksGpuLimits;

typedef struct {
    const ksGpuDevice *device;
#if defined(OS_WINDOWS)
    HDC hDC;
    HGLRC hGLRC;
#elif defined(OS_LINUX_XLIB) || defined(OS_LINUX_XCB_GLX)
    Display *xDisplay;
    uint32_t visualid;
    GLXFBConfig glxFBConfig;
    GLXDrawable glxDrawable;
    GLXContext glxContext;
#elif defined(OS_LINUX_XCB)
    xcb_connection_t *connection;
    uint32_t screen_number;
    xcb_glx_fbconfig_t fbconfigid;
    xcb_visualid_t visualid;
    xcb_glx_drawable_t glxDrawable;
    xcb_glx_context_t glxContext;
    xcb_glx_context_tag_t glxContextTag;
#elif defined(OS_LINUX_WAYLAND)
    EGLNativeWindowType native_window;
    EGLDisplay display;
    EGLContext context;
    EGLConfig config;
    EGLSurface mainSurface;
#elif defined(OS_APPLE_MACOS)
    NSOpenGLContext *nsContext;
    CGLContextObj cglContext;
#elif defined(OS_ANDROID)
    EGLDisplay display;
    EGLConfig config;
    EGLSurface tinySurface;
    EGLSurface mainSurface;
    EGLContext context;
#endif
} ksGpuContext;

typedef struct {
    unsigned char redBits;
    unsigned char greenBits;
    unsigned char blueBits;
    unsigned char alphaBits;
    unsigned char colorBits;
    unsigned char depthBits;
} ksGpuSurfaceBits;

bool ksGpuContext_CreateShared(ksGpuContext *context, const ksGpuContext *other, int queueIndex);
void ksGpuContext_Destroy(ksGpuContext *context);
void ksGpuContext_WaitIdle(ksGpuContext *context);
void ksGpuContext_SetCurrent(ksGpuContext *context);
void ksGpuContext_UnsetCurrent(ksGpuContext *context);
bool ksGpuContext_CheckCurrent(ksGpuContext *context);

/*
================================================================================================================================

GPU Window.

Window with associated GPU context for GPU accelerated rendering.
For optimal performance a window should only be created at load time, not at runtime.
Because on some platforms the OS/drivers use thread local storage, ksGpuWindow *must* be created
and destroyed on the same thread that will actually render to the window and swap buffers.

ksGpuWindow
ksGpuWindowEvent
ksGpuWindowInput
ksKeyboardKey
ksMouseButton

bool ksGpuWindow_Create( ksGpuWindow * window, ksDriverInstance * instance,
                                                const ksGpuQueueInfo * queueInfo, const int queueIndex,
                                                const ksGpuSurfaceColorFormat colorFormat, const ksGpuSurfaceDepthFormat
depthFormat,
                                                const ksGpuSampleCount sampleCount, const int width, const int height, const bool
fullscreen );
void ksGpuWindow_Destroy( ksGpuWindow * window );
void ksGpuWindow_Exit( ksGpuWindow * window );
ksGpuWindowEvent ksGpuWindow_ProcessEvents( ksGpuWindow * window );
void ksGpuWindow_SwapInterval( ksGpuWindow * window, const int swapInterval );
void ksGpuWindow_SwapBuffers( ksGpuWindow * window );
ksNanoseconds ksGpuWindow_GetNextSwapTimeNanoseconds( ksGpuWindow * window );
ksNanoseconds ksGpuWindow_GetFrameTimeNanoseconds( ksGpuWindow * window );

================================================================================================================================
*/

typedef enum {
    KS_GPU_WINDOW_EVENT_NONE,
    KS_GPU_WINDOW_EVENT_ACTIVATED,
    KS_GPU_WINDOW_EVENT_DEACTIVATED,
    KS_GPU_WINDOW_EVENT_EXIT
} ksGpuWindowEvent;

typedef struct {
    bool keyInput[256];
    bool mouseInput[8];
    int mouseInputX[8];
    int mouseInputY[8];
} ksGpuWindowInput;

typedef struct {
    ksGpuDevice device;
    ksGpuContext context;
    ksGpuSurfaceColorFormat colorFormat;
    ksGpuSurfaceDepthFormat depthFormat;
    ksGpuSampleCount sampleCount;
    int windowWidth;
    int windowHeight;
    int windowSwapInterval;
    float windowRefreshRate;
    bool windowFullscreen;
    bool windowActive;
    bool windowExit;
    ksGpuWindowInput input;
    ksNanoseconds lastSwapTime;

#if defined(OS_WINDOWS)
    HINSTANCE hInstance;
    HDC hDC;
    HWND hWnd;
    bool windowActiveState;
#elif defined(OS_LINUX_XLIB)
    Display *xDisplay;
    int xScreen;
    Window xRoot;
    XVisualInfo *xVisual;
    Colormap xColormap;
    Window xWindow;
    int desktopWidth;
    int desktopHeight;
    float desktopRefreshRate;
#elif defined(OS_LINUX_XCB) || defined(OS_LINUX_XCB_GLX)
    Display *xDisplay;
    xcb_connection_t *connection;
    xcb_screen_t *screen;
    xcb_colormap_t colormap;
    xcb_window_t window;
    xcb_atom_t wm_delete_window_atom;
    xcb_key_symbols_t *key_symbols;
    xcb_glx_window_t glxWindow;
    int desktopWidth;
    int desktopHeight;
    float desktopRefreshRate;
#elif defined(OS_LINUX_WAYLAND)
    struct wl_display *display;

    struct wl_surface *surface;

    struct wl_registry *registry;
    struct wl_compositor *compositor;
    struct zxdg_shell_v6 *shell;
    struct zxdg_surface_v6 *shell_surface;

    struct wl_keyboard *keyboard;
    struct wl_pointer *pointer;
    struct wl_seat *seat;
#elif defined(OS_APPLE_MACOS)
    CGDirectDisplayID display;
    CGDisplayModeRef desktopDisplayMode;
    NSWindow *nsWindow;
    NSView *nsView;
#elif defined(OS_APPLE_IOS)
    UIWindow *uiWindow;
    UIView *uiView;
#elif defined(OS_ANDROID)
    EGLDisplay display;
    EGLint majorVersion;
    EGLint minorVersion;
    struct android_app *app;
    Java_t java;
    ANativeWindow *nativeWindow;
    bool resumed;
#endif
} ksGpuWindow;

bool ksGpuWindow_Create(ksGpuWindow *window, ksDriverInstance *instance, const ksGpuQueueInfo *queueInfo, int queueIndex,
                        ksGpuSurfaceColorFormat colorFormat, ksGpuSurfaceDepthFormat depthFormat, ksGpuSampleCount sampleCount,
                        int width, int height, bool fullscreen);
void ksGpuWindow_Destroy(ksGpuWindow *window);
void ksGpuWindow_Exit(ksGpuWindow *window);
ksGpuWindowEvent ksGpuWindow_ProcessEvents(ksGpuWindow *window);
void ksGpuWindow_SwapInterval(ksGpuWindow *window, int swapInterval);
void ksGpuWindow_SwapBuffers(ksGpuWindow *window);
ksNanoseconds ksGpuWindow_GetNextSwapTimeNanoseconds(ksGpuWindow *window);
ksNanoseconds ksGpuWindow_GetFrameTimeNanoseconds(ksGpuWindow *window);

/*
================================================================================================================================

GPU timer.

A timer is used to measure the amount of time it takes to complete GPU commands.
For optimal performance a timer should only be created at load time, not at runtime.
To avoid synchronization, ksGpuTimer_GetNanoseconds() reports the time from KS_GPU_TIMER_FRAMES_DELAYED frames ago.
Timer queries are allowed to overlap and can be nested.
Timer queries that are issued inside a render pass may not produce accurate times on tiling GPUs.

ksGpuTimer

void ksGpuTimer_Create( ksGpuContext * context, ksGpuTimer * timer );
void ksGpuTimer_Destroy( ksGpuContext * context, ksGpuTimer * timer );
ksNanoseconds ksGpuTimer_GetNanoseconds( ksGpuTimer * timer );

================================================================================================================================
*/

#define KS_GPU_TIMER_FRAMES_DELAYED 2

typedef struct {
    GLuint beginQueries[KS_GPU_TIMER_FRAMES_DELAYED];
    GLuint endQueries[KS_GPU_TIMER_FRAMES_DELAYED];
    int queryIndex;
    ksNanoseconds gpuTime;
} ksGpuTimer;

void ksGpuTimer_Create(ksGpuContext *context, ksGpuTimer *timer);
void ksGpuTimer_Destroy(ksGpuContext *context, ksGpuTimer *timer);
ksNanoseconds ksGpuTimer_GetNanoseconds(ksGpuTimer *timer);

/*
================================================================================================================================

GPU buffer.

A buffer maintains a block of memory for a specific use by GPU programs (vertex, index, uniform, storage).
For optimal performance a buffer should only be created at load time, not at runtime.
The best performance is typically achieved when the buffer is not host visible.

ksGpuBufferType
ksGpuBuffer

bool ksGpuBuffer_Create( ksGpuContext * context, ksGpuBuffer * buffer, const ksGpuBufferType type,
                                                        const size_t dataSize, const void * data, const bool hostVisible );
void ksGpuBuffer_CreateReference( ksGpuContext * context, ksGpuBuffer * buffer, const ksGpuBuffer * other );
void ksGpuBuffer_Destroy( ksGpuContext * context, ksGpuBuffer * buffer );

================================================================================================================================
*/

typedef enum {
    KS_GPU_BUFFER_TYPE_VERTEX,
    KS_GPU_BUFFER_TYPE_INDEX,
    KS_GPU_BUFFER_TYPE_UNIFORM,
    KS_GPU_BUFFER_TYPE_STORAGE
} ksGpuBufferType;

typedef struct {
    GLuint target;
    GLuint buffer;
    size_t size;
    bool owner;
} ksGpuBuffer;
bool ksGpuBuffer_Create(ksGpuContext *context, ksGpuBuffer *buffer, const ksGpuBufferType type, const size_t dataSize,
                        const void *data, const bool hostVisible);
void ksGpuBuffer_CreateReference(ksGpuContext *context, ksGpuBuffer *buffer, const ksGpuBuffer *other);

void ksGpuBuffer_Destroy(ksGpuContext *context, ksGpuBuffer *buffer);

/*
================================================================================================================================

GPU texture.

Supports loading textures from raw data or KTX container files.
Textures are always created as immutable textures.
For optimal performance a texture should only be created or modified at load time, not at runtime.
Note that the geometry code assumes the texture origin 0,0 = left-top as opposed to left-bottom.
In other words, textures are expected to be stored top-down as opposed to bottom-up.

ksGpuTextureFormat
ksGpuTextureUsage
ksGpuTextureWrapMode
ksGpuTextureFilter
ksGpuTextureDefault
ksGpuTexture

bool ksGpuTexture_Create2D( ksGpuContext * context, ksGpuTexture * texture, const ksGpuTextureFormat format, const ksGpuSampleCount
sampleCount, const int width, const int height, const int mipCount, const ksGpuTextureUsageFlags usageFlags, const void * data,
const size_t dataSize );

bool ksGpuTexture_Create2DArray( ksGpuContext * context, ksGpuTexture * texture, const
ksGpuTextureFormat format, const ksGpuSampleCount sampleCount, const int width, const int height, const int layerCount, const int
mipCount, const ksGpuTextureUsageFlags usageFlags, const void * data, const size_t dataSize );

bool ksGpuTexture_CreateDefault( ksGpuContext * context, ksGpuTexture * texture, const ksGpuTextureDefault defaultType, const int
width, const int height, const int depth, const int layerCount, const int faceCount, const bool mipmaps, const bool border );

bool ksGpuTexture_CreateFromSwapchain( ksGpuContext * context, ksGpuTexture * texture, const ksGpuWindow * window, int index );

bool ksGpuTexture_CreateFromFile( ksGpuContext * context, ksGpuTexture * texture, const char * fileName );

void ksGpuTexture_Destroy( ksGpuContext * context, ksGpuTexture * texture );

void ksGpuTexture_SetFilter( ksGpuContext * context, ksGpuTexture * texture, const ksGpuTextureFilter filter );
void ksGpuTexture_SetAniso( ksGpuContext * context, ksGpuTexture * texture, const float maxAniso );
void ksGpuTexture_SetWrapMode( ksGpuContext * context, ksGpuTexture * texture, const ksGpuTextureWrapMode wrapMode );

================================================================================================================================
*/

// Note that the channel listed first in the name shall occupy the least significant bit.
typedef enum {
    //
    // 8 bits per component
    //
    KS_GPU_TEXTURE_FORMAT_R8_UNORM = GL_R8,           // 1-component, 8-bit unsigned normalized
    KS_GPU_TEXTURE_FORMAT_R8G8_UNORM = GL_RG8,        // 2-component, 8-bit unsigned normalized
    KS_GPU_TEXTURE_FORMAT_R8G8B8A8_UNORM = GL_RGBA8,  // 4-component, 8-bit unsigned normalized

    KS_GPU_TEXTURE_FORMAT_R8_SNORM = GL_R8_SNORM,           // 1-component, 8-bit signed normalized
    KS_GPU_TEXTURE_FORMAT_R8G8_SNORM = GL_RG8_SNORM,        // 2-component, 8-bit signed normalized
    KS_GPU_TEXTURE_FORMAT_R8G8B8A8_SNORM = GL_RGBA8_SNORM,  // 4-component, 8-bit signed normalized

    KS_GPU_TEXTURE_FORMAT_R8_UINT = GL_R8UI,           // 1-component, 8-bit unsigned integer
    KS_GPU_TEXTURE_FORMAT_R8G8_UINT = GL_RG8UI,        // 2-component, 8-bit unsigned integer
    KS_GPU_TEXTURE_FORMAT_R8G8B8A8_UINT = GL_RGBA8UI,  // 4-component, 8-bit unsigned integer

    KS_GPU_TEXTURE_FORMAT_R8_SINT = GL_R8I,           // 1-component, 8-bit signed integer
    KS_GPU_TEXTURE_FORMAT_R8G8_SINT = GL_RG8I,        // 2-component, 8-bit signed integer
    KS_GPU_TEXTURE_FORMAT_R8G8B8A8_SINT = GL_RGBA8I,  // 4-component, 8-bit signed integer

#if defined(GL_SR8)
    KS_GPU_TEXTURE_FORMAT_R8_SRGB = GL_SR8,     // 1-component, 8-bit sRGB
    KS_GPU_TEXTURE_FORMAT_R8G8_SRGB = GL_SRG8,  // 2-component, 8-bit sRGB
#elif defined(GL_SR8_EXT)
    KS_GPU_TEXTURE_FORMAT_R8_SRGB = GL_SR8_EXT,                      // 1-component, 8-bit sRGB
    KS_GPU_TEXTURE_FORMAT_R8G8_SRGB = GL_SRG8_EXT,                   // 2-component, 8-bit sRGB
#endif
    KS_GPU_TEXTURE_FORMAT_R8G8B8A8_SRGB = GL_SRGB8_ALPHA8,  // 4-component, 8-bit sRGB

//
// 16 bits per component
//
#if defined(GL_R16)
    KS_GPU_TEXTURE_FORMAT_R16_UNORM = GL_R16,              // 1-component, 16-bit unsigned normalized
    KS_GPU_TEXTURE_FORMAT_R16G16_UNORM = GL_RG16,          // 2-component, 16-bit unsigned normalized
    KS_GPU_TEXTURE_FORMAT_R16G16B16A16_UNORM = GL_RGBA16,  // 4-component, 16-bit unsigned normalized
#elif defined(GL_R16_EXT)
    KS_GPU_TEXTURE_FORMAT_R16_UNORM = GL_R16_EXT,                    // 1-component, 16-bit unsigned normalized
    KS_GPU_TEXTURE_FORMAT_R16G16_UNORM = GL_RG16_EXT,                // 2-component, 16-bit unsigned normalized
    KS_GPU_TEXTURE_FORMAT_R16G16B16A16_UNORM = GL_RGBA16_EXT,        // 4-component, 16-bit unsigned normalized
#endif

#if defined(GL_R16_SNORM)
    KS_GPU_TEXTURE_FORMAT_R16_SNORM = GL_R16_SNORM,              // 1-component, 16-bit signed normalized
    KS_GPU_TEXTURE_FORMAT_R16G16_SNORM = GL_RG16_SNORM,          // 2-component, 16-bit signed normalized
    KS_GPU_TEXTURE_FORMAT_R16G16B16A16_SNORM = GL_RGBA16_SNORM,  // 4-component, 16-bit signed normalized
#elif defined(GL_R16_SNORM_EXT)
    KS_GPU_TEXTURE_FORMAT_R16_SNORM = GL_R16_SNORM_EXT,              // 1-component, 16-bit signed normalized
    KS_GPU_TEXTURE_FORMAT_R16G16_SNORM = GL_RG16_SNORM_EXT,          // 2-component, 16-bit signed normalized
    KS_GPU_TEXTURE_FORMAT_R16G16B16A16_SNORM = GL_RGBA16_SNORM_EXT,  // 4-component, 16-bit signed normalized
#endif

    KS_GPU_TEXTURE_FORMAT_R16_UINT = GL_R16UI,              // 1-component, 16-bit unsigned integer
    KS_GPU_TEXTURE_FORMAT_R16G16_UINT = GL_RG16UI,          // 2-component, 16-bit unsigned integer
    KS_GPU_TEXTURE_FORMAT_R16G16B16A16_UINT = GL_RGBA16UI,  // 4-component, 16-bit unsigned integer

    KS_GPU_TEXTURE_FORMAT_R16_SINT = GL_R16I,              // 1-component, 16-bit signed integer
    KS_GPU_TEXTURE_FORMAT_R16G16_SINT = GL_RG16I,          // 2-component, 16-bit signed integer
    KS_GPU_TEXTURE_FORMAT_R16G16B16A16_SINT = GL_RGBA16I,  // 4-component, 16-bit signed integer

    KS_GPU_TEXTURE_FORMAT_R16_SFLOAT = GL_R16F,              // 1-component, 16-bit floating-point
    KS_GPU_TEXTURE_FORMAT_R16G16_SFLOAT = GL_RG16F,          // 2-component, 16-bit floating-point
    KS_GPU_TEXTURE_FORMAT_R16G16B16A16_SFLOAT = GL_RGBA16F,  // 4-component, 16-bit floating-point

    //
    // 32 bits per component
    //
    KS_GPU_TEXTURE_FORMAT_R32_UINT = GL_R32UI,              // 1-component, 32-bit unsigned integer
    KS_GPU_TEXTURE_FORMAT_R32G32_UINT = GL_RG32UI,          // 2-component, 32-bit unsigned integer
    KS_GPU_TEXTURE_FORMAT_R32G32B32A32_UINT = GL_RGBA32UI,  // 4-component, 32-bit unsigned integer

    KS_GPU_TEXTURE_FORMAT_R32_SINT = GL_R32I,              // 1-component, 32-bit signed integer
    KS_GPU_TEXTURE_FORMAT_R32G32_SINT = GL_RG32I,          // 2-component, 32-bit signed integer
    KS_GPU_TEXTURE_FORMAT_R32G32B32A32_SINT = GL_RGBA32I,  // 4-component, 32-bit signed integer

    KS_GPU_TEXTURE_FORMAT_R32_SFLOAT = GL_R32F,              // 1-component, 32-bit floating-point
    KS_GPU_TEXTURE_FORMAT_R32G32_SFLOAT = GL_RG32F,          // 2-component, 32-bit floating-point
    KS_GPU_TEXTURE_FORMAT_R32G32B32A32_SFLOAT = GL_RGBA32F,  // 4-component, 32-bit floating-point

//
// S3TC/DXT/BC
//
#if defined(GL_COMPRESSED_RGB_S3TC_DXT1_EXT)
    KS_GPU_TEXTURE_FORMAT_BC1_R8G8B8_UNORM =
        GL_COMPRESSED_RGB_S3TC_DXT1_EXT,  // 3-component, line through 3D space, unsigned normalized
    KS_GPU_TEXTURE_FORMAT_BC1_R8G8B8A1_UNORM =
        GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,  // 4-component, line through 3D space plus 1-bit alpha, unsigned normalized
    KS_GPU_TEXTURE_FORMAT_BC2_R8G8B8A8_UNORM =
        GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,  // 4-component, line through 3D space plus line through 1D space, unsigned normalized
    KS_GPU_TEXTURE_FORMAT_BC3_R8G8B8A4_UNORM =
        GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,  // 4-component, line through 3D space plus 4-bit alpha, unsigned normalized
#endif

#if defined(GL_COMPRESSED_SRGB_S3TC_DXT1_EXT)
    KS_GPU_TEXTURE_FORMAT_BC1_R8G8B8_SRGB = GL_COMPRESSED_SRGB_S3TC_DXT1_EXT,  // 3-component, line through 3D space, sRGB
    KS_GPU_TEXTURE_FORMAT_BC1_R8G8B8A1_SRGB =
        GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT,  // 4-component, line through 3D space plus 1-bit alpha, sRGB
    KS_GPU_TEXTURE_FORMAT_BC2_R8G8B8A8_SRGB =
        GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT,  // 4-component, line through 3D space plus line through 1D space, sRGB
    KS_GPU_TEXTURE_FORMAT_BC3_R8G8B8A4_SRGB =
        GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT,  // 4-component, line through 3D space plus 4-bit alpha, sRGB
#endif

#if defined(GL_COMPRESSED_LUMINANCE_LATC1_EXT)
    KS_GPU_TEXTURE_FORMAT_BC4_R8_UNORM =
        GL_COMPRESSED_LUMINANCE_LATC1_EXT,  // 1-component, line through 1D space, unsigned normalized
    KS_GPU_TEXTURE_FORMAT_BC5_R8G8_UNORM =
        GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT,  // 2-component, two lines through 1D space, unsigned normalized
    KS_GPU_TEXTURE_FORMAT_BC4_R8_SNORM =
        GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT,  // 1-component, line through 1D space, signed normalized
    KS_GPU_TEXTURE_FORMAT_BC5_R8G8_SNORM =
        GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT,  // 2-component, two lines through 1D space, signed normalized
#endif

//
// ETC
//
#if defined(GL_COMPRESSED_RGB8_ETC2)
    KS_GPU_TEXTURE_FORMAT_ETC2_R8G8B8_UNORM = GL_COMPRESSED_RGB8_ETC2,  // 3-component ETC2, unsigned normalized
    KS_GPU_TEXTURE_FORMAT_ETC2_R8G8B8A1_UNORM =
        GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2,  // 3-component with 1-bit alpha ETC2, unsigned normalized
    KS_GPU_TEXTURE_FORMAT_ETC2_R8G8B8A8_UNORM = GL_COMPRESSED_RGBA8_ETC2_EAC,  // 4-component ETC2, unsigned normalized

    KS_GPU_TEXTURE_FORMAT_ETC2_R8G8B8_SRGB = GL_COMPRESSED_SRGB8_ETC2,  // 3-component ETC2, sRGB
    KS_GPU_TEXTURE_FORMAT_ETC2_R8G8B8A1_SRGB =
        GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2,                                // 3-component with 1-bit alpha ETC2, sRGB
    KS_GPU_TEXTURE_FORMAT_ETC2_R8G8B8A8_SRGB = GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC,  // 4-component ETC2, sRGB
#endif

#if defined(GL_COMPRESSED_R11_EAC)
    KS_GPU_TEXTURE_FORMAT_EAC_R11_UNORM = GL_COMPRESSED_R11_EAC,  // 1-component ETC, line through 1D space, unsigned normalized
    KS_GPU_TEXTURE_FORMAT_EAC_R11G11_UNORM =
        GL_COMPRESSED_RG11_EAC,  // 2-component ETC, two lines through 1D space, unsigned normalized
    KS_GPU_TEXTURE_FORMAT_EAC_R11_SNORM =
        GL_COMPRESSED_SIGNED_R11_EAC,  // 1-component ETC, line through 1D space, signed normalized
    KS_GPU_TEXTURE_FORMAT_EAC_R11G11_SNORM =
        GL_COMPRESSED_SIGNED_RG11_EAC,  // 2-component ETC, two lines through 1D space, signed normalized
#endif

//
// ASTC
//
#if defined(GL_COMPRESSED_RGBA_ASTC_4x4_KHR)
    KS_GPU_TEXTURE_FORMAT_ASTC_4x4_UNORM = GL_COMPRESSED_RGBA_ASTC_4x4_KHR,    // 4-component ASTC, 4x4 blocks, unsigned normalized
    KS_GPU_TEXTURE_FORMAT_ASTC_5x4_UNORM = GL_COMPRESSED_RGBA_ASTC_5x4_KHR,    // 4-component ASTC, 5x4 blocks, unsigned normalized
    KS_GPU_TEXTURE_FORMAT_ASTC_5x5_UNORM = GL_COMPRESSED_RGBA_ASTC_5x5_KHR,    // 4-component ASTC, 5x5 blocks, unsigned normalized
    KS_GPU_TEXTURE_FORMAT_ASTC_6x5_UNORM = GL_COMPRESSED_RGBA_ASTC_6x5_KHR,    // 4-component ASTC, 6x5 blocks, unsigned normalized
    KS_GPU_TEXTURE_FORMAT_ASTC_6x6_UNORM = GL_COMPRESSED_RGBA_ASTC_6x6_KHR,    // 4-component ASTC, 6x6 blocks, unsigned normalized
    KS_GPU_TEXTURE_FORMAT_ASTC_8x5_UNORM = GL_COMPRESSED_RGBA_ASTC_8x5_KHR,    // 4-component ASTC, 8x5 blocks, unsigned normalized
    KS_GPU_TEXTURE_FORMAT_ASTC_8x6_UNORM = GL_COMPRESSED_RGBA_ASTC_8x6_KHR,    // 4-component ASTC, 8x6 blocks, unsigned normalized
    KS_GPU_TEXTURE_FORMAT_ASTC_8x8_UNORM = GL_COMPRESSED_RGBA_ASTC_8x8_KHR,    // 4-component ASTC, 8x8 blocks, unsigned normalized
    KS_GPU_TEXTURE_FORMAT_ASTC_10x5_UNORM = GL_COMPRESSED_RGBA_ASTC_10x5_KHR,  // 4-component ASTC, 10x5 blocks, unsigned normalized
    KS_GPU_TEXTURE_FORMAT_ASTC_10x6_UNORM = GL_COMPRESSED_RGBA_ASTC_10x6_KHR,  // 4-component ASTC, 10x6 blocks, unsigned normalized
    KS_GPU_TEXTURE_FORMAT_ASTC_10x8_UNORM = GL_COMPRESSED_RGBA_ASTC_10x8_KHR,  // 4-component ASTC, 10x8 blocks, unsigned normalized
    KS_GPU_TEXTURE_FORMAT_ASTC_10x10_UNORM =
        GL_COMPRESSED_RGBA_ASTC_10x10_KHR,  // 4-component ASTC, 10x10 blocks, unsigned normalized
    KS_GPU_TEXTURE_FORMAT_ASTC_12x10_UNORM =
        GL_COMPRESSED_RGBA_ASTC_12x10_KHR,  // 4-component ASTC, 12x10 blocks, unsigned normalized
    KS_GPU_TEXTURE_FORMAT_ASTC_12x12_UNORM =
        GL_COMPRESSED_RGBA_ASTC_12x12_KHR,  // 4-component ASTC, 12x12 blocks, unsigned normalized

    KS_GPU_TEXTURE_FORMAT_ASTC_4x4_SRGB = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR,      // 4-component ASTC, 4x4 blocks, sRGB
    KS_GPU_TEXTURE_FORMAT_ASTC_5x4_SRGB = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR,      // 4-component ASTC, 5x4 blocks, sRGB
    KS_GPU_TEXTURE_FORMAT_ASTC_5x5_SRGB = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR,      // 4-component ASTC, 5x5 blocks, sRGB
    KS_GPU_TEXTURE_FORMAT_ASTC_6x5_SRGB = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR,      // 4-component ASTC, 6x5 blocks, sRGB
    KS_GPU_TEXTURE_FORMAT_ASTC_6x6_SRGB = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR,      // 4-component ASTC, 6x6 blocks, sRGB
    KS_GPU_TEXTURE_FORMAT_ASTC_8x5_SRGB = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR,      // 4-component ASTC, 8x5 blocks, sRGB
    KS_GPU_TEXTURE_FORMAT_ASTC_8x6_SRGB = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR,      // 4-component ASTC, 8x6 blocks, sRGB
    KS_GPU_TEXTURE_FORMAT_ASTC_8x8_SRGB = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR,      // 4-component ASTC, 8x8 blocks, sRGB
    KS_GPU_TEXTURE_FORMAT_ASTC_10x5_SRGB = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR,    // 4-component ASTC, 10x5 blocks, sRGB
    KS_GPU_TEXTURE_FORMAT_ASTC_10x6_SRGB = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR,    // 4-component ASTC, 10x6 blocks, sRGB
    KS_GPU_TEXTURE_FORMAT_ASTC_10x8_SRGB = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR,    // 4-component ASTC, 10x8 blocks, sRGB
    KS_GPU_TEXTURE_FORMAT_ASTC_10x10_SRGB = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR,  // 4-component ASTC, 10x10 blocks, sRGB
    KS_GPU_TEXTURE_FORMAT_ASTC_12x10_SRGB = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR,  // 4-component ASTC, 12x10 blocks, sRGB
    KS_GPU_TEXTURE_FORMAT_ASTC_12x12_SRGB = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR,  // 4-component ASTC, 12x12 blocks, sRGB
#endif
} ksGpuTextureFormat;

typedef enum {
    KS_GPU_TEXTURE_USAGE_UNDEFINED = BIT(0),
    KS_GPU_TEXTURE_USAGE_GENERAL = BIT(1),
    KS_GPU_TEXTURE_USAGE_TRANSFER_SRC = BIT(2),
    KS_GPU_TEXTURE_USAGE_TRANSFER_DST = BIT(3),
    KS_GPU_TEXTURE_USAGE_SAMPLED = BIT(4),
    KS_GPU_TEXTURE_USAGE_STORAGE = BIT(5),
    KS_GPU_TEXTURE_USAGE_COLOR_ATTACHMENT = BIT(6),
    KS_GPU_TEXTURE_USAGE_PRESENTATION = BIT(7)
} ksGpuTextureUsage;

typedef unsigned int ksGpuTextureUsageFlags;

typedef enum {
    KS_GPU_TEXTURE_WRAP_MODE_REPEAT,
    KS_GPU_TEXTURE_WRAP_MODE_CLAMP_TO_EDGE,
    KS_GPU_TEXTURE_WRAP_MODE_CLAMP_TO_BORDER
} ksGpuTextureWrapMode;

typedef enum { KS_GPU_TEXTURE_FILTER_NEAREST, KS_GPU_TEXTURE_FILTER_LINEAR, KS_GPU_TEXTURE_FILTER_BILINEAR } ksGpuTextureFilter;

typedef enum {
    KS_GPU_TEXTURE_DEFAULT_CHECKERBOARD,  // 32x32 checkerboard pattern (KS_GPU_TEXTURE_FORMAT_R8G8B8A8_UNORM)
    KS_GPU_TEXTURE_DEFAULT_PYRAMIDS,      // 32x32 block pattern of pyramids (KS_GPU_TEXTURE_FORMAT_R8G8B8A8_UNORM)
    KS_GPU_TEXTURE_DEFAULT_CIRCLES        // 32x32 block pattern with circles (KS_GPU_TEXTURE_FORMAT_R8G8B8A8_UNORM)
} ksGpuTextureDefault;

typedef struct {
    int width;
    int height;
    int depth;
    int layerCount;
    int mipCount;
    ksGpuSampleCount sampleCount;
    ksGpuTextureUsage usage;
    ksGpuTextureUsageFlags usageFlags;
    ksGpuTextureWrapMode wrapMode;
    ksGpuTextureFilter filter;
    float maxAnisotropy;
    GLenum format;
    GLuint target;
    GLuint texture;
} ksGpuTexture;

bool ksGpuTexture_Create2D(ksGpuContext *context, ksGpuTexture *texture, const ksGpuTextureFormat format,
                           const ksGpuSampleCount sampleCount, const int width, const int height, const int mipCount,
                           const ksGpuTextureUsageFlags usageFlags, const void *data, const size_t dataSize);
bool ksGpuTexture_Create2DArray(ksGpuContext *context, ksGpuTexture *texture, const ksGpuTextureFormat format,
                                const ksGpuSampleCount sampleCount, const int width, const int height, const int layerCount,
                                const int mipCount, const ksGpuTextureUsageFlags usageFlags, const void *data,
                                const size_t dataSize);
bool ksGpuTexture_CreateDefault(ksGpuContext *context, ksGpuTexture *texture, const ksGpuTextureDefault defaultType,
                                const int width, const int height, const int depth, const int layerCount, const int faceCount,
                                const bool mipmaps, const bool border);
bool ksGpuTexture_CreateFromSwapchain(ksGpuContext *context, ksGpuTexture *texture, const ksGpuWindow *window, int index);
bool ksGpuTexture_CreateFromFile(ksGpuContext *context, ksGpuTexture *texture, const char *fileName);
void ksGpuTexture_Destroy(ksGpuContext *context, ksGpuTexture *texture);

void ksGpuTexture_SetFilter(ksGpuContext *context, ksGpuTexture *texture, const ksGpuTextureFilter filter);
void ksGpuTexture_SetAniso(ksGpuContext *context, ksGpuTexture *texture, const float maxAniso);
void ksGpuTexture_SetWrapMode(ksGpuContext *context, ksGpuTexture *texture, const ksGpuTextureWrapMode wrapMode);

/*
================================================================================================================================

GPU indices and vertex attributes.

ksGpuTriangleIndex
ksGpuTriangleIndexArray
ksGpuVertexAttribute
ksGpuVertexAttributeArrays

================================================================================================================================
*/

typedef unsigned short ksGpuTriangleIndex;

typedef struct {
    const ksGpuBuffer *buffer;
    ksGpuTriangleIndex *indexArray;
    int indexCount;
} ksGpuTriangleIndexArray;

typedef enum {
    KS_GPU_ATTRIBUTE_FORMAT_R32_SFLOAT = (1 << 16) | GL_FLOAT,
    KS_GPU_ATTRIBUTE_FORMAT_R32G32_SFLOAT = (2 << 16) | GL_FLOAT,
    KS_GPU_ATTRIBUTE_FORMAT_R32G32B32_SFLOAT = (3 << 16) | GL_FLOAT,
    KS_GPU_ATTRIBUTE_FORMAT_R32G32B32A32_SFLOAT = (4 << 16) | GL_FLOAT
} ksGpuAttributeFormat;

typedef struct {
    int attributeFlag;                     // VERTEX_ATTRIBUTE_FLAG_
    size_t attributeOffset;                // Offset in bytes to the pointer in ksGpuVertexAttributeArrays
    size_t attributeSize;                  // Size in bytes of a single attribute
    ksGpuAttributeFormat attributeFormat;  // Format of the attribute
    int locationCount;                     // Number of attribute locations
    const char *name;                      // Name in vertex program
} ksGpuVertexAttribute;

typedef struct {
    const ksGpuBuffer *buffer;
    const ksGpuVertexAttribute *layout;
    void *data;
    size_t dataSize;
    int vertexCount;
    int attribsFlags;
} ksGpuVertexAttributeArrays;

/*
================================================================================================================================

GPU default vertex attribute layout.

ksDefaultVertexAttributeFlags
ksDefaultVertexAttributeArrays

================================================================================================================================
*/

typedef enum {
    VERTEX_ATTRIBUTE_FLAG_POSITION = BIT(0),       // vec3 vertexPosition
    VERTEX_ATTRIBUTE_FLAG_NORMAL = BIT(1),         // vec3 vertexNormal
    VERTEX_ATTRIBUTE_FLAG_TANGENT = BIT(2),        // vec3 vertexTangent
    VERTEX_ATTRIBUTE_FLAG_BINORMAL = BIT(3),       // vec3 vertexBinormal
    VERTEX_ATTRIBUTE_FLAG_COLOR = BIT(4),          // vec4 vertexColor
    VERTEX_ATTRIBUTE_FLAG_UV0 = BIT(5),            // vec2 vertexUv0
    VERTEX_ATTRIBUTE_FLAG_UV1 = BIT(6),            // vec2 vertexUv1
    VERTEX_ATTRIBUTE_FLAG_UV2 = BIT(7),            // vec2 vertexUv2
    VERTEX_ATTRIBUTE_FLAG_JOINT_INDICES = BIT(8),  // vec4 jointIndices
    VERTEX_ATTRIBUTE_FLAG_JOINT_WEIGHTS = BIT(9),  // vec4 jointWeights
    VERTEX_ATTRIBUTE_FLAG_TRANSFORM = BIT(10)      // mat4 vertexTransform (NOTE this mat4 takes up 4 attribute locations)
} ksDefaultVertexAttributeFlags;

typedef struct {
    ksGpuVertexAttributeArrays base;
    ksVector3f *position;
    ksVector3f *normal;
    ksVector3f *tangent;
    ksVector3f *binormal;
    ksVector4f *color;
    ksVector2f *uv0;
    ksVector2f *uv1;
    ksVector2f *uv2;
    ksVector4f *jointIndices;
    ksVector4f *jointWeights;
    ksMatrix4x4f *transform;
} ksDefaultVertexAttributeArrays;

static const ksGpuVertexAttribute ksDefaultVertexAttributeLayout[] = {
    {VERTEX_ATTRIBUTE_FLAG_POSITION, OFFSETOF_MEMBER(ksDefaultVertexAttributeArrays, position),
     SIZEOF_MEMBER(ksDefaultVertexAttributeArrays, position[0]), KS_GPU_ATTRIBUTE_FORMAT_R32G32B32_SFLOAT, 1, "vertexPosition"},
    {VERTEX_ATTRIBUTE_FLAG_NORMAL, OFFSETOF_MEMBER(ksDefaultVertexAttributeArrays, normal),
     SIZEOF_MEMBER(ksDefaultVertexAttributeArrays, normal[0]), KS_GPU_ATTRIBUTE_FORMAT_R32G32B32_SFLOAT, 1, "vertexNormal"},
    {VERTEX_ATTRIBUTE_FLAG_TANGENT, OFFSETOF_MEMBER(ksDefaultVertexAttributeArrays, tangent),
     SIZEOF_MEMBER(ksDefaultVertexAttributeArrays, tangent[0]), KS_GPU_ATTRIBUTE_FORMAT_R32G32B32_SFLOAT, 1, "vertexTangent"},
    {VERTEX_ATTRIBUTE_FLAG_BINORMAL, OFFSETOF_MEMBER(ksDefaultVertexAttributeArrays, binormal),
     SIZEOF_MEMBER(ksDefaultVertexAttributeArrays, binormal[0]), KS_GPU_ATTRIBUTE_FORMAT_R32G32B32_SFLOAT, 1, "vertexBinormal"},
    {VERTEX_ATTRIBUTE_FLAG_COLOR, OFFSETOF_MEMBER(ksDefaultVertexAttributeArrays, color),
     SIZEOF_MEMBER(ksDefaultVertexAttributeArrays, color[0]), KS_GPU_ATTRIBUTE_FORMAT_R32G32B32A32_SFLOAT, 1, "vertexColor"},
    {VERTEX_ATTRIBUTE_FLAG_UV0, OFFSETOF_MEMBER(ksDefaultVertexAttributeArrays, uv0),
     SIZEOF_MEMBER(ksDefaultVertexAttributeArrays, uv0[0]), KS_GPU_ATTRIBUTE_FORMAT_R32G32_SFLOAT, 1, "vertexUv0"},
    {VERTEX_ATTRIBUTE_FLAG_UV1, OFFSETOF_MEMBER(ksDefaultVertexAttributeArrays, uv1),
     SIZEOF_MEMBER(ksDefaultVertexAttributeArrays, uv1[0]), KS_GPU_ATTRIBUTE_FORMAT_R32G32_SFLOAT, 1, "vertexUv1"},
    {VERTEX_ATTRIBUTE_FLAG_UV2, OFFSETOF_MEMBER(ksDefaultVertexAttributeArrays, uv2),
     SIZEOF_MEMBER(ksDefaultVertexAttributeArrays, uv2[0]), KS_GPU_ATTRIBUTE_FORMAT_R32G32_SFLOAT, 1, "vertexUv2"},
    {VERTEX_ATTRIBUTE_FLAG_JOINT_INDICES, OFFSETOF_MEMBER(ksDefaultVertexAttributeArrays, jointIndices),
     SIZEOF_MEMBER(ksDefaultVertexAttributeArrays, jointIndices[0]), KS_GPU_ATTRIBUTE_FORMAT_R32G32B32A32_SFLOAT, 1,
     "vertexJointIndices"},
    {VERTEX_ATTRIBUTE_FLAG_JOINT_WEIGHTS, OFFSETOF_MEMBER(ksDefaultVertexAttributeArrays, jointWeights),
     SIZEOF_MEMBER(ksDefaultVertexAttributeArrays, jointWeights[0]), KS_GPU_ATTRIBUTE_FORMAT_R32G32B32A32_SFLOAT, 1,
     "vertexJointWeights"},
    {VERTEX_ATTRIBUTE_FLAG_TRANSFORM, OFFSETOF_MEMBER(ksDefaultVertexAttributeArrays, transform),
     SIZEOF_MEMBER(ksDefaultVertexAttributeArrays, transform[0]), KS_GPU_ATTRIBUTE_FORMAT_R32G32B32A32_SFLOAT, 4,
     "vertexTransform"},
    {0, 0, 0, 0, 0, ""}};

/*
================================================================================================================================

GPU geometry.

For optimal performance geometry should only be created at load time, not at runtime.
The vertex attributes are not packed. Each attribute is stored in a separate array for
optimal binning on tiling GPUs that only transform the vertex position for the binning pass.
Storing each attribute in a saparate array is preferred even on immediate-mode GPUs to avoid
wasting cache space for attributes that are not used by a particular vertex shader.

ksGpuGeometry

void ksGpuGeometry_Create( ksGpuContext * context, ksGpuGeometry * geometry,
                                                                const ksGpuVertexAttributeArrays * attribs,
                                                                const ksGpuTriangleIndexArray * indices );
void ksGpuGeometry_CreateQuad( ksGpuContext * context, ksGpuGeometry * geometry, const float offset, const float scale );
void ksGpuGeometry_CreateCube( ksGpuContext * context, ksGpuGeometry * geometry, const float offset, const float scale );
void ksGpuGeometry_CreateTorus( ksGpuContext * context, ksGpuGeometry * geometry, const int tesselation, const float offset,
const float scale );

void ksGpuGeometry_Destroy( ksGpuContext * context, ksGpuGeometry * geometry );

void ksGpuGeometry_AddInstanceAttributes( ksGpuContext * context, ksGpuGeometry * geometry, const int numInstances, const int
instanceAttribsFlags );

================================================================================================================================
*/

typedef struct {
    const ksGpuVertexAttribute *layout;
    int vertexAttribsFlags;
    int instanceAttribsFlags;
    int vertexCount;
    int instanceCount;
    int indexCount;
    ksGpuBuffer vertexBuffer;
    ksGpuBuffer instanceBuffer;
    ksGpuBuffer indexBuffer;
} ksGpuGeometry;

void ksGpuGeometry_Create(ksGpuContext *context, ksGpuGeometry *geometry, const ksGpuVertexAttributeArrays *attribs,
                          const ksGpuTriangleIndexArray *indices);
void ksGpuGeometry_CreateQuad(ksGpuContext *context, ksGpuGeometry *geometry, const float offset, const float scale);
void ksGpuGeometry_CreateCube(ksGpuContext *context, ksGpuGeometry *geometry, const float offset, const float scale);
void ksGpuGeometry_CreateTorus(ksGpuContext *context, ksGpuGeometry *geometry, const int tesselation, const float offset,
                               const float scale);

void ksGpuGeometry_Destroy(ksGpuContext *context, ksGpuGeometry *geometry);

void ksGpuGeometry_AddInstanceAttributes(ksGpuContext *context, ksGpuGeometry *geometry, const int numInstances,
                                         const int instanceAttribsFlags);

/*
================================================================================================================================

GPU render pass.

A render pass encapsulates a sequence of graphics commands that can be executed in a single tiling pass.
For optimal performance a render pass should only be created at load time, not at runtime.
Render passes cannot overlap and cannot be nested.

ksGpuRenderPassType
ksGpuRenderPassFlags
ksGpuRenderPass

static bool ksGpuRenderPass_Create( ksGpuContext * context, ksGpuRenderPass * renderPass,
                                                                        const ksGpuSurfaceColorFormat colorFormat, const
ksGpuSurfaceDepthFormat depthFormat, const ksGpuSampleCount sampleCount, const ksGpuRenderPassType type, const uint32_t flags );
static void ksGpuRenderPass_Destroy( ksGpuContext * context, ksGpuRenderPass * renderPass );

================================================================================================================================
*/

typedef enum { KS_GPU_RENDERPASS_TYPE_INLINE, KS_GPU_RENDERPASS_TYPE_SECONDARY_COMMAND_BUFFERS } ksGpuRenderPassType;

typedef enum {
    KS_GPU_RENDERPASS_FLAG_CLEAR_COLOR_BUFFER = BIT(0),
    KS_GPU_RENDERPASS_FLAG_CLEAR_DEPTH_BUFFER = BIT(1)
} ksGpuRenderPassFlags;

typedef struct {
    ksGpuRenderPassType type;
    int flags;
    ksGpuSurfaceColorFormat colorFormat;
    ksGpuSurfaceDepthFormat depthFormat;
    ksGpuSampleCount sampleCount;
} ksGpuRenderPass;

bool ksGpuRenderPass_Create(ksGpuContext *context, ksGpuRenderPass *renderPass, const ksGpuSurfaceColorFormat colorFormat,
                            const ksGpuSurfaceDepthFormat depthFormat, const ksGpuSampleCount sampleCount,
                            const ksGpuRenderPassType type, const int flags);

void ksGpuRenderPass_Destroy(ksGpuContext *context, ksGpuRenderPass *renderPass);

/*
================================================================================================================================

GPU framebuffer.

A framebuffer encapsulates either a swapchain or a buffered set of textures.
For optimal performance a framebuffer should only be created at load time, not at runtime.

ksGpuFramebuffer

bool ksGpuFramebuffer_CreateFromSwapchain(ksGpuWindow *window, ksGpuFramebuffer *framebuffer, ksGpuRenderPass *renderPass);

bool ksGpuFramebuffer_CreateFromTextures(ksGpuContext *context, ksGpuFramebuffer *framebuffer, ksGpuRenderPass *renderPass,
                                         const int width, const int height, const int numBuffers);

bool ksGpuFramebuffer_CreateFromTextureArrays(ksGpuContext *context, ksGpuFramebuffer *framebuffer, ksGpuRenderPass *renderPass,
                                              const int width, const int height, const int numLayers, const int numBuffers,
                                              const bool multiview);

void ksGpuFramebuffer_Destroy(ksGpuContext *context, ksGpuFramebuffer *framebuffer);

int ksGpuFramebuffer_GetWidth(const ksGpuFramebuffer *framebuffer);
int ksGpuFramebuffer_GetHeight(const ksGpuFramebuffer *framebuffer);
ksScreenRect ksGpuFramebuffer_GetRect(const ksGpuFramebuffer *framebuffer);
int ksGpuFramebuffer_GetBufferCount(const ksGpuFramebuffer *framebuffer);
ksGpuTexture *ksGpuFramebuffer_GetColorTexture(const ksGpuFramebuffer *framebuffer);

================================================================================================================================
*/

typedef struct {
    ksGpuTexture *colorTextures;
    GLuint renderTexture;
    GLuint depthBuffer;
    GLuint *renderBuffers;
    GLuint *resolveBuffers;
    bool multiView;
    int sampleCount;
    int numFramebuffersPerTexture;
    int numBuffers;
    int currentBuffer;
} ksGpuFramebuffer;

typedef enum { MSAA_OFF, MSAA_RESOLVE, MSAA_BLIT } ksGpuMsaaMode;

bool ksGpuFramebuffer_CreateFromSwapchain(ksGpuWindow *window, ksGpuFramebuffer *framebuffer, ksGpuRenderPass *renderPass);

bool ksGpuFramebuffer_CreateFromTextures(ksGpuContext *context, ksGpuFramebuffer *framebuffer, ksGpuRenderPass *renderPass,
                                         const int width, const int height, const int numBuffers);

bool ksGpuFramebuffer_CreateFromTextureArrays(ksGpuContext *context, ksGpuFramebuffer *framebuffer, ksGpuRenderPass *renderPass,
                                              const int width, const int height, const int numLayers, const int numBuffers,
                                              const bool multiview);

void ksGpuFramebuffer_Destroy(ksGpuContext *context, ksGpuFramebuffer *framebuffer);

int ksGpuFramebuffer_GetWidth(const ksGpuFramebuffer *framebuffer);
int ksGpuFramebuffer_GetHeight(const ksGpuFramebuffer *framebuffer);
ksScreenRect ksGpuFramebuffer_GetRect(const ksGpuFramebuffer *framebuffer);
int ksGpuFramebuffer_GetBufferCount(const ksGpuFramebuffer *framebuffer);
ksGpuTexture *ksGpuFramebuffer_GetColorTexture(const ksGpuFramebuffer *framebuffer);

/*
================================================================================================================================

GPU program parms and layout.

ksGpuProgramStageFlags
ksGpuProgramParmType
ksGpuProgramParmAccess
ksGpuProgramParm
ksGpuProgramParmLayout

static void ksGpuProgramParmLayout_Create( ksGpuContext * context, ksGpuProgramParmLayout * layout,
                                                                                const ksGpuProgramParm * parms, const int numParms,
                                                                                const GLuint program );
static void ksGpuProgramParmLayout_Destroy( ksGpuContext * context, ksGpuProgramParmLayout * layout );

================================================================================================================================
*/

#define KS_MAX_PROGRAM_PARMS 16

typedef enum {
    KS_GPU_PROGRAM_STAGE_FLAG_VERTEX = BIT(0),
    KS_GPU_PROGRAM_STAGE_FLAG_FRAGMENT = BIT(1),
    KS_GPU_PROGRAM_STAGE_FLAG_COMPUTE = BIT(2),
    KS_GPU_PROGRAM_STAGE_MAX = 3
} ksGpuProgramStageFlags;

typedef enum {
    KS_GPU_PROGRAM_PARM_TYPE_TEXTURE_SAMPLED,    // texture plus sampler bound together        (GLSL: sampler*, isampler*,
                                                 // usampler*)
    KS_GPU_PROGRAM_PARM_TYPE_TEXTURE_STORAGE,    // not sampled, direct read-write storage    (GLSL: image*, iimage*, uimage*)
    KS_GPU_PROGRAM_PARM_TYPE_BUFFER_UNIFORM,     // read-only uniform buffer                    (GLSL: uniform)
    KS_GPU_PROGRAM_PARM_TYPE_BUFFER_STORAGE,     // read-write storage buffer                (GLSL: buffer)
    KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_INT,  // int                                        (GLSL:
                                                 // int)
    KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_INT_VECTOR2,      // int[2]                                    (GLSL:
                                                             // ivec2)
    KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_INT_VECTOR3,      // int[3]                                    (GLSL:
                                                             // ivec3)
    KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_INT_VECTOR4,      // int[4]                                    (GLSL:
                                                             // ivec4)
    KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT,            // float                                    (GLSL:
                                                             // float)
    KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_VECTOR2,    // float[2]                                    (GLSL:
                                                             // vec2)
    KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_VECTOR3,    // float[3]                                    (GLSL:
                                                             // vec3)
    KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_VECTOR4,    // float[4]                                    (GLSL:
                                                             // vec4)
    KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_MATRIX2X2,  // float[2][2]
                                                             // (GLSL: mat2x2 or mat2)
    KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_MATRIX2X3,  // float[2][3]
                                                             // (GLSL: mat2x3)
    KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_MATRIX2X4,  // float[2][4]
                                                             // (GLSL: mat2x4)
    KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_MATRIX3X2,  // float[3][2]
                                                             // (GLSL: mat3x2)
    KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_MATRIX3X3,  // float[3][3]
                                                             // (GLSL: mat3x3 or mat3)
    KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_MATRIX3X4,  // float[3][4]
                                                             // (GLSL: mat3x4)
    KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_MATRIX4X2,  // float[4][2]
                                                             // (GLSL: mat4x2)
    KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_MATRIX4X3,  // float[4][3]
                                                             // (GLSL: mat4x3)
    KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_MATRIX4X4,  // float[4][4]
                                                             // (GLSL: mat4x4 or mat4)
    KS_GPU_PROGRAM_PARM_TYPE_MAX
} ksGpuProgramParmType;

typedef enum {
    KS_GPU_PROGRAM_PARM_ACCESS_READ_ONLY,
    KS_GPU_PROGRAM_PARM_ACCESS_WRITE_ONLY,
    KS_GPU_PROGRAM_PARM_ACCESS_READ_WRITE
} ksGpuProgramParmAccess;

typedef struct {
    int stageFlags;                 // vertex, fragment and/or compute
    ksGpuProgramParmType type;      // texture, buffer or push constant
    ksGpuProgramParmAccess access;  // read and/or write
    int index;                      // index into ksGpuProgramParmState::parms
    const char *name;               // GLSL name
    int binding;                    // OpenGL shader bind points:
                                    // - texture image unit
                                    // - image unit
                                    // - uniform buffer
                                    // - storage buffer
                                    // - uniform
    // Note that each bind point uses its own range of binding indices with each range starting at zero.
    // However, each range is unique across all stages of a pipeline.
    // Note that even though multiple targets can be bound to the same texture image unit,
    // the OpenGL spec disallows rendering from multiple targets using a single texture image unit.

} ksGpuProgramParm;

typedef struct {
    int numParms;
    const ksGpuProgramParm *parms;
    int offsetForIndex[KS_MAX_PROGRAM_PARMS];   // push constant offsets into ksGpuProgramParmState::data based on
                                                // ksGpuProgramParm::index
    GLint parmLocations[KS_MAX_PROGRAM_PARMS];  // OpenGL locations
    GLint parmBindings[KS_MAX_PROGRAM_PARMS];
    GLint numSampledTextureBindings;
    GLint numStorageTextureBindings;
    GLint numUniformBufferBindings;
    GLint numStorageBufferBindings;
} ksGpuProgramParmLayout;

/*
================================================================================================================================

GPU graphics program.

A graphics program encapsulates a vertex and fragment program that are used to render geometry.
For optimal performance a graphics program should only be created at load time, not at runtime.

ksGpuGraphicsProgram

bool ksGpuGraphicsProgram_Create(ksGpuContext *context, ksGpuGraphicsProgram *program, const void *vertexSourceData,
                                 const size_t vertexSourceSize, const void *fragmentSourceData, const size_t fragmentSourceSize,
                                 const ksGpuProgramParm *parms, const int numParms, const ksGpuVertexAttribute *vertexLayout,
                                 const int vertexAttribsFlags);

void ksGpuGraphicsProgram_Destroy(ksGpuContext *context, ksGpuGraphicsProgram *program);

================================================================================================================================
*/

typedef struct {
    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint program;
    ksGpuProgramParmLayout parmLayout;
    int vertexAttribsFlags;
    ksStringHash hash;
} ksGpuGraphicsProgram;

bool ksGpuGraphicsProgram_Create(ksGpuContext *context, ksGpuGraphicsProgram *program, const void *vertexSourceData,
                                 const size_t vertexSourceSize, const void *fragmentSourceData, const size_t fragmentSourceSize,
                                 const ksGpuProgramParm *parms, const int numParms, const ksGpuVertexAttribute *vertexLayout,
                                 const int vertexAttribsFlags);
void ksGpuGraphicsProgram_Destroy(ksGpuContext *context, ksGpuGraphicsProgram *program);

/*
================================================================================================================================

GPU compute program.

For optimal performance a compute program should only be created at load time, not at runtime.

ksGpuComputeProgram

bool ksGpuComputeProgram_Create(ksGpuContext *context, ksGpuComputeProgram *program, const void *computeSourceData,
                                       const size_t computeSourceSize, const ksGpuProgramParm *parms, const int numParms);

void ksGpuComputeProgram_Destroy(ksGpuContext *context, ksGpuComputeProgram *program);

================================================================================================================================
*/

typedef struct {
    GLuint computeShader;
    GLuint program;
    ksGpuProgramParmLayout parmLayout;
    ksStringHash hash;
} ksGpuComputeProgram;

bool ksGpuComputeProgram_Create(ksGpuContext *context, ksGpuComputeProgram *program, const void *computeSourceData,
                                const size_t computeSourceSize, const ksGpuProgramParm *parms, const int numParms);
void ksGpuComputeProgram_Destroy(ksGpuContext *context, ksGpuComputeProgram *program);

/*
================================================================================================================================

GPU graphics pipeline.

A graphics pipeline encapsulates the geometry, program and ROP state that is used to render.
For optimal performance a graphics pipeline should only be created at load time, not at runtime.
Due to the use of a Vertex Array Object (VAO), a graphics pipeline must be created using the same
context that is used to render with the graphics pipeline. The VAO is created here, when both the
geometry and the program are known, to avoid binding vertex attributes that are not used by the
vertex shader, and to avoid binding to a discontinuous set of vertex attribute locations.

ksGpuFrontFace
ksGpuCullMode
ksGpuCompareOp
ksGpuBlendOp
ksGpuBlendFactor
ksGpuRasterOperations
ksGpuGraphicsPipelineParms
ksGpuGraphicsPipeline


void ksGpuGraphicsPipelineParms_Init(ksGpuGraphicsPipelineParms *parms);
bool ksGpuGraphicsPipeline_Create(ksGpuContext *context, ksGpuGraphicsPipeline *pipeline, const ksGpuGraphicsPipelineParms *parms);
void ksGpuGraphicsPipeline_Destroy(ksGpuContext *context, ksGpuGraphicsPipeline *pipeline);

================================================================================================================================
*/

typedef enum { KS_GPU_FRONT_FACE_COUNTER_CLOCKWISE = GL_CCW, KS_GPU_FRONT_FACE_CLOCKWISE = GL_CW } ksGpuFrontFace;

typedef enum { KS_GPU_CULL_MODE_NONE = GL_NONE, KS_GPU_CULL_MODE_FRONT = GL_FRONT, KS_GPU_CULL_MODE_BACK = GL_BACK } ksGpuCullMode;

typedef enum {
    KS_GPU_COMPARE_OP_NEVER = GL_NEVER,
    KS_GPU_COMPARE_OP_LESS = GL_LESS,
    KS_GPU_COMPARE_OP_EQUAL = GL_EQUAL,
    KS_GPU_COMPARE_OP_LESS_OR_EQUAL = GL_LEQUAL,
    KS_GPU_COMPARE_OP_GREATER = GL_GREATER,
    KS_GPU_COMPARE_OP_NOT_EQUAL = GL_NOTEQUAL,
    KS_GPU_COMPARE_OP_GREATER_OR_EQUAL = GL_GEQUAL,
    KS_GPU_COMPARE_OP_ALWAYS = GL_ALWAYS
} ksGpuCompareOp;

typedef enum {
    KS_GPU_BLEND_OP_ADD = GL_FUNC_ADD,
    KS_GPU_BLEND_OP_SUBTRACT = GL_FUNC_SUBTRACT,
    KS_GPU_BLEND_OP_REVERSE_SUBTRACT = GL_FUNC_REVERSE_SUBTRACT,
    KS_GPU_BLEND_OP_MIN = GL_MIN,
    KS_GPU_BLEND_OP_MAX = GL_MAX
} ksGpuBlendOp;

typedef enum {
    KS_GPU_BLEND_FACTOR_ZERO = GL_ZERO,
    KS_GPU_BLEND_FACTOR_ONE = GL_ONE,
    KS_GPU_BLEND_FACTOR_SRC_COLOR = GL_SRC_COLOR,
    KS_GPU_BLEND_FACTOR_ONE_MINUS_SRC_COLOR = GL_ONE_MINUS_SRC_COLOR,
    KS_GPU_BLEND_FACTOR_DST_COLOR = GL_DST_COLOR,
    KS_GPU_BLEND_FACTOR_ONE_MINUS_DST_COLOR = GL_ONE_MINUS_DST_COLOR,
    KS_GPU_BLEND_FACTOR_SRC_ALPHA = GL_SRC_ALPHA,
    KS_GPU_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA = GL_ONE_MINUS_SRC_ALPHA,
    KS_GPU_BLEND_FACTOR_DST_ALPHA = GL_DST_ALPHA,
    KS_GPU_BLEND_FACTOR_ONE_MINUS_DST_ALPHA = GL_ONE_MINUS_DST_ALPHA,
    KS_GPU_BLEND_FACTOR_CONSTANT_COLOR = GL_CONSTANT_COLOR,
    KS_GPU_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR = GL_ONE_MINUS_CONSTANT_COLOR,
    KS_GPU_BLEND_FACTOR_CONSTANT_ALPHA = GL_CONSTANT_ALPHA,
    KS_GPU_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA = GL_ONE_MINUS_CONSTANT_ALPHA,
    KS_GPU_BLEND_FACTOR_SRC_ALPHA_SATURATE = GL_SRC_ALPHA_SATURATE
} ksGpuBlendFactor;

typedef struct {
    bool blendEnable;
    bool redWriteEnable;
    bool blueWriteEnable;
    bool greenWriteEnable;
    bool alphaWriteEnable;
    bool depthTestEnable;
    bool depthWriteEnable;
    ksGpuFrontFace frontFace;
    ksGpuCullMode cullMode;
    ksGpuCompareOp depthCompare;
    ksVector4f blendColor;
    ksGpuBlendOp blendOpColor;
    ksGpuBlendFactor blendSrcColor;
    ksGpuBlendFactor blendDstColor;
    ksGpuBlendOp blendOpAlpha;
    ksGpuBlendFactor blendSrcAlpha;
    ksGpuBlendFactor blendDstAlpha;
} ksGpuRasterOperations;

typedef struct {
    ksGpuRasterOperations rop;
    const ksGpuRenderPass *renderPass;
    const ksGpuGraphicsProgram *program;
    const ksGpuGeometry *geometry;
} ksGpuGraphicsPipelineParms;

typedef struct {
    ksGpuRasterOperations rop;
    const ksGpuGraphicsProgram *program;
    const ksGpuGeometry *geometry;
    GLuint vertexArrayObject;
} ksGpuGraphicsPipeline;

void ksGpuGraphicsPipelineParms_Init(ksGpuGraphicsPipelineParms *parms);
bool ksGpuGraphicsPipeline_Create(ksGpuContext *context, ksGpuGraphicsPipeline *pipeline, const ksGpuGraphicsPipelineParms *parms);
void ksGpuGraphicsPipeline_Destroy(ksGpuContext *context, ksGpuGraphicsPipeline *pipeline);

/*
================================================================================================================================

GPU compute pipeline.

A compute pipeline encapsulates a compute program.
For optimal performance a compute pipeline should only be created at load time, not at runtime.

ksGpuComputePipeline

bool ksGpuComputePipeline_Create(ksGpuContext *context, ksGpuComputePipeline *pipeline, const ksGpuComputeProgram *program);
void ksGpuComputePipeline_Destroy(ksGpuContext *context, ksGpuComputePipeline *pipeline);

================================================================================================================================
*/

typedef struct {
    const ksGpuComputeProgram *program;
} ksGpuComputePipeline;

bool ksGpuComputePipeline_Create(ksGpuContext *context, ksGpuComputePipeline *pipeline, const ksGpuComputeProgram *program);
void ksGpuComputePipeline_Destroy(ksGpuContext *context, ksGpuComputePipeline *pipeline);

/*
================================================================================================================================

GPU fence.

A fence is used to notify completion of a command buffer.
For optimal performance a fence should only be created at load time, not at runtime.

ksGpuFence

void ksGpuFence_Create( ksGpuContext * context, ksGpuFence * fence );
void ksGpuFence_Destroy( ksGpuContext * context, ksGpuFence * fence );
void ksGpuFence_Submit( ksGpuContext * context, ksGpuFence * fence );
void ksGpuFence_IsSignalled( ksGpuContext * context, ksGpuFence * fence );

================================================================================================================================
*/

typedef struct {
#if USE_SYNC_OBJECT == 0
    GLsync sync;
#elif USE_SYNC_OBJECT == 1
    EGLDisplay display;
    EGLSyncKHR sync;
#elif USE_SYNC_OBJECT == 2
    GLuint computeShader;
    GLuint computeProgram;
    GLuint storageBuffer;
    GLuint *mappedBuffer;
#else
#error "invalid USE_SYNC_OBJECT setting"
#endif
} ksGpuFence;

void ksGpuFence_Create(ksGpuContext *context, ksGpuFence *fence);
void ksGpuFence_Destroy(ksGpuContext *context, ksGpuFence *fence);
void ksGpuFence_Submit(ksGpuContext *context, ksGpuFence *fence);
void ksGpuFence_IsSignalled(ksGpuContext *context, ksGpuFence *fence);

/*
================================================================================================================================

GPU program parm state.

ksGpuProgramParmState

================================================================================================================================
*/

#define SAVE_PUSH_CONSTANT_STATE 1
#define MAX_SAVED_PUSH_CONSTANT_BYTES 512

typedef struct {
    const void *parms[KS_MAX_PROGRAM_PARMS];
#if SAVE_PUSH_CONSTANT_STATE == 1
    unsigned char data[MAX_SAVED_PUSH_CONSTANT_BYTES];
#endif
} ksGpuProgramParmState;

void ksGpuProgramParmState_SetParm(ksGpuProgramParmState *parmState, const ksGpuProgramParmLayout *parmLayout, const int index,
                                   const ksGpuProgramParmType parmType, const void *pointer);

const void *ksGpuProgramParmState_NewPushConstantData(const ksGpuProgramParmLayout *newLayout, const int newParmIndex,
                                                      const ksGpuProgramParmState *newParmState,
                                                      const ksGpuProgramParmLayout *oldLayout, const int oldParmIndex,
                                                      const ksGpuProgramParmState *oldParmState, const bool force);

/*
================================================================================================================================

GPU graphics commands.

A graphics command encapsulates all GPU state associated with a single draw call.
The pointers passed in as parameters are expected to point to unique objects that persist
at least past the submission of the command buffer into which the graphics command is
submitted. Because pointers are maintained as state, DO NOT use pointers to local
variables that will go out of scope before the command buffer is submitted.

ksGpuGraphicsCommand

void ksGpuGraphicsCommand_Init(ksGpuGraphicsCommand *command);
void ksGpuGraphicsCommand_SetPipeline(ksGpuGraphicsCommand *command, const ksGpuGraphicsPipeline *pipeline);
void ksGpuGraphicsCommand_SetVertexBuffer(ksGpuGraphicsCommand *command, const ksGpuBuffer *vertexBuffer);
void ksGpuGraphicsCommand_SetInstanceBuffer(ksGpuGraphicsCommand *command, const ksGpuBuffer *instanceBuffer);
void ksGpuGraphicsCommand_SetParmTextureSampled(ksGpuGraphicsCommand *command, const int index, const ksGpuTexture *texture);
void ksGpuGraphicsCommand_SetParmTextureStorage(ksGpuGraphicsCommand *command, const int index, const ksGpuTexture *texture);
void ksGpuGraphicsCommand_SetParmBufferUniform(ksGpuGraphicsCommand *command, const int index, const ksGpuBuffer *buffer);
void ksGpuGraphicsCommand_SetParmBufferStorage(ksGpuGraphicsCommand *command, const int index, const ksGpuBuffer *buffer);
void ksGpuGraphicsCommand_SetParmInt(ksGpuGraphicsCommand *command, const int index, const int *value);
void ksGpuGraphicsCommand_SetParmIntVector2(ksGpuGraphicsCommand *command, const int index, const ksVector2i *value);
void ksGpuGraphicsCommand_SetParmIntVector3(ksGpuGraphicsCommand *command, const int index, const ksVector3i *value);
void ksGpuGraphicsCommand_SetParmIntVector4(ksGpuGraphicsCommand *command, const int index, const ksVector4i *value);
void ksGpuGraphicsCommand_SetParmFloat(ksGpuGraphicsCommand *command, const int index, const float *value);
void ksGpuGraphicsCommand_SetParmFloatVector2(ksGpuGraphicsCommand *command, const int index, const ksVector2f *value);
void ksGpuGraphicsCommand_SetParmFloatVector3(ksGpuGraphicsCommand *command, const int index, const ksVector3f *value);
void ksGpuGraphicsCommand_SetParmFloatVector4(ksGpuGraphicsCommand *command, const int index, const ksVector3f *value);
void ksGpuGraphicsCommand_SetParmFloatMatrix2x2(ksGpuGraphicsCommand *command, const int index, const ksMatrix2x2f *value);
void ksGpuGraphicsCommand_SetParmFloatMatrix2x3(ksGpuGraphicsCommand *command, const int index, const ksMatrix2x3f *value);
void ksGpuGraphicsCommand_SetParmFloatMatrix2x4(ksGpuGraphicsCommand *command, const int index, const ksMatrix2x4f *value);
void ksGpuGraphicsCommand_SetParmFloatMatrix3x2(ksGpuGraphicsCommand *command, const int index, const ksMatrix3x2f *value);
void ksGpuGraphicsCommand_SetParmFloatMatrix3x3(ksGpuGraphicsCommand *command, const int index, const ksMatrix3x3f *value);
void ksGpuGraphicsCommand_SetParmFloatMatrix3x4(ksGpuGraphicsCommand *command, const int index, const ksMatrix3x4f *value);
void ksGpuGraphicsCommand_SetParmFloatMatrix4x2(ksGpuGraphicsCommand *command, const int index, const ksMatrix4x2f *value);
void ksGpuGraphicsCommand_SetParmFloatMatrix4x3(ksGpuGraphicsCommand *command, const int index, const ksMatrix4x3f *value);
void ksGpuGraphicsCommand_SetParmFloatMatrix4x4(ksGpuGraphicsCommand *command, const int index, const ksMatrix4x4f *value);
void ksGpuGraphicsCommand_SetNumInstances(ksGpuGraphicsCommand *command, const int numInstances);

================================================================================================================================
*/

typedef struct {
    const ksGpuGraphicsPipeline *pipeline;
    const ksGpuBuffer *vertexBuffer;    // vertex buffer returned by ksGpuCommandBuffer_MapVertexAttributes
    const ksGpuBuffer *instanceBuffer;  // instance buffer returned by ksGpuCommandBuffer_MapInstanceAttributes
    ksGpuProgramParmState parmState;
    int numInstances;
} ksGpuGraphicsCommand;

void ksGpuGraphicsCommand_Init(ksGpuGraphicsCommand *command);
void ksGpuGraphicsCommand_SetPipeline(ksGpuGraphicsCommand *command, const ksGpuGraphicsPipeline *pipeline);
void ksGpuGraphicsCommand_SetVertexBuffer(ksGpuGraphicsCommand *command, const ksGpuBuffer *vertexBuffer);
void ksGpuGraphicsCommand_SetInstanceBuffer(ksGpuGraphicsCommand *command, const ksGpuBuffer *instanceBuffer);
void ksGpuGraphicsCommand_SetParmTextureSampled(ksGpuGraphicsCommand *command, const int index, const ksGpuTexture *texture);
void ksGpuGraphicsCommand_SetParmTextureStorage(ksGpuGraphicsCommand *command, const int index, const ksGpuTexture *texture);
void ksGpuGraphicsCommand_SetParmBufferUniform(ksGpuGraphicsCommand *command, const int index, const ksGpuBuffer *buffer);
void ksGpuGraphicsCommand_SetParmBufferStorage(ksGpuGraphicsCommand *command, const int index, const ksGpuBuffer *buffer);
void ksGpuGraphicsCommand_SetParmInt(ksGpuGraphicsCommand *command, const int index, const int *value);
void ksGpuGraphicsCommand_SetParmIntVector2(ksGpuGraphicsCommand *command, const int index, const ksVector2i *value);
void ksGpuGraphicsCommand_SetParmIntVector3(ksGpuGraphicsCommand *command, const int index, const ksVector3i *value);
void ksGpuGraphicsCommand_SetParmIntVector4(ksGpuGraphicsCommand *command, const int index, const ksVector4i *value);
void ksGpuGraphicsCommand_SetParmFloat(ksGpuGraphicsCommand *command, const int index, const float *value);
void ksGpuGraphicsCommand_SetParmFloatVector2(ksGpuGraphicsCommand *command, const int index, const ksVector2f *value);
void ksGpuGraphicsCommand_SetParmFloatVector3(ksGpuGraphicsCommand *command, const int index, const ksVector3f *value);
void ksGpuGraphicsCommand_SetParmFloatVector4(ksGpuGraphicsCommand *command, const int index, const ksVector3f *value);
void ksGpuGraphicsCommand_SetParmFloatMatrix2x2(ksGpuGraphicsCommand *command, const int index, const ksMatrix2x2f *value);
void ksGpuGraphicsCommand_SetParmFloatMatrix2x3(ksGpuGraphicsCommand *command, const int index, const ksMatrix2x3f *value);
void ksGpuGraphicsCommand_SetParmFloatMatrix2x4(ksGpuGraphicsCommand *command, const int index, const ksMatrix2x4f *value);
void ksGpuGraphicsCommand_SetParmFloatMatrix3x2(ksGpuGraphicsCommand *command, const int index, const ksMatrix3x2f *value);
void ksGpuGraphicsCommand_SetParmFloatMatrix3x3(ksGpuGraphicsCommand *command, const int index, const ksMatrix3x3f *value);
void ksGpuGraphicsCommand_SetParmFloatMatrix3x4(ksGpuGraphicsCommand *command, const int index, const ksMatrix3x4f *value);
void ksGpuGraphicsCommand_SetParmFloatMatrix4x2(ksGpuGraphicsCommand *command, const int index, const ksMatrix4x2f *value);
void ksGpuGraphicsCommand_SetParmFloatMatrix4x3(ksGpuGraphicsCommand *command, const int index, const ksMatrix4x3f *value);
void ksGpuGraphicsCommand_SetParmFloatMatrix4x4(ksGpuGraphicsCommand *command, const int index, const ksMatrix4x4f *value);
void ksGpuGraphicsCommand_SetNumInstances(ksGpuGraphicsCommand *command, const int numInstances);

#ifdef __cplusplus
}
#endif

#endif  // !KSGRAPHICSWRAPPER_OPENGL_H
