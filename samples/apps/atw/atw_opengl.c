/*
================================================================================================

Description :   Asynchronous Time Warp test utility for OpenGL.
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

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.


DESCRIPTION
===========

This implements the simplest form of time warp transform for OpenGL.
This transform corrects for optical aberration of the optics used in a virtual
reality headset, and only rotates the stereoscopic images based on the very latest
head orientation to reduce the motion-to-photon delay (or end-to-end latency).

This utility can be used to test whether or not a particular combination of hardware,
operating system and graphics driver is capable of rendering stereoscopic pairs of
images, while asynchronously (and ideally concurrently) warping the latest pair of
images onto the display, synchronized with the display refresh without dropping any
frames. Under high system load, the rendering of the stereoscopic images is allowed
to drop frames, but the asynchronous time warp must be able to warp the latest
stereoscopic images onto the display, synchronized with the display refresh
*without ever* dropping any frames.

There is one thread that renders the stereoscopic pairs of images by rendering a scene
to two textures, one for each eye. These eye textures are then handed over to the
asynchronous time warp in a thread safe manner. The asynchronous time warp runs in
another thread and continuously takes the last completed eye textures and warps them
onto the display.

Even though rendering commands are issued concurrently from two separate threads,
most current hardware and drivers serialize these rendering commands because the
hardware cannot actually execute multiple graphics/compute tasks concurrently.
Based on the task switching granularity of the GPU, and on how the rendering
commands are prioritized and serialized, the asynchronous time warp may, or may
not be able to stay synchronized with the display refresh.

On hardware that cannot execute multiple graphics/compute tasks concurrently, the
following is required to keep the asynchronous time warp synchronized with the
display refresh without dropping frames:

    - Context priorities.
    - Fine-grained and low latency priority based task switching.

To significantly reduce the latency in a virtual reality simulation, the asynchronous
time warp needs to be scheduled as close as possible to the display refresh.
In addition to the above requirements, the following is required to achieve this:

    - Accurate timing of the display refresh.
    - Predictable latency and throughput of the time warp execution.


PERFORMANCE
===========

When the frame rate drops, it can be hard to tell whether the stereoscopic rendering,
or the time warp rendering drops frames. Therefore four scrolling bar graphs are
drawn at the bottom left of the screen. Each bar represents a frame. New frames scroll
in on the right and old frames scroll out on the left.

The left-most bar graph represent the frame rate of the stereoscopic rendering (pink).
The next bar graph represents the frame rate of time warp rendering (green). Each bar
that is pink or green respectively reaches the top of the graph and represents a frame
rendered at the display refresh rate. When the frame rate drops, the bars turn red
and become shorter proportional to how much the frame rate drops.

The next two bar graphs shows the CPU and GPU time of the stereoscopic rendering (pink),
the time warp rendering (green) and the bar graph rendering (yellow). The times are
stacked in each graph. The full height of a graph represents a full frame time.
For instance, with a 60Hz display refresh rate, the full graph height represents 16.7
milliseconds.


RESOLUTIONS
===========

The rendering resolutions can be changed by adjusting the display resolution, the
eye image resolution, and the eye image MSAA. For each of these there are four levels:

Display Resolution:
    0: 1920 x 1080
    1: 2560 x 1440
    2: 3840 x 2160
    3: 7680 x 4320

Eye image resolution:
    0: 1024 x 1024
    1: 1536 x 1536
    2: 2048 x 2048
    3: 4096 x 4096

Eye image multi-sampling:
    0: 1x MSAA
    1: 2x MSAA
    2: 4x MSAA
    3: 8x MSAA


SCENE WORKLOAD
==============

The graphics work load of the scene that is rendered for each eye can be changed by
adjusting the number of draw calls, the number of triangles per draw call, the fragment
program complexity and the number of samples. For each of these there are four levels:

Number of draw calls:
    0: 8
    1: 64
    2: 512
    3: 4096

Number of triangles per draw call:
    0: 12
    1: 128
    2: 512
    3: 2048

Fragment program complexity:
    0: flat-shaded with 1 light
    1: normal-mapped with 100 lights
    2: normal-mapped with 1000 lights
    3: normal-mapped with 2000 lights

In the lower right corner of the screen there are four indicators that show
the current level for each. The levels are colored: 0 = green, 1 = blue,
2 = yellow and 3 = red.

The scene is normally rendered separately for each eye. However, there is also
an option to render the scene only once for both eyes (multi-view). The left-most
small indicator in the middle-bottom of the screen shows whether or not multi-view
is enabled: gray = off and red = on.


TIMEWARP SETTINGS
=================

The time warp can run in two modes. The first mode only corrects for spatial
aberration and the second mode also corrects for chromatic aberration.
The middle small indicator in the middle-bottom of the screen shows which mode
is used: gray = spatial and red = chromatic.

There are two implementations of the time warp. The first implementation uses
the conventional graphics pipeline and the second implementation uses compute.
The right-most small indicator in the middle-bottom of the screen shows which
implementation is used: gray = graphics and red = compute.


COMMAND-LINE INPUT
==================

The following command-line options can be used to change various settings.

    -a <.json>  load glTF scene
    -f          start fullscreen
    -v <s>      start with V-Sync disabled for this many seconds
    -h          start with head rotation disabled
    -p          start with the simulation paused
    -r <0-3>    set display resolution level
    -b <0-3>    set eye image resolution level
    -s <0-3>    set eye image multi-sampling level
    -q <0-3>    set per eye draw calls level
    -w <0-3>    set per eye triangles per draw call level
    -e <0-3>    set per eye fragment program complexity level
    -m <0-1>    enable/disable multi-view
    -c <0-1>    enable/disable correction for chromatic aberration
    -i <name>   set time warp implementation: graphics, compute
    -z <name>   set the render mode: atw, tw, scene
    -g          hide graphs
    -l <s>      log 10 frames of OpenGL commands after this many seconds
    -d          dump GLSL to files for conversion to SPIR-V


KEYBOARD INPUT
==============

The following keys can be used at run-time to change various settings.

    [F]      = toggle between windowed and fullscreen
    [V]      = toggle V-Sync on/off
    [H]      = toggle head rotation on/off
    [P]      = pause/resume the simulation
    [R]      = cycle screen resolution level
    [B]      = cycle eye buffer resolution level
    [S]      = cycle multi-sampling level
    [Q]      = cycle per eye draw calls level
    [W]      = cycle per eye triangles per draw call level
    [E]      = cycle per eye fragment program complexity level
    [M]      = toggle multi-view
    [C]      = toggle correction for chromatic aberration
    [I]      = toggle time warp implementation: graphics, compute
    [Z]      = cycle the render mode: atw, tw, scene
    [G]      = cycle between showing graphs, showing paused graphs and hiding graphs
    [L]      = log 10 frames of OpenGL commands
    [D]      = dump GLSL to files for conversion to SPIR-V
    [Esc]    = exit


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


COMMAND-LINE COMPILATION
========================

Microsoft Windows: Visual Studio 2013 Compiler:
    include\GL\glext.h  -> https://www.opengl.org/registry/api/GL/glext.h
    include\GL\wglext.h -> https://www.opengl.org/registry/api/GL/wglext.h
    "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x64
    cl /Zc:wchar_t /Zc:forScope /Wall /MD /GS /Gy /O2 /Oi /arch:SSE2 /Iinclude atw_opengl.c /link user32.lib gdi32.lib
Advapi32.lib opengl32.lib

Microsoft Windows: Intel Compiler 14.0
    include\GL\glext.h  -> https://www.opengl.org/registry/api/GL/glext.h
    include\GL\wglext.h -> https://www.opengl.org/registry/api/GL/wglext.h
    "C:\Program Files (x86)\Intel\Composer XE\bin\iclvars.bat" intel64
    icl /Qstd=c99 /Zc:wchar_t /Zc:forScope /Wall /MD /GS /Gy /O2 /Oi /arch:SSE2 /Iinclude atw_opengl.c /link user32.lib
gdi32.lib Advapi32.lib opengl32.lib

Linux: GCC 4.8.2 Xlib:
    sudo apt-get install libx11-dev
    sudo apt-get install libxxf86vm-dev
    sudo apt-get install libxrandr-dev
    sudo apt-get install mesa-common-dev
    sudo apt-get install libgl1-mesa-dev
    gcc -std=c99 -Wall -g -O2 -m64 -o atw_opengl atw_opengl.c -lm -lpthread -lX11 -lXxf86vm -lXrandr -lGL

Linux: GCC 4.8.2 XCB:
    sudo apt-get install libxcb1-dev
    sudo apt-get install libxcb-keysyms1-dev
    sudo apt-get install libxcb-icccm4-dev
    sudo apt-get install mesa-common-dev
    sudo apt-get install libgl1-mesa-dev
    gcc -std=c99 -Wall -g -O2 -o -m64 atw_opengl atw_opengl.c -lm -lpthread -lxcb -lxcb-keysyms -lxcb-randr -lxcb-glx -lxcb-dri2
-lGL

Apple macOS: Apple LLVM 6.0:
    clang -std=c99 -x objective-c -fno-objc-arc -Wall -g -O2 -o atw_opengl atw_opengl.c -framework Cocoa -framework OpenGL

Android for ARM from Windows: NDK Revision 11c - Android 21 - ANT/Gradle
    ANT:
        cd projects/android/ant/atw_opengl
        ndk-build
        ant debug
        adb install -r bin/atw_opengl-debug.apk
    Gradle:
        cd projects/android/gradle/atw_opengl
        gradlew build
        adb install -r build/outputs/apk/atw_opengl-all-debug.apk


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


VERSION HISTORY
===============

1.0        Initial version.

================================================================================================
*/

#include "gfxwrapper_opengl.h"

/*
================================================================================================================================

GPU buffer.

A buffer maintains a block of memory for a specific use by GPU programs (vertex, index, uniform, storage).
For optimal performance a buffer should only be created at load time, not at runtime.
The best performance is typically achieved when the buffer is not host visible.

ksGpuBufferType
ksGpuBuffer

static bool ksGpuBuffer_Create( ksGpuContext * context, ksGpuBuffer * buffer, const ksGpuBufferType type,
                                                        const size_t dataSize, const void * data, const bool hostVisible );
static void ksGpuBuffer_CreateReference( ksGpuContext * context, ksGpuBuffer * buffer, const ksGpuBuffer * other );
static void ksGpuBuffer_Destroy( ksGpuContext * context, ksGpuBuffer * buffer );

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

static bool ksGpuBuffer_Create(ksGpuContext *context, ksGpuBuffer *buffer, const ksGpuBufferType type, const size_t dataSize,
                               const void *data, const bool hostVisible) {
    UNUSED_PARM(context);
    UNUSED_PARM(hostVisible);

    buffer->target = ((type == KS_GPU_BUFFER_TYPE_VERTEX)
                          ? GL_ARRAY_BUFFER
                          : ((type == KS_GPU_BUFFER_TYPE_INDEX)
                                 ? GL_ELEMENT_ARRAY_BUFFER
                                 : ((type == KS_GPU_BUFFER_TYPE_UNIFORM)
                                        ? GL_UNIFORM_BUFFER
                                        : ((type == KS_GPU_BUFFER_TYPE_STORAGE) ? GL_SHADER_STORAGE_BUFFER : 0))));
    buffer->size = dataSize;

    GL(glGenBuffers(1, &buffer->buffer));
    GL(glBindBuffer(buffer->target, buffer->buffer));
    GL(glBufferData(buffer->target, dataSize, data, GL_STATIC_DRAW));
    GL(glBindBuffer(buffer->target, 0));

    buffer->owner = true;

    return true;
}

static void ksGpuBuffer_CreateReference(ksGpuContext *context, ksGpuBuffer *buffer, const ksGpuBuffer *other) {
    UNUSED_PARM(context);

    buffer->target = other->target;
    buffer->size = other->size;
    buffer->buffer = other->buffer;
    buffer->owner = false;
}

static void ksGpuBuffer_Destroy(ksGpuContext *context, ksGpuBuffer *buffer) {
    UNUSED_PARM(context);

    if (buffer->owner) {
        GL(glDeleteBuffers(1, &buffer->buffer));
    }
    memset(buffer, 0, sizeof(ksGpuBuffer));
}

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

static bool ksGpuTexture_Create2D( ksGpuContext * context, ksGpuTexture * texture,
                                                                        const ksGpuTextureFormat format, const ksGpuSampleCount
sampleCount, const int width, const int height, const int mipCount, const ksGpuTextureUsageFlags usageFlags, const void * data,
const size_t dataSize ); static bool ksGpuTexture_Create2DArray( ksGpuContext * context, ksGpuTexture * texture, const
ksGpuTextureFormat format, const ksGpuSampleCount sampleCount, const int width, const int height, const int layerCount, const int
mipCount, const ksGpuTextureUsageFlags usageFlags, const void * data, const size_t dataSize ); static bool
ksGpuTexture_CreateDefault( ksGpuContext * context, ksGpuTexture * texture, const ksGpuTextureDefault defaultType, const int width,
const int height, const int depth, const int layerCount, const int faceCount, const bool mipmaps, const bool border ); static bool
ksGpuTexture_CreateFromSwapchain( ksGpuContext * context, ksGpuTexture * texture, const ksGpuWindow * window, int index ); static
bool ksGpuTexture_CreateFromFile( ksGpuContext * context, ksGpuTexture * texture, const char * fileName ); static void
ksGpuTexture_Destroy( ksGpuContext * context, ksGpuTexture * texture );

static void ksGpuTexture_SetFilter( ksGpuContext * context, ksGpuTexture * texture, const ksGpuTextureFilter filter );
static void ksGpuTexture_SetAniso( ksGpuContext * context, ksGpuTexture * texture, const float maxAniso );
static void ksGpuTexture_SetWrapMode( ksGpuContext * context, ksGpuTexture * texture, const ksGpuTextureWrapMode wrapMode );

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

static int IntegerLog2(int i) {
    int r = 0;
    int t;
    t = ((~((i >> 16) + ~0U)) >> 27) & 0x10;
    r |= t;
    i >>= t;
    t = ((~((i >> 8) + ~0U)) >> 28) & 0x08;
    r |= t;
    i >>= t;
    t = ((~((i >> 4) + ~0U)) >> 29) & 0x04;
    r |= t;
    i >>= t;
    t = ((~((i >> 2) + ~0U)) >> 30) & 0x02;
    r |= t;
    i >>= t;
    return (r | (i >> 1));
}

// 'width' must be >= 1 and <= 32768.
// 'height' must be >= 1 and <= 32768.
// 'depth' must be >= 0 and <= 32768.
// 'layerCount' must be >= 0.
// 'faceCount' must be either 1 or 6.
// 'mipCount' must be -1 or >= 1.
// 'mipCount' includes the finest level.
// 'mipCount' set to -1 will allocate the full mip chain.
// 'data' may be NULL to allocate a texture without initialization.
// 'dataSize' is the full data size in bytes.
// The 'data' is expected to be stored packed on a per mip level basis.
// If 'data' != NULL and 'mipCount' <= 0, then the full mip chain will be generated from the finest data level.
static bool ksGpuTexture_CreateInternal(ksGpuContext *context, ksGpuTexture *texture, const char *fileName,
                                        const GLenum glInternalFormat, const ksGpuSampleCount sampleCount, const int width,
                                        const int height, const int depth, const int layerCount, const int faceCount,
                                        const int mipCount, const ksGpuTextureUsageFlags usageFlags, const void *data,
                                        const size_t dataSize, const bool mipSizeStored) {
    UNUSED_PARM(context);

    memset(texture, 0, sizeof(ksGpuTexture));

    assert(depth >= 0);
    assert(layerCount >= 0);
    assert(faceCount == 1 || faceCount == 6);

    if (width < 1 || width > 32768 || height < 1 || height > 32768 || depth < 0 || depth > 32768) {
        Error("%s: Invalid texture size (%dx%dx%d)", fileName, width, height, depth);
        return false;
    }

    if (faceCount != 1 && faceCount != 6) {
        Error("%s: Cube maps must have 6 faces (%d)", fileName, faceCount);
        return false;
    }

    if (faceCount == 6 && width != height) {
        Error("%s: Cube maps must be square (%dx%d)", fileName, width, height);
        return false;
    }

    if (depth > 0 && layerCount > 0) {
        Error("%s: 3D array textures not supported", fileName);
        return false;
    }

    const int maxDimension = width > height ? (width > depth ? width : depth) : (height > depth ? height : depth);
    const int maxMipLevels = (1 + IntegerLog2(maxDimension));

    if (mipCount > maxMipLevels) {
        Error("%s: Too many mip levels (%d > %d)", fileName, mipCount, maxMipLevels);
        return false;
    }

    const GLenum glTarget =
        ((depth > 0) ? GL_TEXTURE_3D
                     : ((faceCount == 6) ? ((layerCount > 0) ? GL_TEXTURE_CUBE_MAP_ARRAY : GL_TEXTURE_CUBE_MAP)
                                         : ((height > 0) ? ((layerCount > 0) ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D)
                                                         : ((layerCount > 0) ? GL_TEXTURE_1D_ARRAY : GL_TEXTURE_1D))));

    const int numStorageLevels = (mipCount >= 1) ? mipCount : maxMipLevels;

    GL(glGenTextures(1, &texture->texture));
    GL(glBindTexture(glTarget, texture->texture));
    if (depth <= 0 && layerCount <= 0) {
        if (sampleCount > KS_GPU_SAMPLE_COUNT_1) {
            GL(glTexStorage2DMultisample(glTarget, sampleCount, glInternalFormat, width, height, GL_TRUE));
        } else {
            GL(glTexStorage2D(glTarget, numStorageLevels, glInternalFormat, width, height));
        }
    } else {
        if (sampleCount > KS_GPU_SAMPLE_COUNT_1) {
            GL(glTexStorage3DMultisample(glTarget, sampleCount, glInternalFormat, width, height, MAX(depth, 1) * MAX(layerCount, 1),
                                         GL_TRUE));
        } else {
            GL(glTexStorage3D(glTarget, numStorageLevels, glInternalFormat, width, height, MAX(depth, 1) * MAX(layerCount, 1)));
        }
    }

    texture->target = glTarget;
    texture->format = glInternalFormat;
    texture->width = width;
    texture->height = height;
    texture->depth = depth;
    texture->layerCount = layerCount;
    texture->mipCount = numStorageLevels;
    texture->sampleCount = sampleCount;
    texture->usage = KS_GPU_TEXTURE_USAGE_UNDEFINED;
    texture->usageFlags = usageFlags;
    texture->wrapMode = KS_GPU_TEXTURE_WRAP_MODE_REPEAT;
    texture->filter = (numStorageLevels > 1) ? KS_GPU_TEXTURE_FILTER_BILINEAR : KS_GPU_TEXTURE_FILTER_LINEAR;
    texture->maxAnisotropy = 1.0f;

    if (data != NULL) {
        assert(sampleCount == KS_GPU_SAMPLE_COUNT_1);

        const int numDataLevels = (mipCount >= 1) ? mipCount : 1;
        const unsigned char *levelData = (const unsigned char *)data;
        const unsigned char *endOfBuffer = levelData + dataSize;
        bool compressed = false;

        for (int mipLevel = 0; mipLevel < numDataLevels; mipLevel++) {
            const int mipWidth = (width >> mipLevel) >= 1 ? (width >> mipLevel) : 1;
            const int mipHeight = (height >> mipLevel) >= 1 ? (height >> mipLevel) : 1;
            const int mipDepth = (depth >> mipLevel) >= 1 ? (depth >> mipLevel) : 1;

            size_t mipSize = 0;
            GLenum glFormat = GL_RGBA;
            GLenum glDataType = GL_UNSIGNED_BYTE;
            switch (glInternalFormat) {
                //
                // 8 bits per component
                //
                case GL_R8: {
                    mipSize = mipWidth * mipHeight * mipDepth * 1 * sizeof(unsigned char);
                    glFormat = GL_RED;
                    glDataType = GL_UNSIGNED_BYTE;
                    break;
                }
                case GL_RG8: {
                    mipSize = mipWidth * mipHeight * mipDepth * 2 * sizeof(unsigned char);
                    glFormat = GL_RG;
                    glDataType = GL_UNSIGNED_BYTE;
                    break;
                }
                case GL_RGBA8: {
                    mipSize = mipWidth * mipHeight * mipDepth * 4 * sizeof(unsigned char);
                    glFormat = GL_RGBA;
                    glDataType = GL_UNSIGNED_BYTE;
                    break;
                }

                case GL_R8_SNORM: {
                    mipSize = mipWidth * mipHeight * mipDepth * 1 * sizeof(char);
                    glFormat = GL_RED;
                    glDataType = GL_BYTE;
                    break;
                }
                case GL_RG8_SNORM: {
                    mipSize = mipWidth * mipHeight * mipDepth * 2 * sizeof(char);
                    glFormat = GL_RG;
                    glDataType = GL_BYTE;
                    break;
                }
                case GL_RGBA8_SNORM: {
                    mipSize = mipWidth * mipHeight * mipDepth * 4 * sizeof(char);
                    glFormat = GL_RGBA;
                    glDataType = GL_BYTE;
                    break;
                }

                case GL_R8UI: {
                    mipSize = mipWidth * mipHeight * mipDepth * 1 * sizeof(unsigned char);
                    glFormat = GL_RED;
                    glDataType = GL_UNSIGNED_BYTE;
                    break;
                }
                case GL_RG8UI: {
                    mipSize = mipWidth * mipHeight * mipDepth * 2 * sizeof(unsigned char);
                    glFormat = GL_RG;
                    glDataType = GL_UNSIGNED_BYTE;
                    break;
                }
                case GL_RGBA8UI: {
                    mipSize = mipWidth * mipHeight * mipDepth * 4 * sizeof(unsigned char);
                    glFormat = GL_RGBA;
                    glDataType = GL_UNSIGNED_BYTE;
                    break;
                }

                case GL_R8I: {
                    mipSize = mipWidth * mipHeight * mipDepth * 1 * sizeof(char);
                    glFormat = GL_RED;
                    glDataType = GL_BYTE;
                    break;
                }
                case GL_RG8I: {
                    mipSize = mipWidth * mipHeight * mipDepth * 2 * sizeof(char);
                    glFormat = GL_RG;
                    glDataType = GL_BYTE;
                    break;
                }
                case GL_RGBA8I: {
                    mipSize = mipWidth * mipHeight * mipDepth * 4 * sizeof(char);
                    glFormat = GL_RGBA;
                    glDataType = GL_BYTE;
                    break;
                }

                case GL_SR8_EXT: {
                    mipSize = mipWidth * mipHeight * mipDepth * 1 * sizeof(unsigned char);
                    glFormat = GL_RED;
                    glDataType = GL_UNSIGNED_BYTE;
                    break;
                }
                case GL_SRG8_EXT: {
                    mipSize = mipWidth * mipHeight * mipDepth * 2 * sizeof(unsigned char);
                    glFormat = GL_RG;
                    glDataType = GL_UNSIGNED_BYTE;
                    break;
                }
                case GL_SRGB8_ALPHA8: {
                    mipSize = mipWidth * mipHeight * mipDepth * 4 * sizeof(unsigned char);
                    glFormat = GL_RGBA;
                    glDataType = GL_UNSIGNED_BYTE;
                    break;
                }

                //
                // 16 bits per component
                //
#if defined(GL_R16)
                case GL_R16: {
                    mipSize = mipWidth * mipHeight * mipDepth * 1 * sizeof(unsigned short);
                    glFormat = GL_RED;
                    glDataType = GL_UNSIGNED_SHORT;
                    break;
                }
                case GL_RG16: {
                    mipSize = mipWidth * mipHeight * mipDepth * 2 * sizeof(unsigned short);
                    glFormat = GL_RG;
                    glDataType = GL_UNSIGNED_SHORT;
                    break;
                }
                case GL_RGBA16: {
                    mipSize = mipWidth * mipHeight * mipDepth * 4 * sizeof(unsigned short);
                    glFormat = GL_RGBA;
                    glDataType = GL_UNSIGNED_SHORT;
                    break;
                }
#elif defined(GL_R16_EXT)
                case GL_R16_EXT: {
                    mipSize = mipWidth * mipHeight * mipDepth * 1 * sizeof(unsigned short);
                    glFormat = GL_RED;
                    glDataType = GL_UNSIGNED_SHORT;
                    break;
                }
                case GL_RG16_EXT: {
                    mipSize = mipWidth * mipHeight * mipDepth * 2 * sizeof(unsigned short);
                    glFormat = GL_RG;
                    glDataType = GL_UNSIGNED_SHORT;
                    break;
                }
                case GL_RGB16_EXT: {
                    mipSize = mipWidth * mipHeight * mipDepth * 4 * sizeof(unsigned short);
                    glFormat = GL_RGBA;
                    glDataType = GL_UNSIGNED_SHORT;
                    break;
                }
#endif

#if defined(GL_R16_SNORM)
                case GL_R16_SNORM: {
                    mipSize = mipWidth * mipHeight * mipDepth * 1 * sizeof(short);
                    glFormat = GL_RED;
                    glDataType = GL_SHORT;
                    break;
                }
                case GL_RG16_SNORM: {
                    mipSize = mipWidth * mipHeight * mipDepth * 2 * sizeof(short);
                    glFormat = GL_RG;
                    glDataType = GL_SHORT;
                    break;
                }
                case GL_RGBA16_SNORM: {
                    mipSize = mipWidth * mipHeight * mipDepth * 4 * sizeof(short);
                    glFormat = GL_RGBA;
                    glDataType = GL_SHORT;
                    break;
                }
#elif defined(GL_R16_SNORM_EXT)
                case GL_R16_SNORM_EXT: {
                    mipSize = mipWidth * mipHeight * mipDepth * 1 * sizeof(short);
                    glFormat = GL_RED;
                    glDataType = GL_SHORT;
                    break;
                }
                case GL_RG16_SNORM_EXT: {
                    mipSize = mipWidth * mipHeight * mipDepth * 2 * sizeof(short);
                    glFormat = GL_RG;
                    glDataType = GL_SHORT;
                    break;
                }
                case GL_RGBA16_SNORM_EXT: {
                    mipSize = mipWidth * mipHeight * mipDepth * 4 * sizeof(short);
                    glFormat = GL_RGBA;
                    glDataType = GL_SHORT;
                    break;
                }
#endif

                case GL_R16UI: {
                    mipSize = mipWidth * mipHeight * mipDepth * 1 * sizeof(unsigned short);
                    glFormat = GL_RED;
                    glDataType = GL_UNSIGNED_SHORT;
                    break;
                }
                case GL_RG16UI: {
                    mipSize = mipWidth * mipHeight * mipDepth * 2 * sizeof(unsigned short);
                    glFormat = GL_RG;
                    glDataType = GL_UNSIGNED_SHORT;
                    break;
                }
                case GL_RGBA16UI: {
                    mipSize = mipWidth * mipHeight * mipDepth * 4 * sizeof(unsigned short);
                    glFormat = GL_RGBA;
                    glDataType = GL_UNSIGNED_SHORT;
                    break;
                }

                case GL_R16I: {
                    mipSize = mipWidth * mipHeight * mipDepth * 1 * sizeof(short);
                    glFormat = GL_RED;
                    glDataType = GL_SHORT;
                    break;
                }
                case GL_RG16I: {
                    mipSize = mipWidth * mipHeight * mipDepth * 2 * sizeof(short);
                    glFormat = GL_RG;
                    glDataType = GL_SHORT;
                    break;
                }
                case GL_RGBA16I: {
                    mipSize = mipWidth * mipHeight * mipDepth * 4 * sizeof(short);
                    glFormat = GL_RGBA;
                    glDataType = GL_SHORT;
                    break;
                }

                case GL_R16F: {
                    mipSize = mipWidth * mipHeight * mipDepth * 1 * sizeof(unsigned short);
                    glFormat = GL_RED;
                    glDataType = GL_HALF_FLOAT;
                    break;
                }
                case GL_RG16F: {
                    mipSize = mipWidth * mipHeight * mipDepth * 2 * sizeof(unsigned short);
                    glFormat = GL_RG;
                    glDataType = GL_HALF_FLOAT;
                    break;
                }
                case GL_RGBA16F: {
                    mipSize = mipWidth * mipHeight * mipDepth * 4 * sizeof(unsigned short);
                    glFormat = GL_RGBA;
                    glDataType = GL_HALF_FLOAT;
                    break;
                }

                //
                // 32 bits per component
                //
                case GL_R32UI: {
                    mipSize = mipWidth * mipHeight * mipDepth * 1 * sizeof(unsigned int);
                    glFormat = GL_RED;
                    glDataType = GL_UNSIGNED_INT;
                    break;
                }
                case GL_RG32UI: {
                    mipSize = mipWidth * mipHeight * mipDepth * 2 * sizeof(unsigned int);
                    glFormat = GL_RG;
                    glDataType = GL_UNSIGNED_INT;
                    break;
                }
                case GL_RGBA32UI: {
                    mipSize = mipWidth * mipHeight * mipDepth * 4 * sizeof(unsigned int);
                    glFormat = GL_RGBA;
                    glDataType = GL_UNSIGNED_INT;
                    break;
                }

                case GL_R32I: {
                    mipSize = mipWidth * mipHeight * mipDepth * 1 * sizeof(int);
                    glFormat = GL_RED;
                    glDataType = GL_INT;
                    break;
                }
                case GL_RG32I: {
                    mipSize = mipWidth * mipHeight * mipDepth * 2 * sizeof(int);
                    glFormat = GL_RG;
                    glDataType = GL_INT;
                    break;
                }
                case GL_RGBA32I: {
                    mipSize = mipWidth * mipHeight * mipDepth * 4 * sizeof(int);
                    glFormat = GL_RGBA;
                    glDataType = GL_INT;
                    break;
                }

                case GL_R32F: {
                    mipSize = mipWidth * mipHeight * mipDepth * 1 * sizeof(float);
                    glFormat = GL_RED;
                    glDataType = GL_FLOAT;
                    break;
                }
                case GL_RG32F: {
                    mipSize = mipWidth * mipHeight * mipDepth * 2 * sizeof(float);
                    glFormat = GL_RG;
                    glDataType = GL_FLOAT;
                    break;
                }
                case GL_RGBA32F: {
                    mipSize = mipWidth * mipHeight * mipDepth * 4 * sizeof(float);
                    glFormat = GL_RGBA;
                    glDataType = GL_FLOAT;
                    break;
                }

                //
                // S3TC/DXT/BC
                //
#if defined(GL_COMPRESSED_RGB_S3TC_DXT1_EXT)
                case GL_COMPRESSED_RGB_S3TC_DXT1_EXT: {
                    mipSize = ((mipWidth + 3) / 4) * ((mipHeight + 3) / 4) * mipDepth * 8;
                    compressed = true;
                    break;
                }
                case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT: {
                    mipSize = ((mipWidth + 3) / 4) * ((mipHeight + 3) / 4) * mipDepth * 8;
                    compressed = true;
                    break;
                }
                case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT: {
                    mipSize = ((mipWidth + 3) / 4) * ((mipHeight + 3) / 4) * mipDepth * 16;
                    compressed = true;
                    break;
                }
                case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT: {
                    mipSize = ((mipWidth + 3) / 4) * ((mipHeight + 3) / 4) * mipDepth * 16;
                    compressed = true;
                    break;
                }
#endif

#if defined(GL_COMPRESSED_SRGB_S3TC_DXT1_EXT)
                case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT: {
                    mipSize = ((mipWidth + 3) / 4) * ((mipHeight + 3) / 4) * mipDepth * 8;
                    compressed = true;
                    break;
                }
                case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT: {
                    mipSize = ((mipWidth + 3) / 4) * ((mipHeight + 3) / 4) * mipDepth * 8;
                    compressed = true;
                    break;
                }
                case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT: {
                    mipSize = ((mipWidth + 3) / 4) * ((mipHeight + 3) / 4) * mipDepth * 16;
                    compressed = true;
                    break;
                }
                case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT: {
                    mipSize = ((mipWidth + 3) / 4) * ((mipHeight + 3) / 4) * mipDepth * 16;
                    compressed = true;
                    break;
                }
#endif

#if defined(GL_COMPRESSED_LUMINANCE_LATC1_EXT)
                case GL_COMPRESSED_LUMINANCE_LATC1_EXT: {
                    mipSize = ((mipWidth + 3) / 4) * ((mipHeight + 3) / 4) * mipDepth * 8;
                    compressed = true;
                    break;
                }
                case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT: {
                    mipSize = ((mipWidth + 3) / 4) * ((mipHeight + 3) / 4) * mipDepth * 16;
                    compressed = true;
                    break;
                }

                case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT: {
                    mipSize = ((mipWidth + 3) / 4) * ((mipHeight + 3) / 4) * mipDepth * 8;
                    compressed = true;
                    break;
                }
                case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT: {
                    mipSize = ((mipWidth + 3) / 4) * ((mipHeight + 3) / 4) * mipDepth * 16;
                    compressed = true;
                    break;
                }
#endif

                //
                // ETC
                //
#if defined(GL_COMPRESSED_RGB8_ETC2)
                case GL_COMPRESSED_RGB8_ETC2: {
                    mipSize = ((mipWidth + 3) / 4) * ((mipHeight + 3) / 4) * mipDepth * 8;
                    compressed = true;
                    break;
                }
                case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2: {
                    mipSize = ((mipWidth + 3) / 4) * ((mipHeight + 3) / 4) * mipDepth * 8;
                    compressed = true;
                    break;
                }
                case GL_COMPRESSED_RGBA8_ETC2_EAC: {
                    mipSize = ((mipWidth + 3) / 4) * ((mipHeight + 3) / 4) * mipDepth * 16;
                    compressed = true;
                    break;
                }

                case GL_COMPRESSED_SRGB8_ETC2: {
                    mipSize = ((mipWidth + 3) / 4) * ((mipHeight + 3) / 4) * mipDepth * 8;
                    compressed = true;
                    break;
                }
                case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2: {
                    mipSize = ((mipWidth + 3) / 4) * ((mipHeight + 3) / 4) * mipDepth * 8;
                    compressed = true;
                    break;
                }
                case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC: {
                    mipSize = ((mipWidth + 3) / 4) * ((mipHeight + 3) / 4) * mipDepth * 16;
                    compressed = true;
                    break;
                }
#endif

#if defined(GL_COMPRESSED_R11_EAC)
                case GL_COMPRESSED_R11_EAC: {
                    mipSize = ((mipWidth + 3) / 4) * ((mipHeight + 3) / 4) * mipDepth * 8;
                    compressed = true;
                    break;
                }
                case GL_COMPRESSED_RG11_EAC: {
                    mipSize = ((mipWidth + 3) / 4) * ((mipHeight + 3) / 4) * mipDepth * 16;
                    compressed = true;
                    break;
                }

                case GL_COMPRESSED_SIGNED_R11_EAC: {
                    mipSize = ((mipWidth + 3) / 4) * ((mipHeight + 3) / 4) * mipDepth * 8;
                    compressed = true;
                    break;
                }
                case GL_COMPRESSED_SIGNED_RG11_EAC: {
                    mipSize = ((mipWidth + 3) / 4) * ((mipHeight + 3) / 4) * mipDepth * 16;
                    compressed = true;
                    break;
                }
#endif

                //
                // ASTC
                //
#if defined(GL_COMPRESSED_RGBA_ASTC_4x4_KHR)
                case GL_COMPRESSED_RGBA_ASTC_4x4_KHR: {
                    mipSize = ((mipWidth + 3) / 4) * ((mipHeight + 3) / 4) * mipDepth * 16;
                    compressed = true;
                    break;
                }
                case GL_COMPRESSED_RGBA_ASTC_5x4_KHR: {
                    mipSize = ((mipWidth + 4) / 5) * ((mipHeight + 3) / 4) * mipDepth * 16;
                    compressed = true;
                    break;
                }
                case GL_COMPRESSED_RGBA_ASTC_5x5_KHR: {
                    mipSize = ((mipWidth + 4) / 5) * ((mipHeight + 4) / 5) * mipDepth * 16;
                    compressed = true;
                    break;
                }
                case GL_COMPRESSED_RGBA_ASTC_6x5_KHR: {
                    mipSize = ((mipWidth + 5) / 6) * ((mipHeight + 4) / 5) * mipDepth * 16;
                    compressed = true;
                    break;
                }
                case GL_COMPRESSED_RGBA_ASTC_6x6_KHR: {
                    mipSize = ((mipWidth + 5) / 6) * ((mipHeight + 5) / 6) * mipDepth * 16;
                    compressed = true;
                    break;
                }
                case GL_COMPRESSED_RGBA_ASTC_8x5_KHR: {
                    mipSize = ((mipWidth + 7) / 8) * ((mipHeight + 4) / 5) * mipDepth * 16;
                    compressed = true;
                    break;
                }
                case GL_COMPRESSED_RGBA_ASTC_8x6_KHR: {
                    mipSize = ((mipWidth + 7) / 8) * ((mipHeight + 5) / 6) * mipDepth * 16;
                    compressed = true;
                    break;
                }
                case GL_COMPRESSED_RGBA_ASTC_8x8_KHR: {
                    mipSize = ((mipWidth + 7) / 8) * ((mipHeight + 7) / 8) * mipDepth * 16;
                    compressed = true;
                    break;
                }
                case GL_COMPRESSED_RGBA_ASTC_10x5_KHR: {
                    mipSize = ((mipWidth + 9) / 10) * ((mipHeight + 4) / 5) * mipDepth * 16;
                    compressed = true;
                    break;
                }
                case GL_COMPRESSED_RGBA_ASTC_10x6_KHR: {
                    mipSize = ((mipWidth + 9) / 10) * ((mipHeight + 5) / 6) * mipDepth * 16;
                    compressed = true;
                    break;
                }
                case GL_COMPRESSED_RGBA_ASTC_10x8_KHR: {
                    mipSize = ((mipWidth + 9) / 10) * ((mipHeight + 7) / 8) * mipDepth * 16;
                    compressed = true;
                    break;
                }
                case GL_COMPRESSED_RGBA_ASTC_10x10_KHR: {
                    mipSize = ((mipWidth + 9) / 10) * ((mipHeight + 9) / 10) * mipDepth * 16;
                    compressed = true;
                    break;
                }
                case GL_COMPRESSED_RGBA_ASTC_12x10_KHR: {
                    mipSize = ((mipWidth + 11) / 12) * ((mipHeight + 9) / 10) * mipDepth * 16;
                    compressed = true;
                    break;
                }
                case GL_COMPRESSED_RGBA_ASTC_12x12_KHR: {
                    mipSize = ((mipWidth + 11) / 12) * ((mipHeight + 11) / 12) * mipDepth * 16;
                    compressed = true;
                    break;
                }

                case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR: {
                    mipSize = ((mipWidth + 3) / 4) * ((mipHeight + 3) / 4) * mipDepth * 16;
                    compressed = true;
                    break;
                }
                case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR: {
                    mipSize = ((mipWidth + 4) / 5) * ((mipHeight + 3) / 4) * mipDepth * 16;
                    compressed = true;
                    break;
                }
                case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR: {
                    mipSize = ((mipWidth + 4) / 5) * ((mipHeight + 4) / 5) * mipDepth * 16;
                    compressed = true;
                    break;
                }
                case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR: {
                    mipSize = ((mipWidth + 5) / 6) * ((mipHeight + 4) / 5) * mipDepth * 16;
                    compressed = true;
                    break;
                }
                case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR: {
                    mipSize = ((mipWidth + 5) / 6) * ((mipHeight + 5) / 6) * mipDepth * 16;
                    compressed = true;
                    break;
                }
                case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR: {
                    mipSize = ((mipWidth + 7) / 8) * ((mipHeight + 4) / 5) * mipDepth * 16;
                    compressed = true;
                    break;
                }
                case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR: {
                    mipSize = ((mipWidth + 7) / 8) * ((mipHeight + 5) / 6) * mipDepth * 16;
                    compressed = true;
                    break;
                }
                case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR: {
                    mipSize = ((mipWidth + 7) / 8) * ((mipHeight + 7) / 8) * mipDepth * 16;
                    compressed = true;
                    break;
                }
                case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR: {
                    mipSize = ((mipWidth + 9) / 10) * ((mipHeight + 4) / 5) * mipDepth * 16;
                    compressed = true;
                    break;
                }
                case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR: {
                    mipSize = ((mipWidth + 9) / 10) * ((mipHeight + 5) / 6) * mipDepth * 16;
                    compressed = true;
                    break;
                }
                case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR: {
                    mipSize = ((mipWidth + 9) / 10) * ((mipHeight + 7) / 8) * mipDepth * 16;
                    compressed = true;
                    break;
                }
                case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR: {
                    mipSize = ((mipWidth + 9) / 10) * ((mipHeight + 9) / 10) * mipDepth * 16;
                    compressed = true;
                    break;
                }
                case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR: {
                    mipSize = ((mipWidth + 11) / 12) * ((mipHeight + 9) / 10) * mipDepth * 16;
                    compressed = true;
                    break;
                }
                case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR: {
                    mipSize = ((mipWidth + 11) / 12) * ((mipHeight + 11) / 12) * mipDepth * 16;
                    compressed = true;
                    break;
                }
#endif
                default: {
                    Error("%s: Unsupported image format %d", fileName, glInternalFormat);
                    GL(glBindTexture(glTarget, 0));
                    return false;
                }
            }

            if (layerCount > 0) {
                mipSize = mipSize * layerCount * faceCount;
            }

            if (mipSizeStored) {
                if (levelData + 4 > endOfBuffer) {
                    Error("%s: Image data exceeds buffer size", fileName);
                    GL(glBindTexture(glTarget, 0));
                    return false;
                }
                mipSize = (size_t) * (const unsigned int *)levelData;
                levelData += 4;
            }

            if (depth <= 0 && layerCount <= 0) {
                for (int face = 0; face < faceCount; face++) {
                    if (mipSize <= 0 || mipSize > (size_t)(endOfBuffer - levelData)) {
                        Error("%s: Mip %d data exceeds buffer size (%lld > %lld)", fileName, mipLevel, (uint64_t)mipSize,
                              (uint64_t)(endOfBuffer - levelData));
                        GL(glBindTexture(glTarget, 0));
                        return false;
                    }

                    const GLenum uploadTarget = (glTarget == GL_TEXTURE_CUBE_MAP) ? GL_TEXTURE_CUBE_MAP_POSITIVE_X : GL_TEXTURE_2D;
                    if (compressed) {
                        GL(glCompressedTexSubImage2D(uploadTarget + face, mipLevel, 0, 0, mipWidth, mipHeight, glInternalFormat,
                                                     (GLsizei)mipSize, levelData));
                    } else {
                        GL(glTexSubImage2D(uploadTarget + face, mipLevel, 0, 0, mipWidth, mipHeight, glFormat, glDataType,
                                           levelData));
                    }

                    levelData += mipSize;

                    if (mipSizeStored) {
                        levelData += 3 - ((mipSize + 3) % 4);
                        if (levelData > endOfBuffer) {
                            Error("%s: Image data exceeds buffer size", fileName);
                            GL(glBindTexture(glTarget, 0));
                            return false;
                        }
                    }
                }
            } else {
                if (mipSize <= 0 || mipSize > (size_t)(endOfBuffer - levelData)) {
                    Error("%s: Mip %d data exceeds buffer size (%lld > %lld)", fileName, mipLevel, (uint64_t)mipSize,
                          (uint64_t)(endOfBuffer - levelData));
                    GL(glBindTexture(glTarget, 0));
                    return false;
                }

                if (compressed) {
                    GL(glCompressedTexSubImage3D(glTarget, mipLevel, 0, 0, 0, mipWidth, mipHeight, mipDepth * MAX(layerCount, 1),
                                                 glInternalFormat, (GLsizei)mipSize, levelData));
                } else {
                    GL(glTexSubImage3D(glTarget, mipLevel, 0, 0, 0, mipWidth, mipHeight, mipDepth * MAX(layerCount, 1), glFormat,
                                       glDataType, levelData));
                }

                levelData += mipSize;

                if (mipSizeStored) {
                    levelData += 3 - ((mipSize + 3) % 4);
                    if (levelData > endOfBuffer) {
                        Error("%s: Image data exceeds buffer size", fileName);
                        GL(glBindTexture(glTarget, 0));
                        return false;
                    }
                }
            }
        }

        if (mipCount < 1) {
            // Can ony generate mip levels for uncompressed textures.
            assert(compressed == false);

            GL(glGenerateMipmap(glTarget));
        }
    }

    GL(glTexParameteri(glTarget, GL_TEXTURE_MIN_FILTER, (numStorageLevels > 1) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR));
    GL(glTexParameteri(glTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    GL(glBindTexture(glTarget, 0));

    texture->usage = KS_GPU_TEXTURE_USAGE_SAMPLED;

    return true;
}

static bool ksGpuTexture_Create2D(ksGpuContext *context, ksGpuTexture *texture, const ksGpuTextureFormat format,
                                  const ksGpuSampleCount sampleCount, const int width, const int height, const int mipCount,
                                  const ksGpuTextureUsageFlags usageFlags, const void *data, const size_t dataSize) {
    const int depth = 0;
    const int layerCount = 0;
    const int faceCount = 1;
    return ksGpuTexture_CreateInternal(context, texture, "data", (GLenum)format, sampleCount, width, height, depth, layerCount,
                                       faceCount, mipCount, usageFlags, data, dataSize, false);
}

static bool ksGpuTexture_Create2DArray(ksGpuContext *context, ksGpuTexture *texture, const ksGpuTextureFormat format,
                                       const ksGpuSampleCount sampleCount, const int width, const int height, const int layerCount,
                                       const int mipCount, const ksGpuTextureUsageFlags usageFlags, const void *data,
                                       const size_t dataSize) {
    const int depth = 0;
    const int faceCount = 1;
    return ksGpuTexture_CreateInternal(context, texture, "data", (GLenum)format, sampleCount, width, height, depth, layerCount,
                                       faceCount, mipCount, usageFlags, data, dataSize, false);
}

static bool ksGpuTexture_CreateDefault(ksGpuContext *context, ksGpuTexture *texture, const ksGpuTextureDefault defaultType,
                                       const int width, const int height, const int depth, const int layerCount,
                                       const int faceCount, const bool mipmaps, const bool border) {
    const int TEXEL_SIZE = 4;
    const int layerSize = width * height * TEXEL_SIZE;
    const int dataSize = MAX(depth, 1) * MAX(layerCount, 1) * faceCount * layerSize;
    unsigned char *data = (unsigned char *)malloc(dataSize);

    if (defaultType == KS_GPU_TEXTURE_DEFAULT_CHECKERBOARD) {
        const int blockSize = 32;  // must be a power of two
        for (int layer = 0; layer < MAX(depth, 1) * MAX(layerCount, 1) * faceCount; layer++) {
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    if ((((x / blockSize) ^ (y / blockSize)) & 1) == 0) {
                        data[layer * layerSize + (y * width + x) * TEXEL_SIZE + 0] = (layer & 1) == 0 ? 96 : 160;
                        data[layer * layerSize + (y * width + x) * TEXEL_SIZE + 1] = 64;
                        data[layer * layerSize + (y * width + x) * TEXEL_SIZE + 2] = (layer & 1) == 0 ? 255 : 96;
                    } else {
                        data[layer * layerSize + (y * width + x) * TEXEL_SIZE + 0] = (layer & 1) == 0 ? 64 : 160;
                        data[layer * layerSize + (y * width + x) * TEXEL_SIZE + 1] = 32;
                        data[layer * layerSize + (y * width + x) * TEXEL_SIZE + 2] = (layer & 1) == 0 ? 255 : 64;
                    }
                    data[layer * layerSize + (y * 128 + x) * TEXEL_SIZE + 3] = 255;
                }
            }
        }
    } else if (defaultType == KS_GPU_TEXTURE_DEFAULT_PYRAMIDS) {
        const int blockSize = 32;  // must be a power of two
        for (int layer = 0; layer < MAX(depth, 1) * MAX(layerCount, 1) * faceCount; layer++) {
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    const int mask = blockSize - 1;
                    const int lx = x & mask;
                    const int ly = y & mask;
                    const int rx = mask - lx;
                    const int ry = mask - ly;

                    char cx = 0;
                    char cy = 0;
                    if (lx != ly && lx != ry) {
                        int m = blockSize;
                        if (lx < m) {
                            m = lx;
                            cx = -96;
                            cy = 0;
                        }
                        if (ly < m) {
                            m = ly;
                            cx = 0;
                            cy = -96;
                        }
                        if (rx < m) {
                            m = rx;
                            cx = +96;
                            cy = 0;
                        }
                        if (ry < m) {
                            m = ry;
                            cx = 0;
                            cy = +96;
                        }
                    }
                    data[layer * layerSize + (y * width + x) * TEXEL_SIZE + 0] = 128 + cx;
                    data[layer * layerSize + (y * width + x) * TEXEL_SIZE + 1] = 128 + cy;
                    data[layer * layerSize + (y * width + x) * TEXEL_SIZE + 2] = 128 + 85;
                    data[layer * layerSize + (y * width + x) * TEXEL_SIZE + 3] = 255;
                }
            }
        }
    } else if (defaultType == KS_GPU_TEXTURE_DEFAULT_CIRCLES) {
        const int blockSize = 32;  // must be a power of two
        const int radius = 10;
        const unsigned char colors[4][4] = {
            {0xFF, 0x00, 0x00, 0xFF}, {0x00, 0xFF, 0x00, 0xFF}, {0x00, 0x00, 0xFF, 0xFF}, {0xFF, 0xFF, 0x00, 0xFF}};
        for (int layer = 0; layer < MAX(depth, 1) * MAX(layerCount, 1) * faceCount; layer++) {
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    // Pick a color per block of texels.
                    const int index = (((y / (blockSize / 2)) & 2) ^ ((x / (blockSize * 1)) & 2)) |
                                      (((x / (blockSize * 1)) & 1) ^ ((y / (blockSize * 2)) & 1));

                    // Draw a circle with radius 10 centered inside each 32x32 block of texels.
                    const int dX = (x & ~(blockSize - 1)) + (blockSize / 2) - x;
                    const int dY = (y & ~(blockSize - 1)) + (blockSize / 2) - y;
                    const int dS = abs(dX * dX + dY * dY - radius * radius);
                    const int scale = (dS <= blockSize) ? dS : blockSize;

                    for (int c = 0; c < TEXEL_SIZE - 1; c++) {
                        data[layer * layerSize + (y * width + x) * TEXEL_SIZE + c] =
                            (unsigned char)((colors[index][c] * scale) / blockSize);
                    }
                    data[layer * layerSize + (y * width + x) * TEXEL_SIZE + TEXEL_SIZE - 1] = 255;
                }
            }
        }
    }

    if (border) {
        for (int layer = 0; layer < MAX(depth, 1) * MAX(layerCount, 1) * faceCount; layer++) {
            for (int x = 0; x < width; x++) {
                data[layer * layerSize + (0 * width + x) * TEXEL_SIZE + 0] = 0;
                data[layer * layerSize + (0 * width + x) * TEXEL_SIZE + 1] = 0;
                data[layer * layerSize + (0 * width + x) * TEXEL_SIZE + 2] = 0;
                data[layer * layerSize + (0 * width + x) * TEXEL_SIZE + 3] = 255;

                data[layer * layerSize + ((height - 1) * width + x) * TEXEL_SIZE + 0] = 0;
                data[layer * layerSize + ((height - 1) * width + x) * TEXEL_SIZE + 1] = 0;
                data[layer * layerSize + ((height - 1) * width + x) * TEXEL_SIZE + 2] = 0;
                data[layer * layerSize + ((height - 1) * width + x) * TEXEL_SIZE + 3] = 255;
            }
            for (int y = 0; y < height; y++) {
                data[layer * layerSize + (y * width + 0) * TEXEL_SIZE + 0] = 0;
                data[layer * layerSize + (y * width + 0) * TEXEL_SIZE + 1] = 0;
                data[layer * layerSize + (y * width + 0) * TEXEL_SIZE + 2] = 0;
                data[layer * layerSize + (y * width + 0) * TEXEL_SIZE + 3] = 255;

                data[layer * layerSize + (y * width + width - 1) * TEXEL_SIZE + 0] = 0;
                data[layer * layerSize + (y * width + width - 1) * TEXEL_SIZE + 1] = 0;
                data[layer * layerSize + (y * width + width - 1) * TEXEL_SIZE + 2] = 0;
                data[layer * layerSize + (y * width + width - 1) * TEXEL_SIZE + 3] = 255;
            }
        }
    }

    const int mipCount = (mipmaps) ? -1 : 1;
    bool success =
        ksGpuTexture_CreateInternal(context, texture, "data", GL_RGBA8, KS_GPU_SAMPLE_COUNT_1, width, height, depth, layerCount,
                                    faceCount, mipCount, KS_GPU_TEXTURE_USAGE_SAMPLED, data, dataSize, false);

    free(data);

    return success;
}

static bool ksGpuTexture_CreateFromSwapchain(ksGpuContext *context, ksGpuTexture *texture, const ksGpuWindow *window, int index) {
    UNUSED_PARM(context);
    UNUSED_PARM(index);

    memset(texture, 0, sizeof(ksGpuTexture));

    texture->width = window->windowWidth;
    texture->height = window->windowHeight;
    texture->depth = 1;
    texture->layerCount = 1;
    texture->mipCount = 1;
    texture->sampleCount = KS_GPU_SAMPLE_COUNT_1;
    texture->usage = KS_GPU_TEXTURE_USAGE_UNDEFINED;
    texture->wrapMode = KS_GPU_TEXTURE_WRAP_MODE_REPEAT;
    texture->filter = KS_GPU_TEXTURE_FILTER_LINEAR;
    texture->maxAnisotropy = 1.0f;
    texture->format = ksGpuContext_InternalSurfaceColorFormat(window->colorFormat);
    texture->target = 0;
    texture->texture = 0;

    return true;
}

// KTX is a format for storing textures for OpenGL and OpenGL ES applications.
// https://www.khronos.org/opengles/sdk/tools/KTX/file_format_spec/
// This KTX loader does not do any conversions. In other words, the format
// stored in the KTX file must be the same as the glInternaltFormat.
static bool ksGpuTexture_CreateFromKTX(ksGpuContext *context, ksGpuTexture *texture, const char *fileName,
                                       const unsigned char *buffer, const size_t bufferSize) {
    memset(texture, 0, sizeof(ksGpuTexture));

#pragma pack(1)
    typedef struct {
        unsigned char identifier[12];
        unsigned int endianness;
        unsigned int glType;
        unsigned int glTypeSize;
        unsigned int glFormat;
        unsigned int glInternalFormat;
        unsigned int glBaseInternalFormat;
        unsigned int pixelWidth;
        unsigned int pixelHeight;
        unsigned int pixelDepth;
        unsigned int numberOfArrayElements;
        unsigned int numberOfFaces;
        unsigned int numberOfMipmapLevels;
        unsigned int bytesOfKeyValueData;
    } GlHeaderKTX_t;
#pragma pack()

    if (bufferSize < sizeof(GlHeaderKTX_t)) {
        Error("%s: Invalid KTX file", fileName);
        return false;
    }

    const unsigned char fileIdentifier[12] = {(unsigned char)'\xAB', 'K',  'T',  'X',    ' ', '1', '1',
                                              (unsigned char)'\xBB', '\r', '\n', '\x1A', '\n'};

    const GlHeaderKTX_t *header = (GlHeaderKTX_t *)buffer;
    if (memcmp(header->identifier, fileIdentifier, sizeof(fileIdentifier)) != 0) {
        Error("%s: Invalid KTX file", fileName);
        return false;
    }
    // only support little endian
    if (header->endianness != 0x04030201) {
        Error("%s: KTX file has wrong endianess", fileName);
        return false;
    }
    // skip the key value data
    const size_t startTex = sizeof(GlHeaderKTX_t) + header->bytesOfKeyValueData;
    if ((startTex < sizeof(GlHeaderKTX_t)) || (startTex >= bufferSize)) {
        Error("%s: Invalid KTX header sizes", fileName);
        return false;
    }

    const unsigned int derivedFormat = glGetFormatFromInternalFormat(header->glInternalFormat);
    const unsigned int derivedType = glGetTypeFromInternalFormat(header->glInternalFormat);

    UNUSED_PARM(derivedFormat);
    UNUSED_PARM(derivedType);

    // The glFormat and glType must either both be zero or both be non-zero.
    assert((header->glFormat == 0) == (header->glType == 0));
    // Uncompressed glTypeSize must be 1, 2, 4 or 8.
    assert(header->glFormat == 0 || header->glTypeSize == 1 || header->glTypeSize == 2 || header->glTypeSize == 4 ||
           header->glTypeSize == 8);
    // Uncompressed glFormat must match the format derived from glInternalFormat.
    assert(header->glFormat == 0 || header->glFormat == derivedFormat);
    // Uncompressed glType must match the type derived from glInternalFormat.
    assert(header->glFormat == 0 || header->glType == derivedType);
    // Uncompressed glBaseInternalFormat must be the same as glFormat.
    assert(header->glFormat == 0 || header->glBaseInternalFormat == header->glFormat);
    // Compressed glTypeSize must be 1.
    assert(header->glFormat != 0 || header->glTypeSize == 1);
    // Compressed glBaseInternalFormat must match the format drived from glInternalFormat.
    assert(header->glFormat != 0 || header->glBaseInternalFormat == derivedFormat);

    const int numberOfFaces = (header->numberOfFaces >= 1) ? header->numberOfFaces : 1;
    const GLenum format = header->glInternalFormat;

    return ksGpuTexture_CreateInternal(context, texture, fileName, format, KS_GPU_SAMPLE_COUNT_1, header->pixelWidth,
                                       header->pixelHeight, header->pixelDepth, header->numberOfArrayElements, numberOfFaces,
                                       header->numberOfMipmapLevels, KS_GPU_TEXTURE_USAGE_SAMPLED, buffer + startTex,
                                       bufferSize - startTex, true);
}

static bool ksGpuTexture_CreateFromFile(ksGpuContext *context, ksGpuTexture *texture, const char *fileName) {
    memset(texture, 0, sizeof(ksGpuTexture));

    FILE *fp = fopen(fileName, "rb");
    if (fp == NULL) {
        Error("Failed to open %s", fileName);
        return false;
    }

    fseek(fp, 0L, SEEK_END);
    size_t bufferSize = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    unsigned char *buffer = (unsigned char *)malloc(bufferSize);
    if (fread(buffer, 1, bufferSize, fp) != bufferSize) {
        Error("Failed to read %s", fileName);
        free(buffer);
        fclose(fp);
        return false;
    }
    fclose(fp);

    bool success = ksGpuTexture_CreateFromKTX(context, texture, fileName, buffer, bufferSize);

    free(buffer);

    return success;
}

static void ksGpuTexture_Destroy(ksGpuContext *context, ksGpuTexture *texture) {
    UNUSED_PARM(context);

    if (texture->texture) {
        GL(glDeleteTextures(1, &texture->texture));
    }
    memset(texture, 0, sizeof(ksGpuTexture));
}

static void ksGpuTexture_SetWrapMode(ksGpuContext *context, ksGpuTexture *texture, const ksGpuTextureWrapMode wrapMode) {
    UNUSED_PARM(context);

    texture->wrapMode = wrapMode;

    const GLint wrap =
        ((wrapMode == KS_GPU_TEXTURE_WRAP_MODE_CLAMP_TO_EDGE)
             ? GL_CLAMP_TO_EDGE
             : ((wrapMode == KS_GPU_TEXTURE_WRAP_MODE_CLAMP_TO_BORDER) ? glExtensions.texture_clamp_to_border_id : (GL_REPEAT)));

    GL(glBindTexture(texture->target, texture->texture));
    GL(glTexParameteri(texture->target, GL_TEXTURE_WRAP_S, wrap));
    GL(glTexParameteri(texture->target, GL_TEXTURE_WRAP_T, wrap));
    GL(glBindTexture(texture->target, 0));
}

static void ksGpuTexture_SetFilter(ksGpuContext *context, ksGpuTexture *texture, const ksGpuTextureFilter filter) {
    UNUSED_PARM(context);

    texture->filter = filter;

    GL(glBindTexture(texture->target, texture->texture));
    if (filter == KS_GPU_TEXTURE_FILTER_NEAREST) {
        GL(glTexParameteri(texture->target, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        GL(glTexParameteri(texture->target, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    } else if (filter == KS_GPU_TEXTURE_FILTER_LINEAR) {
        GL(glTexParameteri(texture->target, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GL(glTexParameteri(texture->target, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    } else if (filter == KS_GPU_TEXTURE_FILTER_BILINEAR) {
        GL(glTexParameteri(texture->target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
        GL(glTexParameteri(texture->target, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    }
    GL(glBindTexture(texture->target, 0));
}

static void ksGpuTexture_SetAniso(ksGpuContext *context, ksGpuTexture *texture, const float maxAniso) {
    UNUSED_PARM(context);

    texture->maxAnisotropy = maxAniso;

    GL(glBindTexture(texture->target, texture->texture));
    GL(glTexParameterf(texture->target, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso));
    GL(glBindTexture(texture->target, 0));
}

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

static void ksGpuTriangleIndexArray_CreateFromBuffer(ksGpuTriangleIndexArray *indices, const int indexCount,
                                                     const ksGpuBuffer *buffer) {
    indices->indexCount = indexCount;
    indices->indexArray = NULL;
    indices->buffer = buffer;
}

static void ksGpuTriangleIndexArray_Alloc(ksGpuTriangleIndexArray *indices, const int indexCount, const ksGpuTriangleIndex *data) {
    indices->indexCount = indexCount;
    indices->indexArray = (ksGpuTriangleIndex *)malloc(indexCount * sizeof(ksGpuTriangleIndex));
    if (data != NULL) {
        memcpy(indices->indexArray, data, indexCount * sizeof(ksGpuTriangleIndex));
    }
    indices->buffer = NULL;
}

static void ksGpuTriangleIndexArray_Free(ksGpuTriangleIndexArray *indices) {
    free(indices->indexArray);
    memset(indices, 0, sizeof(ksGpuTriangleIndexArray));
}

static size_t ksGpuVertexAttributeArrays_GetDataSize(const ksGpuVertexAttribute *layout, const int vertexCount,
                                                     const int attribsFlags) {
    size_t totalSize = 0;
    for (int i = 0; layout[i].attributeFlag != 0; i++) {
        const ksGpuVertexAttribute *v = &layout[i];
        if ((v->attributeFlag & attribsFlags) != 0) {
            totalSize += v->attributeSize;
        }
    }
    return vertexCount * totalSize;
}

static void ksGpuVertexAttributeArrays_Map(ksGpuVertexAttributeArrays *attribs, void *data, const size_t dataSize,
                                           const int vertexCount, const int attribsFlags) {
    unsigned char *dataBytePtr = (unsigned char *)data;
    size_t offset = 0;

    for (int i = 0; attribs->layout[i].attributeFlag != 0; i++) {
        const ksGpuVertexAttribute *v = &attribs->layout[i];
        void **attribPtr = (void **)(((char *)attribs) + v->attributeOffset);
        if ((v->attributeFlag & attribsFlags) != 0) {
            *attribPtr = (dataBytePtr + offset);
            offset += vertexCount * v->attributeSize;
        } else {
            *attribPtr = NULL;
        }
    }

    assert(offset == dataSize);
    UNUSED_PARM(dataSize);
}

static void ksGpuVertexAttributeArrays_CreateFromBuffer(ksGpuVertexAttributeArrays *attribs, const ksGpuVertexAttribute *layout,
                                                        const int vertexCount, const int attribsFlags, const ksGpuBuffer *buffer) {
    attribs->buffer = buffer;
    attribs->layout = layout;
    attribs->data = NULL;
    attribs->dataSize = 0;
    attribs->vertexCount = vertexCount;
    attribs->attribsFlags = attribsFlags;
}

static void ksGpuVertexAttributeArrays_Alloc(ksGpuVertexAttributeArrays *attribs, const ksGpuVertexAttribute *layout,
                                             const int vertexCount, const int attribsFlags) {
    const size_t dataSize = ksGpuVertexAttributeArrays_GetDataSize(layout, vertexCount, attribsFlags);
    void *data = malloc(dataSize);
    attribs->buffer = NULL;
    attribs->layout = layout;
    attribs->data = data;
    attribs->dataSize = dataSize;
    attribs->vertexCount = vertexCount;
    attribs->attribsFlags = attribsFlags;
    ksGpuVertexAttributeArrays_Map(attribs, data, dataSize, vertexCount, attribsFlags);
}

static void ksGpuVertexAttributeArrays_Free(ksGpuVertexAttributeArrays *attribs) {
    free(attribs->data);
    memset(attribs, 0, sizeof(ksGpuVertexAttributeArrays));
}

static void *ksGpuVertexAttributeArrays_FindAtribute(ksGpuVertexAttributeArrays *attribs, const char *name,
                                                     const ksGpuAttributeFormat format) {
    for (int i = 0; attribs->layout[i].attributeFlag != 0; i++) {
        const ksGpuVertexAttribute *v = &attribs->layout[i];
        if (v->attributeFormat == format && strcmp(v->name, name) == 0) {
            void **attribPtr = (void **)(((char *)attribs) + v->attributeOffset);
            return *attribPtr;
        }
    }
    return NULL;
}

static void ksGpuVertexAttributeArrays_CalculateTangents(ksGpuVertexAttributeArrays *attribs,
                                                         const ksGpuTriangleIndexArray *indices) {
    ksVector3f *vertexPosition =
        (ksVector3f *)ksGpuVertexAttributeArrays_FindAtribute(attribs, "vertexPosition", KS_GPU_ATTRIBUTE_FORMAT_R32G32B32_SFLOAT);
    ksVector3f *vertexNormal =
        (ksVector3f *)ksGpuVertexAttributeArrays_FindAtribute(attribs, "vertexNormal", KS_GPU_ATTRIBUTE_FORMAT_R32G32B32_SFLOAT);
    ksVector3f *vertexTangent =
        (ksVector3f *)ksGpuVertexAttributeArrays_FindAtribute(attribs, "vertexTangent", KS_GPU_ATTRIBUTE_FORMAT_R32G32B32_SFLOAT);
    ksVector3f *vertexBinormal =
        (ksVector3f *)ksGpuVertexAttributeArrays_FindAtribute(attribs, "vertexBinormal", KS_GPU_ATTRIBUTE_FORMAT_R32G32B32_SFLOAT);
    ksVector2f *vertexUv0 =
        (ksVector2f *)ksGpuVertexAttributeArrays_FindAtribute(attribs, "vertexUv0", KS_GPU_ATTRIBUTE_FORMAT_R32G32_SFLOAT);

    if (vertexPosition == NULL || vertexNormal == NULL || vertexTangent == NULL || vertexBinormal == NULL || vertexUv0 == NULL) {
        assert(false);
        return;
    }

    for (int i = 0; i < attribs->vertexCount; i++) {
        ksVector3f_Set(&vertexTangent[i], 0.0f);
        ksVector3f_Set(&vertexBinormal[i], 0.0f);
    }

    for (int i = 0; i < indices->indexCount; i += 3) {
        const ksGpuTriangleIndex *v = indices->indexArray + i;
        const ksVector3f *pos = vertexPosition;
        const ksVector2f *uv0 = vertexUv0;

        const ksVector3f delta0 = {pos[v[1]].x - pos[v[0]].x, pos[v[1]].y - pos[v[0]].y, pos[v[1]].z - pos[v[0]].z};
        const ksVector3f delta1 = {pos[v[2]].x - pos[v[1]].x, pos[v[2]].y - pos[v[1]].y, pos[v[2]].z - pos[v[1]].z};
        const ksVector3f delta2 = {pos[v[0]].x - pos[v[2]].x, pos[v[0]].y - pos[v[2]].y, pos[v[0]].z - pos[v[2]].z};

        const float l0 = delta0.x * delta0.x + delta0.y * delta0.y + delta0.z * delta0.z;
        const float l1 = delta1.x * delta1.x + delta1.y * delta1.y + delta1.z * delta1.z;
        const float l2 = delta2.x * delta2.x + delta2.y * delta2.y + delta2.z * delta2.z;

        const int i0 = (l0 > l1) ? (l0 > l2 ? 2 : 1) : (l1 > l2 ? 0 : 1);
        const int i1 = (i0 + 1) % 3;
        const int i2 = (i0 + 2) % 3;

        const ksVector3f d0 = {pos[v[i1]].x - pos[v[i0]].x, pos[v[i1]].y - pos[v[i0]].y, pos[v[i1]].z - pos[v[i0]].z};
        const ksVector3f d1 = {pos[v[i2]].x - pos[v[i0]].x, pos[v[i2]].y - pos[v[i0]].y, pos[v[i2]].z - pos[v[i0]].z};

        const ksVector2f s0 = {uv0[v[i1]].x - uv0[v[i0]].x, uv0[v[i1]].y - uv0[v[i0]].y};
        const ksVector2f s1 = {uv0[v[i2]].x - uv0[v[i0]].x, uv0[v[i2]].y - uv0[v[i0]].y};

        const float sign = (s0.x * s1.y - s0.y * s1.x) < 0.0f ? -1.0f : 1.0f;

        ksVector3f tangent = {(d0.x * s1.y - d1.x * s0.y) * sign, (d0.y * s1.y - d1.y * s0.y) * sign,
                              (d0.z * s1.y - d1.z * s0.y) * sign};
        ksVector3f binormal = {(d1.x * s0.x - d0.x * s1.x) * sign, (d1.y * s0.x - d0.y * s1.x) * sign,
                               (d1.z * s0.x - d0.z * s1.x) * sign};

        ksVector3f_Normalize(&tangent);
        ksVector3f_Normalize(&binormal);

        for (int j = 0; j < 3; j++) {
            vertexTangent[v[j]].x += tangent.x;
            vertexTangent[v[j]].y += tangent.y;
            vertexTangent[v[j]].z += tangent.z;

            vertexBinormal[v[j]].x += binormal.x;
            vertexBinormal[v[j]].y += binormal.y;
            vertexBinormal[v[j]].z += binormal.z;
        }
    }

    for (int i = 0; i < attribs->vertexCount; i++) {
        ksVector3f_Normalize(&vertexTangent[i]);
        ksVector3f_Normalize(&vertexBinormal[i]);
    }
}

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

static const ksGpuVertexAttribute DefaultVertexAttributeLayout[] = {
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

static void ksGpuGeometry_Create( ksGpuContext * context, ksGpuGeometry * geometry,
                                                                const ksGpuVertexAttributeArrays * attribs,
                                                                const ksGpuTriangleIndexArray * indices );
static void ksGpuGeometry_CreateQuad( ksGpuContext * context, ksGpuGeometry * geometry, const float offset, const float scale );
static void ksGpuGeometry_CreateCube( ksGpuContext * context, ksGpuGeometry * geometry, const float offset, const float scale );
static void ksGpuGeometry_CreateTorus( ksGpuContext * context, ksGpuGeometry * geometry, const int tesselation, const float offset,
const float scale ); static void ksGpuGeometry_Destroy( ksGpuContext * context, ksGpuGeometry * geometry );

static void ksGpuGeometry_AddInstanceAttributes( ksGpuContext * context, ksGpuGeometry * geometry, const int numInstances, const int
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

static void ksGpuGeometry_Create(ksGpuContext *context, ksGpuGeometry *geometry, const ksGpuVertexAttributeArrays *attribs,
                                 const ksGpuTriangleIndexArray *indices) {
    memset(geometry, 0, sizeof(ksGpuGeometry));

    geometry->layout = attribs->layout;
    geometry->vertexAttribsFlags = attribs->attribsFlags;
    geometry->vertexCount = attribs->vertexCount;
    geometry->indexCount = indices->indexCount;

    if (attribs->buffer != NULL) {
        ksGpuBuffer_CreateReference(context, &geometry->vertexBuffer, attribs->buffer);
    } else {
        ksGpuBuffer_Create(context, &geometry->vertexBuffer, KS_GPU_BUFFER_TYPE_VERTEX, attribs->dataSize, attribs->data, false);
    }
    if (indices->buffer != NULL) {
        ksGpuBuffer_CreateReference(context, &geometry->indexBuffer, indices->buffer);
    } else {
        ksGpuBuffer_Create(context, &geometry->indexBuffer, KS_GPU_BUFFER_TYPE_INDEX,
                           indices->indexCount * sizeof(indices->indexArray[0]), indices->indexArray, false);
    }
}

// The quad is centered about the origin and without offset/scale spans the [-1, 1] X-Y range.
static void ksGpuGeometry_CreateQuad(ksGpuContext *context, ksGpuGeometry *geometry, const float offset, const float scale) {
    const ksVector3f quadPositions[4] = {{-1.0f, -1.0f, 0.0f}, {+1.0f, -1.0f, 0.0f}, {+1.0f, +1.0f, 0.0f}, {-1.0f, +1.0f, 0.0f}};

    const ksVector3f quadNormals[4] = {{0.0f, 0.0f, +1.0f}, {0.0f, 0.0f, +1.0f}, {0.0f, 0.0f, +1.0f}, {0.0f, 0.0f, +1.0f}};

    const ksVector2f quadUvs[4] = {{0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f}};

    const ksGpuTriangleIndex quadIndices[6] = {0, 1, 2, 2, 3, 0};

    ksDefaultVertexAttributeArrays quadAttributeArrays;
    ksGpuVertexAttributeArrays_Alloc(&quadAttributeArrays.base, DefaultVertexAttributeLayout, 4,
                                     VERTEX_ATTRIBUTE_FLAG_POSITION | VERTEX_ATTRIBUTE_FLAG_NORMAL | VERTEX_ATTRIBUTE_FLAG_TANGENT |
                                         VERTEX_ATTRIBUTE_FLAG_BINORMAL | VERTEX_ATTRIBUTE_FLAG_UV0);

    for (int i = 0; i < 4; i++) {
        quadAttributeArrays.position[i].x = (quadPositions[i].x + offset) * scale;
        quadAttributeArrays.position[i].y = (quadPositions[i].y + offset) * scale;
        quadAttributeArrays.position[i].z = (quadPositions[i].z + offset) * scale;
        quadAttributeArrays.normal[i].x = quadNormals[i].x;
        quadAttributeArrays.normal[i].y = quadNormals[i].y;
        quadAttributeArrays.normal[i].z = quadNormals[i].z;
        quadAttributeArrays.uv0[i].x = quadUvs[i].x;
        quadAttributeArrays.uv0[i].y = quadUvs[i].y;
    }

    ksGpuTriangleIndexArray quadIndexArray;
    ksGpuTriangleIndexArray_Alloc(&quadIndexArray, 6, quadIndices);

    ksGpuVertexAttributeArrays_CalculateTangents(&quadAttributeArrays.base, &quadIndexArray);

    ksGpuGeometry_Create(context, geometry, &quadAttributeArrays.base, &quadIndexArray);

    ksGpuVertexAttributeArrays_Free(&quadAttributeArrays.base);
    ksGpuTriangleIndexArray_Free(&quadIndexArray);
}

// The cube is centered about the origin and without offset/scale spans the [-1, 1] X-Y-Z range.
static void ksGpuGeometry_CreateCube(ksGpuContext *context, ksGpuGeometry *geometry, const float offset, const float scale) {
    const ksVector3f cubePositions[24] = {
        {+1.0f, -1.0f, -1.0f}, {+1.0f, +1.0f, -1.0f}, {+1.0f, +1.0f, +1.0f}, {+1.0f, -1.0f, +1.0f},
        {-1.0f, -1.0f, -1.0f}, {-1.0f, -1.0f, +1.0f}, {-1.0f, +1.0f, +1.0f}, {-1.0f, +1.0f, -1.0f},

        {-1.0f, +1.0f, -1.0f}, {+1.0f, +1.0f, -1.0f}, {+1.0f, +1.0f, +1.0f}, {-1.0f, +1.0f, +1.0f},
        {-1.0f, -1.0f, -1.0f}, {-1.0f, -1.0f, +1.0f}, {+1.0f, -1.0f, +1.0f}, {+1.0f, -1.0f, -1.0f},

        {-1.0f, -1.0f, +1.0f}, {+1.0f, -1.0f, +1.0f}, {+1.0f, +1.0f, +1.0f}, {-1.0f, +1.0f, +1.0f},
        {-1.0f, -1.0f, -1.0f}, {-1.0f, +1.0f, -1.0f}, {+1.0f, +1.0f, -1.0f}, {+1.0f, -1.0f, -1.0f}};

    const ksVector3f cubeNormals[24] = {{+1.0f, 0.0f, 0.0f}, {+1.0f, 0.0f, 0.0f}, {+1.0f, 0.0f, 0.0f}, {+1.0f, 0.0f, 0.0f},
                                        {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f},

                                        {0.0f, +1.0f, 0.0f}, {0.0f, +1.0f, 0.0f}, {0.0f, +1.0f, 0.0f}, {0.0f, +1.0f, 0.0f},
                                        {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f},

                                        {0.0f, 0.0f, +1.0f}, {0.0f, 0.0f, +1.0f}, {0.0f, 0.0f, +1.0f}, {0.0f, 0.0f, +1.0f},
                                        {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}};

    const ksVector2f cubeUvs[24] = {
        {0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f},

        {0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f},

        {0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f},
    };

    const ksGpuTriangleIndex cubeIndices[36] = {0,  1,  2,  2,  3,  0,  4,  5,  6,  6,  7,  4,  8,  10, 9,  10, 8,  11,
                                                12, 14, 13, 14, 12, 15, 16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 23, 20};

    ksDefaultVertexAttributeArrays cubeAttributeArrays;
    ksGpuVertexAttributeArrays_Alloc(&cubeAttributeArrays.base, DefaultVertexAttributeLayout, 24,
                                     VERTEX_ATTRIBUTE_FLAG_POSITION | VERTEX_ATTRIBUTE_FLAG_NORMAL | VERTEX_ATTRIBUTE_FLAG_TANGENT |
                                         VERTEX_ATTRIBUTE_FLAG_BINORMAL | VERTEX_ATTRIBUTE_FLAG_UV0);

    for (int i = 0; i < 24; i++) {
        cubeAttributeArrays.position[i].x = (cubePositions[i].x + offset) * scale;
        cubeAttributeArrays.position[i].y = (cubePositions[i].y + offset) * scale;
        cubeAttributeArrays.position[i].z = (cubePositions[i].z + offset) * scale;
        cubeAttributeArrays.normal[i].x = cubeNormals[i].x;
        cubeAttributeArrays.normal[i].y = cubeNormals[i].y;
        cubeAttributeArrays.normal[i].z = cubeNormals[i].z;
        cubeAttributeArrays.uv0[i].x = cubeUvs[i].x;
        cubeAttributeArrays.uv0[i].y = cubeUvs[i].y;
    }

    ksGpuTriangleIndexArray cubeIndexArray;
    ksGpuTriangleIndexArray_Alloc(&cubeIndexArray, 36, cubeIndices);

    ksGpuVertexAttributeArrays_CalculateTangents(&cubeAttributeArrays.base, &cubeIndexArray);

    ksGpuGeometry_Create(context, geometry, &cubeAttributeArrays.base, &cubeIndexArray);

    ksGpuVertexAttributeArrays_Free(&cubeAttributeArrays.base);
    ksGpuTriangleIndexArray_Free(&cubeIndexArray);
}

// The torus is centered about the origin and without offset/scale spans the [-1, 1] X-Y range and the [-0.3, 0.3] Z range.
static void ksGpuGeometry_CreateTorus(ksGpuContext *context, ksGpuGeometry *geometry, const int tesselation, const float offset,
                                      const float scale) {
    const int minorTesselation = tesselation;
    const int majorTesselation = tesselation;
    const float tubeRadius = 0.3f;
    const float tubeCenter = 0.7f;
    const int vertexCount = (majorTesselation + 1) * (minorTesselation + 1);
    const int indexCount = majorTesselation * minorTesselation * 6;

    ksDefaultVertexAttributeArrays torusAttributeArrays;
    ksGpuVertexAttributeArrays_Alloc(&torusAttributeArrays.base, DefaultVertexAttributeLayout, vertexCount,
                                     VERTEX_ATTRIBUTE_FLAG_POSITION | VERTEX_ATTRIBUTE_FLAG_NORMAL | VERTEX_ATTRIBUTE_FLAG_TANGENT |
                                         VERTEX_ATTRIBUTE_FLAG_BINORMAL | VERTEX_ATTRIBUTE_FLAG_UV0);

    for (int u = 0; u <= majorTesselation; u++) {
        const float ua = 2.0f * MATH_PI * u / majorTesselation;
        const float majorCos = cosf(ua);
        const float majorSin = sinf(ua);

        for (int v = 0; v <= minorTesselation; v++) {
            const float va = MATH_PI + 2.0f * MATH_PI * v / minorTesselation;
            const float minorCos = cosf(va);
            const float minorSin = sinf(va);

            const float minorX = tubeCenter + tubeRadius * minorCos;
            const float minorZ = tubeRadius * minorSin;

            const int index = u * (minorTesselation + 1) + v;
            torusAttributeArrays.position[index].x = (minorX * majorCos * scale) + offset;
            torusAttributeArrays.position[index].y = (minorX * majorSin * scale) + offset;
            torusAttributeArrays.position[index].z = (minorZ * scale) + offset;
            torusAttributeArrays.normal[index].x = minorCos * majorCos;
            torusAttributeArrays.normal[index].y = minorCos * majorSin;
            torusAttributeArrays.normal[index].z = minorSin;
            torusAttributeArrays.uv0[index].x = (float)u / majorTesselation;
            torusAttributeArrays.uv0[index].y = (float)v / minorTesselation;
        }
    }

    ksGpuTriangleIndexArray torusIndexArray;
    ksGpuTriangleIndexArray_Alloc(&torusIndexArray, indexCount, NULL);

    for (int u = 0; u < majorTesselation; u++) {
        for (int v = 0; v < minorTesselation; v++) {
            const int index = (u * minorTesselation + v) * 6;
            torusIndexArray.indexArray[index + 0] = (ksGpuTriangleIndex)((u + 0) * (minorTesselation + 1) + (v + 0));
            torusIndexArray.indexArray[index + 1] = (ksGpuTriangleIndex)((u + 1) * (minorTesselation + 1) + (v + 0));
            torusIndexArray.indexArray[index + 2] = (ksGpuTriangleIndex)((u + 1) * (minorTesselation + 1) + (v + 1));
            torusIndexArray.indexArray[index + 3] = (ksGpuTriangleIndex)((u + 1) * (minorTesselation + 1) + (v + 1));
            torusIndexArray.indexArray[index + 4] = (ksGpuTriangleIndex)((u + 0) * (minorTesselation + 1) + (v + 1));
            torusIndexArray.indexArray[index + 5] = (ksGpuTriangleIndex)((u + 0) * (minorTesselation + 1) + (v + 0));
        }
    }

    ksGpuVertexAttributeArrays_CalculateTangents(&torusAttributeArrays.base, &torusIndexArray);

    ksGpuGeometry_Create(context, geometry, &torusAttributeArrays.base, &torusIndexArray);

    ksGpuVertexAttributeArrays_Free(&torusAttributeArrays.base);
    ksGpuTriangleIndexArray_Free(&torusIndexArray);
}

static void ksGpuGeometry_Destroy(ksGpuContext *context, ksGpuGeometry *geometry) {
    ksGpuBuffer_Destroy(context, &geometry->indexBuffer);
    ksGpuBuffer_Destroy(context, &geometry->vertexBuffer);
    if (geometry->instanceBuffer.size != 0) {
        ksGpuBuffer_Destroy(context, &geometry->instanceBuffer);
    }

    memset(geometry, 0, sizeof(ksGpuGeometry));
}

static void ksGpuGeometry_AddInstanceAttributes(ksGpuContext *context, ksGpuGeometry *geometry, const int numInstances,
                                                const int instanceAttribsFlags) {
    assert(geometry->layout != NULL);
    assert((geometry->vertexAttribsFlags & instanceAttribsFlags) == 0);

    geometry->instanceCount = numInstances;
    geometry->instanceAttribsFlags = instanceAttribsFlags;

    const size_t dataSize = ksGpuVertexAttributeArrays_GetDataSize(geometry->layout, numInstances, geometry->instanceAttribsFlags);

    ksGpuBuffer_Create(context, &geometry->instanceBuffer, KS_GPU_BUFFER_TYPE_VERTEX, dataSize, NULL, false);
}

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

static bool ksGpuRenderPass_Create(ksGpuContext *context, ksGpuRenderPass *renderPass, const ksGpuSurfaceColorFormat colorFormat,
                                   const ksGpuSurfaceDepthFormat depthFormat, const ksGpuSampleCount sampleCount,
                                   const ksGpuRenderPassType type, const int flags) {
    UNUSED_PARM(context);

    assert(type == KS_GPU_RENDERPASS_TYPE_INLINE);

    renderPass->type = type;
    renderPass->flags = flags;
    renderPass->colorFormat = colorFormat;
    renderPass->depthFormat = depthFormat;
    renderPass->sampleCount = sampleCount;
    return true;
}

static void ksGpuRenderPass_Destroy(ksGpuContext *context, ksGpuRenderPass *renderPass) {
    UNUSED_PARM(context);
    UNUSED_PARM(renderPass);
}

/*
================================================================================================================================

GPU framebuffer.

A framebuffer encapsulates either a swapchain or a buffered set of textures.
For optimal performance a framebuffer should only be created at load time, not at runtime.

ksGpuFramebuffer

static bool ksGpuFramebuffer_CreateFromSwapchain( ksGpuWindow * window, ksGpuFramebuffer * framebuffer, ksGpuRenderPass * renderPass
); static bool ksGpuFramebuffer_CreateFromTextures( ksGpuContext * context, ksGpuFramebuffer * framebuffer, ksGpuRenderPass *
renderPass, const int width, const int height, const int numBuffers ); static bool ksGpuFramebuffer_CreateFromTextureArrays(
ksGpuContext * context, ksGpuFramebuffer * framebuffer, ksGpuRenderPass * renderPass, const int width, const int height, const int
numLayers, const int numBuffers, const bool multiview ); static void ksGpuFramebuffer_Destroy( ksGpuContext * context,
ksGpuFramebuffer * framebuffer );

static int ksGpuFramebuffer_GetWidth( const ksGpuFramebuffer * framebuffer );
static int ksGpuFramebuffer_GetHeight( const ksGpuFramebuffer * framebuffer );
static ksScreenRect ksGpuFramebuffer_GetRect( const ksGpuFramebuffer * framebuffer );
static int ksGpuFramebuffer_GetBufferCount( const ksGpuFramebuffer * framebuffer );
static ksGpuTexture * ksGpuFramebuffer_GetColorTexture( const ksGpuFramebuffer * framebuffer );

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

static bool ksGpuFramebuffer_CreateFromSwapchain(ksGpuWindow *window, ksGpuFramebuffer *framebuffer, ksGpuRenderPass *renderPass) {
    assert(window->sampleCount == renderPass->sampleCount);

    UNUSED_PARM(renderPass);

    memset(framebuffer, 0, sizeof(ksGpuFramebuffer));

    static const int NUM_BUFFERS = 1;

    framebuffer->colorTextures = (ksGpuTexture *)malloc(NUM_BUFFERS * sizeof(ksGpuTexture));
    framebuffer->renderTexture = 0;
    framebuffer->depthBuffer = 0;
    framebuffer->renderBuffers = (GLuint *)malloc(NUM_BUFFERS * sizeof(GLuint));
    framebuffer->resolveBuffers = framebuffer->renderBuffers;
    framebuffer->multiView = false;
    framebuffer->sampleCount = KS_GPU_SAMPLE_COUNT_1;
    framebuffer->numFramebuffersPerTexture = 1;
    framebuffer->numBuffers = NUM_BUFFERS;
    framebuffer->currentBuffer = 0;

    // Create the color textures.
    for (int bufferIndex = 0; bufferIndex < NUM_BUFFERS; bufferIndex++) {
        assert(renderPass->colorFormat == window->colorFormat);
        assert(renderPass->depthFormat == window->depthFormat);

        ksGpuTexture_CreateFromSwapchain(&window->context, &framebuffer->colorTextures[bufferIndex], window, bufferIndex);

        assert(window->windowWidth == framebuffer->colorTextures[bufferIndex].width);
        assert(window->windowHeight == framebuffer->colorTextures[bufferIndex].height);

        framebuffer->renderBuffers[bufferIndex] = 0;
    }

    return true;
}

static bool ksGpuFramebuffer_CreateFromTextures(ksGpuContext *context, ksGpuFramebuffer *framebuffer, ksGpuRenderPass *renderPass,
                                                const int width, const int height, const int numBuffers) {
    UNUSED_PARM(context);

    memset(framebuffer, 0, sizeof(ksGpuFramebuffer));

    framebuffer->colorTextures = (ksGpuTexture *)malloc(numBuffers * sizeof(ksGpuTexture));
    framebuffer->renderTexture = 0;
    framebuffer->depthBuffer = 0;
    framebuffer->renderBuffers = (GLuint *)malloc(numBuffers * sizeof(GLuint));
    framebuffer->resolveBuffers = framebuffer->renderBuffers;
    framebuffer->multiView = false;
    framebuffer->sampleCount = KS_GPU_SAMPLE_COUNT_1;
    framebuffer->numFramebuffersPerTexture = 1;
    framebuffer->numBuffers = numBuffers;
    framebuffer->currentBuffer = 0;

    const ksGpuMsaaMode mode = ((renderPass->sampleCount > KS_GPU_SAMPLE_COUNT_1 && glExtensions.multi_sampled_resolve)
                                    ? MSAA_RESOLVE
                                    : ((renderPass->sampleCount > KS_GPU_SAMPLE_COUNT_1) ? MSAA_BLIT : MSAA_OFF));

    // Create the color textures.
    const GLenum colorFormat = ksGpuContext_InternalSurfaceColorFormat(renderPass->colorFormat);
    for (int bufferIndex = 0; bufferIndex < numBuffers; bufferIndex++) {
        ksGpuTexture_Create2D(context, &framebuffer->colorTextures[bufferIndex], (ksGpuTextureFormat)colorFormat,
                              KS_GPU_SAMPLE_COUNT_1, width, height, 1,
                              KS_GPU_TEXTURE_USAGE_SAMPLED | KS_GPU_TEXTURE_USAGE_COLOR_ATTACHMENT | KS_GPU_TEXTURE_USAGE_STORAGE,
                              NULL, 0);
        ksGpuTexture_SetWrapMode(context, &framebuffer->colorTextures[bufferIndex], KS_GPU_TEXTURE_WRAP_MODE_CLAMP_TO_BORDER);
    }

    // Create the depth buffer.
    if (renderPass->depthFormat != KS_GPU_SURFACE_DEPTH_FORMAT_NONE) {
        const GLenum depthFormat = ksGpuContext_InternalSurfaceDepthFormat(renderPass->depthFormat);

        GL(glGenRenderbuffers(1, &framebuffer->depthBuffer));
        GL(glBindRenderbuffer(GL_RENDERBUFFER, framebuffer->depthBuffer));
        if (mode == MSAA_RESOLVE) {
            GL(glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER, renderPass->sampleCount, depthFormat, width, height));
        } else if (mode == MSAA_BLIT) {
            GL(glRenderbufferStorageMultisample(GL_RENDERBUFFER, renderPass->sampleCount, depthFormat, width, height));
        } else {
            GL(glRenderbufferStorage(GL_RENDERBUFFER, depthFormat, width, height));
        }
        GL(glBindRenderbuffer(GL_RENDERBUFFER, 0));
    }

    // Create the render buffers.
    const int numRenderBuffers = (mode == MSAA_BLIT) ? 1 : numBuffers;
    for (int bufferIndex = 0; bufferIndex < numRenderBuffers; bufferIndex++) {
        GL(glGenFramebuffers(1, &framebuffer->renderBuffers[bufferIndex]));
        GL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer->renderBuffers[bufferIndex]));
        if (mode == MSAA_RESOLVE) {
            GL(glFramebufferTexture2DMultisampleEXT(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                                    framebuffer->colorTextures[bufferIndex].texture, 0, renderPass->sampleCount));
        } else if (mode == MSAA_BLIT) {
            GL(glRenderbufferStorageMultisample(GL_RENDERBUFFER, renderPass->sampleCount, colorFormat, width, height));
        } else {
            GL(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                      framebuffer->colorTextures[bufferIndex].texture, 0));
        }
        if (renderPass->depthFormat != KS_GPU_SURFACE_DEPTH_FORMAT_NONE) {
            GL(glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, framebuffer->depthBuffer));
        }
        GL(glGetIntegerv(GL_SAMPLES, &framebuffer->sampleCount));
        GL(GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));
        GL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
        GL(glClear(GL_COLOR_BUFFER_BIT));
        GL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            Error("Incomplete frame buffer object: %s", GlFramebufferStatusString(status));
            return false;
        }
    }

    // Create the resolve buffers.
    if (mode == MSAA_BLIT) {
        framebuffer->resolveBuffers = (GLuint *)malloc(numBuffers * sizeof(GLuint));
        for (int bufferIndex = 0; bufferIndex < numBuffers; bufferIndex++) {
            framebuffer->renderBuffers[bufferIndex] = framebuffer->renderBuffers[0];

            GL(glGenFramebuffers(1, &framebuffer->resolveBuffers[bufferIndex]));
            GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                      framebuffer->colorTextures[bufferIndex].texture, 0));
            GL(GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));
            GL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
            if (status != GL_FRAMEBUFFER_COMPLETE) {
                Error("Incomplete frame buffer object: %s", GlFramebufferStatusString(status));
                return false;
            }
        }
    }

    return true;
}

static bool ksGpuFramebuffer_CreateFromTextureArrays(ksGpuContext *context, ksGpuFramebuffer *framebuffer,
                                                     ksGpuRenderPass *renderPass, const int width, const int height,
                                                     const int numLayers, const int numBuffers, const bool multiview) {
    UNUSED_PARM(context);

    memset(framebuffer, 0, sizeof(ksGpuFramebuffer));

    framebuffer->colorTextures = (ksGpuTexture *)malloc(numBuffers * sizeof(ksGpuTexture));
    framebuffer->depthBuffer = 0;
    framebuffer->multiView = multiview;
    framebuffer->sampleCount = KS_GPU_SAMPLE_COUNT_1;
    framebuffer->numFramebuffersPerTexture = (multiview ? 1 : numLayers);
    framebuffer->renderBuffers = (GLuint *)malloc(numBuffers * framebuffer->numFramebuffersPerTexture * sizeof(GLuint));
    framebuffer->resolveBuffers = framebuffer->renderBuffers;
    framebuffer->numBuffers = numBuffers;
    framebuffer->currentBuffer = 0;

    const ksGpuMsaaMode mode =
        ((renderPass->sampleCount > KS_GPU_SAMPLE_COUNT_1 && !multiview && glExtensions.multi_sampled_resolve)
             ? MSAA_RESOLVE
             : ((renderPass->sampleCount > KS_GPU_SAMPLE_COUNT_1 && multiview && glExtensions.multi_view_multi_sampled_resolve)
                    ? MSAA_RESOLVE
                    : ((renderPass->sampleCount > KS_GPU_SAMPLE_COUNT_1 && glExtensions.multi_sampled_storage) ? MSAA_BLIT
                                                                                                               : MSAA_OFF)));

    // Create the color textures.
    const GLenum colorFormat = ksGpuContext_InternalSurfaceColorFormat(renderPass->colorFormat);
    for (int bufferIndex = 0; bufferIndex < numBuffers; bufferIndex++) {
        ksGpuTexture_Create2DArray(
            context, &framebuffer->colorTextures[bufferIndex], (ksGpuTextureFormat)colorFormat, KS_GPU_SAMPLE_COUNT_1, width,
            height, numLayers, 1,
            KS_GPU_TEXTURE_USAGE_SAMPLED | KS_GPU_TEXTURE_USAGE_COLOR_ATTACHMENT | KS_GPU_TEXTURE_USAGE_STORAGE, NULL, 0);
        ksGpuTexture_SetWrapMode(context, &framebuffer->colorTextures[bufferIndex], KS_GPU_TEXTURE_WRAP_MODE_CLAMP_TO_BORDER);
    }

    // Create the render texture.
    if (mode == MSAA_BLIT) {
        GL(glGenTextures(1, &framebuffer->renderTexture));
        GL(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, framebuffer->renderTexture));
        GL(glTexStorage3DMultisample(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, renderPass->sampleCount, colorFormat, width, height,
                                     numLayers, GL_TRUE));
        GL(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, 0));
    }

    // Create the depth buffer.
    if (renderPass->depthFormat != KS_GPU_SURFACE_DEPTH_FORMAT_NONE) {
        const GLenum depthFormat = ksGpuContext_InternalSurfaceDepthFormat(renderPass->depthFormat);
        const GLenum target = (mode == MSAA_BLIT) ? GL_TEXTURE_2D_MULTISAMPLE_ARRAY : GL_TEXTURE_2D_ARRAY;

        GL(glGenTextures(1, &framebuffer->depthBuffer));
        GL(glBindTexture(target, framebuffer->depthBuffer));
        if (mode == MSAA_BLIT) {
            GL(glTexStorage3DMultisample(target, renderPass->sampleCount, depthFormat, width, height, numLayers, GL_TRUE));
        } else {
            GL(glTexStorage3D(target, 1, depthFormat, width, height, numLayers));
        }
        GL(glBindTexture(target, 0));
    }

    // Create the render buffers.
    const int numRenderBuffers = (mode == MSAA_BLIT) ? 1 : numBuffers;
    for (int bufferIndex = 0; bufferIndex < numRenderBuffers; bufferIndex++) {
        for (int layerIndex = 0; layerIndex < framebuffer->numFramebuffersPerTexture; layerIndex++) {
            GL(glGenFramebuffers(1,
                                 &framebuffer->renderBuffers[bufferIndex * framebuffer->numFramebuffersPerTexture + layerIndex]));
            GL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER,
                                 framebuffer->renderBuffers[bufferIndex * framebuffer->numFramebuffersPerTexture + layerIndex]));
            if (multiview) {
                if (mode == MSAA_RESOLVE) {
                    GL(glFramebufferTextureMultisampleMultiviewOVR(
                        GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, framebuffer->colorTextures[bufferIndex].texture, 0 /* level */,
                        renderPass->sampleCount /* samples */, 0 /* baseViewIndex */, numLayers /* numViews */));
                    if (renderPass->depthFormat != KS_GPU_SURFACE_DEPTH_FORMAT_NONE) {
                        GL(glFramebufferTextureMultisampleMultiviewOVR(
                            GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, framebuffer->depthBuffer, 0 /* level */,
                            renderPass->sampleCount /* samples */, 0 /* baseViewIndex */, numLayers /* numViews */));
                    }
                } else if (mode == MSAA_BLIT) {
                    GL(glFramebufferTextureMultiviewOVR(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, framebuffer->renderTexture,
                                                        0 /* level */, 0 /* baseViewIndex */, numLayers /* numViews */));
                    if (renderPass->depthFormat != KS_GPU_SURFACE_DEPTH_FORMAT_NONE) {
                        GL(glFramebufferTextureMultiviewOVR(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, framebuffer->depthBuffer,
                                                            0 /* level */, 0 /* baseViewIndex */, numLayers /* numViews */));
                    }
                } else {
                    GL(glFramebufferTextureMultiviewOVR(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                                        framebuffer->colorTextures[bufferIndex].texture, 0 /* level */,
                                                        0 /* baseViewIndex */, numLayers /* numViews */));
                    if (renderPass->depthFormat != KS_GPU_SURFACE_DEPTH_FORMAT_NONE) {
                        GL(glFramebufferTextureMultiviewOVR(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, framebuffer->depthBuffer,
                                                            0 /* level */, 0 /* baseViewIndex */, numLayers /* numViews */));
                    }
                }
            } else {
                if (mode == MSAA_RESOLVE) {
                    // Note: using glFramebufferTextureMultisampleMultiviewOVR with a single view because there is no
                    // glFramebufferTextureLayerMultisampleEXT
                    GL(glFramebufferTextureMultisampleMultiviewOVR(
                        GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, framebuffer->colorTextures[bufferIndex].texture, 0 /* level */,
                        renderPass->sampleCount /* samples */, layerIndex /* baseViewIndex */, 1 /* numViews */));
                    if (renderPass->depthFormat != KS_GPU_SURFACE_DEPTH_FORMAT_NONE) {
                        GL(glFramebufferTextureMultisampleMultiviewOVR(
                            GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, framebuffer->depthBuffer, 0 /* level */,
                            renderPass->sampleCount /* samples */, layerIndex /* baseViewIndex */, 1 /* numViews */));
                    }
                } else if (mode == MSAA_BLIT) {
                    GL(glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, framebuffer->renderTexture,
                                                 0 /* level */, layerIndex /* layerIndex */));
                    if (renderPass->depthFormat != KS_GPU_SURFACE_DEPTH_FORMAT_NONE) {
                        GL(glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, framebuffer->depthBuffer,
                                                     0 /* level */, layerIndex /* layerIndex */));
                    }
                } else {
                    GL(glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                                 framebuffer->colorTextures[bufferIndex].texture, 0 /* level */,
                                                 layerIndex /* layerIndex */));
                    if (renderPass->depthFormat != KS_GPU_SURFACE_DEPTH_FORMAT_NONE) {
                        GL(glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, framebuffer->depthBuffer,
                                                     0 /* level */, layerIndex /* layerIndex */));
                    }
                }
            }
            GL(glGetIntegerv(GL_SAMPLES, &framebuffer->sampleCount));
            GL(GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));
            GL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
            if (status != GL_FRAMEBUFFER_COMPLETE) {
                Error("Incomplete frame buffer object: %s", GlFramebufferStatusString(status));
                return false;
            }
        }
    }

    // Create the resolve buffers.
    if (mode == MSAA_BLIT) {
        framebuffer->resolveBuffers = (GLuint *)malloc(numBuffers * framebuffer->numFramebuffersPerTexture * sizeof(GLuint));
        for (int bufferIndex = 0; bufferIndex < numBuffers; bufferIndex++) {
            for (int layerIndex = 0; layerIndex < framebuffer->numFramebuffersPerTexture; layerIndex++) {
                framebuffer->renderBuffers[bufferIndex * framebuffer->numFramebuffersPerTexture + layerIndex] =
                    framebuffer->renderBuffers[layerIndex];

                GL(glGenFramebuffers(
                    1, &framebuffer->resolveBuffers[bufferIndex * framebuffer->numFramebuffersPerTexture + layerIndex]));
                GL(glBindFramebuffer(
                    GL_DRAW_FRAMEBUFFER,
                    framebuffer->resolveBuffers[bufferIndex * framebuffer->numFramebuffersPerTexture + layerIndex]));
                GL(glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, framebuffer->colorTextures[bufferIndex].texture,
                                             0 /* level */, layerIndex /* layerIndex */));
                GL(GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));
                GL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
                if (status != GL_FRAMEBUFFER_COMPLETE) {
                    Error("Incomplete frame buffer object: %s", GlFramebufferStatusString(status));
                    return false;
                }
            }
        }
    }
    return true;
}

static void ksGpuFramebuffer_Destroy(ksGpuContext *context, ksGpuFramebuffer *framebuffer) {
    for (int bufferIndex = 0; bufferIndex < framebuffer->numBuffers; bufferIndex++) {
        if (framebuffer->resolveBuffers != framebuffer->renderBuffers) {
            for (int layerIndex = 0; layerIndex < framebuffer->numFramebuffersPerTexture; layerIndex++) {
                if (framebuffer->resolveBuffers[bufferIndex * framebuffer->numFramebuffersPerTexture + layerIndex] != 0) {
                    GL(glDeleteFramebuffers(
                        1, &framebuffer->resolveBuffers[bufferIndex * framebuffer->numFramebuffersPerTexture + layerIndex]));
                }
            }
        }
        if (bufferIndex == 0 ||
            framebuffer->renderBuffers[bufferIndex * framebuffer->numFramebuffersPerTexture + 0] != framebuffer->renderBuffers[0]) {
            for (int layerIndex = 0; layerIndex < framebuffer->numFramebuffersPerTexture; layerIndex++) {
                if (framebuffer->renderBuffers[bufferIndex * framebuffer->numFramebuffersPerTexture + layerIndex] != 0) {
                    GL(glDeleteFramebuffers(
                        1, &framebuffer->renderBuffers[bufferIndex * framebuffer->numFramebuffersPerTexture + layerIndex]));
                }
            }
        }
    }
    if (framebuffer->depthBuffer != 0) {
        if (framebuffer->colorTextures[0].layerCount > 0) {
            GL(glDeleteTextures(1, &framebuffer->depthBuffer));
        } else {
            GL(glDeleteRenderbuffers(1, &framebuffer->depthBuffer));
        }
    }
    if (framebuffer->renderTexture != 0) {
        if (framebuffer->colorTextures[0].layerCount > 0) {
            GL(glDeleteTextures(1, &framebuffer->renderTexture));
        } else {
            GL(glDeleteRenderbuffers(1, &framebuffer->renderTexture));
        }
    }
    for (int bufferIndex = 0; bufferIndex < framebuffer->numBuffers; bufferIndex++) {
        if (framebuffer->colorTextures[bufferIndex].texture != 0) {
            ksGpuTexture_Destroy(context, &framebuffer->colorTextures[bufferIndex]);
        }
    }
    if (framebuffer->resolveBuffers != framebuffer->renderBuffers) {
        free(framebuffer->resolveBuffers);
    }
    free(framebuffer->renderBuffers);
    free(framebuffer->colorTextures);

    memset(framebuffer, 0, sizeof(ksGpuFramebuffer));
}

static int ksGpuFramebuffer_GetWidth(const ksGpuFramebuffer *framebuffer) {
    return framebuffer->colorTextures[framebuffer->currentBuffer].width;
}

static int ksGpuFramebuffer_GetHeight(const ksGpuFramebuffer *framebuffer) {
    return framebuffer->colorTextures[framebuffer->currentBuffer].height;
}

static ksScreenRect ksGpuFramebuffer_GetRect(const ksGpuFramebuffer *framebuffer) {
    ksScreenRect rect;
    rect.x = 0;
    rect.y = 0;
    rect.width = framebuffer->colorTextures[framebuffer->currentBuffer].width;
    rect.height = framebuffer->colorTextures[framebuffer->currentBuffer].height;
    return rect;
}

static int ksGpuFramebuffer_GetBufferCount(const ksGpuFramebuffer *framebuffer) { return framebuffer->numBuffers; }

static ksGpuTexture *ksGpuFramebuffer_GetColorTexture(const ksGpuFramebuffer *framebuffer) {
    assert(framebuffer->colorTextures != NULL);
    return &framebuffer->colorTextures[framebuffer->currentBuffer];
}

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

#define MAX_PROGRAM_PARMS 16

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
    int offsetForIndex[MAX_PROGRAM_PARMS];   // push constant offsets into ksGpuProgramParmState::data based on
                                             // ksGpuProgramParm::index
    GLint parmLocations[MAX_PROGRAM_PARMS];  // OpenGL locations
    GLint parmBindings[MAX_PROGRAM_PARMS];
    GLint numSampledTextureBindings;
    GLint numStorageTextureBindings;
    GLint numUniformBufferBindings;
    GLint numStorageBufferBindings;
} ksGpuProgramParmLayout;

static bool ksGpuProgramParm_IsOpaqueBinding(const ksGpuProgramParmType type) {
    return ((type == KS_GPU_PROGRAM_PARM_TYPE_TEXTURE_SAMPLED)
                ? true
                : ((type == KS_GPU_PROGRAM_PARM_TYPE_TEXTURE_STORAGE)
                       ? true
                       : ((type == KS_GPU_PROGRAM_PARM_TYPE_BUFFER_UNIFORM)
                              ? true
                              : ((type == KS_GPU_PROGRAM_PARM_TYPE_BUFFER_STORAGE) ? true : false))));
}

static int ksGpuProgramParm_GetPushConstantSize(const ksGpuProgramParmType type) {
    static const int parmSize[KS_GPU_PROGRAM_PARM_TYPE_MAX] = {(unsigned int)0,
                                                               (unsigned int)0,
                                                               (unsigned int)0,
                                                               (unsigned int)0,
                                                               (unsigned int)sizeof(int),
                                                               (unsigned int)sizeof(int[2]),
                                                               (unsigned int)sizeof(int[3]),
                                                               (unsigned int)sizeof(int[4]),
                                                               (unsigned int)sizeof(float),
                                                               (unsigned int)sizeof(float[2]),
                                                               (unsigned int)sizeof(float[3]),
                                                               (unsigned int)sizeof(float[4]),
                                                               (unsigned int)sizeof(float[2][2]),
                                                               (unsigned int)sizeof(float[2][3]),
                                                               (unsigned int)sizeof(float[2][4]),
                                                               (unsigned int)sizeof(float[3][2]),
                                                               (unsigned int)sizeof(float[3][3]),
                                                               (unsigned int)sizeof(float[3][4]),
                                                               (unsigned int)sizeof(float[4][2]),
                                                               (unsigned int)sizeof(float[4][3]),
                                                               (unsigned int)sizeof(float[4][4])};
    assert(ARRAY_SIZE(parmSize) == KS_GPU_PROGRAM_PARM_TYPE_MAX);
    return parmSize[type];
}

static const char *ksGpuProgramParm_GetPushConstantGlslType(const ksGpuProgramParmType type) {
    static const char *glslType[KS_GPU_PROGRAM_PARM_TYPE_MAX] = {"",       "",       "",     "",       "int",    "ivec2",  "ivec3",
                                                                 "ivec4",  "float",  "vec2", "vec3",   "vec4",   "mat2",   "mat2x3",
                                                                 "mat2x4", "mat3x2", "mat3", "mat3x4", "mat4x2", "mat4x3", "mat4"};
    assert(ARRAY_SIZE(glslType) == KS_GPU_PROGRAM_PARM_TYPE_MAX);
    return glslType[type];
}

static void ksGpuProgramParmLayout_Create(ksGpuContext *context, ksGpuProgramParmLayout *layout, const ksGpuProgramParm *parms,
                                          const int numParms, const GLuint program) {
    UNUSED_PARM(context);
    assert(numParms <= MAX_PROGRAM_PARMS);

    memset(layout, 0, sizeof(ksGpuProgramParmLayout));

    layout->numParms = numParms;
    layout->parms = parms;

    int offset = 0;
    memset(layout->offsetForIndex, -1, sizeof(layout->offsetForIndex));

    // Get the texture/buffer/uniform locations and set bindings.
    for (int i = 0; i < numParms; i++) {
        if (parms[i].type == KS_GPU_PROGRAM_PARM_TYPE_TEXTURE_SAMPLED) {
            layout->parmLocations[i] = glGetUniformLocation(program, parms[i].name);
            assert(layout->parmLocations[i] != -1);
            if (layout->parmLocations[i] != -1) {
                // set "texture image unit" binding
                layout->parmBindings[i] = layout->numSampledTextureBindings++;
                GL(glProgramUniform1i(program, layout->parmLocations[i], layout->parmBindings[i]));
            }
        } else if (parms[i].type == KS_GPU_PROGRAM_PARM_TYPE_TEXTURE_STORAGE) {
            layout->parmLocations[i] = glGetUniformLocation(program, parms[i].name);
            assert(layout->parmLocations[i] != -1);
            if (layout->parmLocations[i] != -1) {
                // set "image unit" binding
                layout->parmBindings[i] = layout->numStorageTextureBindings++;
#if !defined(OS_ANDROID)
                // OpenGL ES does not support changing the location after linking, so rely on the layout( binding ) being correct.
                GL(glProgramUniform1i(program, layout->parmLocations[i], layout->parmBindings[i]));
#endif
            }
        } else if (parms[i].type == KS_GPU_PROGRAM_PARM_TYPE_BUFFER_UNIFORM) {
            layout->parmLocations[i] = glGetUniformBlockIndex(program, parms[i].name);
            assert(layout->parmLocations[i] != -1);
            if (layout->parmLocations[i] != -1) {
                // set "uniform block" binding
                layout->parmBindings[i] = layout->numUniformBufferBindings++;
                GL(glUniformBlockBinding(program, layout->parmLocations[i], layout->parmBindings[i]));
            }
        } else if (parms[i].type == KS_GPU_PROGRAM_PARM_TYPE_BUFFER_STORAGE) {
            layout->parmLocations[i] = glGetProgramResourceIndex(program, GL_SHADER_STORAGE_BLOCK, parms[i].name);
            assert(layout->parmLocations[i] != -1);
            if (layout->parmLocations[i] != -1) {
                // set "shader storage block" binding
                layout->parmBindings[i] = layout->numStorageBufferBindings++;
#if !defined(OS_ANDROID)
                // OpenGL ES does not support glShaderStorageBlockBinding, so rely on the layout( binding ) being correct.
                GL(glShaderStorageBlockBinding(program, layout->parmLocations[i], layout->parmBindings[i]));
#endif
            }
        } else {
            layout->parmLocations[i] = glGetUniformLocation(program, parms[i].name);
            assert(layout->parmLocations[i] != -1);  // The parm is not actually used in the shader?
            layout->parmBindings[i] = i;

            layout->offsetForIndex[parms[i].index] = offset;
            offset += ksGpuProgramParm_GetPushConstantSize(parms[i].type);
        }
    }

    assert(layout->numSampledTextureBindings <= glGetInteger(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS));
#if OPENGL_COMPUTE_ENABLED == 1
    assert(layout->numStorageTextureBindings <= glGetInteger(GL_MAX_IMAGE_UNITS));
    assert(layout->numUniformBufferBindings <= glGetInteger(GL_MAX_UNIFORM_BUFFER_BINDINGS));
    assert(layout->numStorageBufferBindings <= glGetInteger(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS));
#endif
}

static void ksGpuProgramParmLayout_Destroy(ksGpuContext *context, ksGpuProgramParmLayout *layout) {
    UNUSED_PARM(context);
    UNUSED_PARM(layout);
}

/*
================================================================================================================================

GPU graphics program.

A graphics program encapsulates a vertex and fragment program that are used to render geometry.
For optimal performance a graphics program should only be created at load time, not at runtime.

ksGpuGraphicsProgram

static bool ksGpuGraphicsProgram_Create( ksGpuContext * context, ksGpuGraphicsProgram * program,
                                                                                const void * vertexSourceData, const size_t
vertexSourceSize, const void * fragmentSourceData, const size_t fragmentSourceSize, const ksGpuProgramParm * parms, const int
numParms, const ksGpuVertexAttribute * vertexLayout, const int vertexAttribsFlags ); static void ksGpuGraphicsProgram_Destroy(
ksGpuContext * context, ksGpuGraphicsProgram * program );

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

static bool ksGpuGraphicsProgram_Create(ksGpuContext *context, ksGpuGraphicsProgram *program, const void *vertexSourceData,
                                        const size_t vertexSourceSize, const void *fragmentSourceData,
                                        const size_t fragmentSourceSize, const ksGpuProgramParm *parms, const int numParms,
                                        const ksGpuVertexAttribute *vertexLayout, const int vertexAttribsFlags) {
    UNUSED_PARM(vertexSourceSize);
    UNUSED_PARM(fragmentSourceSize);

    program->vertexAttribsFlags = vertexAttribsFlags;

    GLint r;
    GL(program->vertexShader = glCreateShader(GL_VERTEX_SHADER));
    GL(glShaderSource(program->vertexShader, 1, (const char **)&vertexSourceData, 0));
    GL(glCompileShader(program->vertexShader));
    GL(glGetShaderiv(program->vertexShader, GL_COMPILE_STATUS, &r));
    if (r == GL_FALSE) {
        GLchar msg[4096];
        GLsizei length;
        GL(glGetShaderInfoLog(program->vertexShader, sizeof(msg), &length, msg));
        Error("%s\nlength=%d\n%s\n", (const char *)vertexSourceData, length, msg);
        return false;
    }

    GL(program->fragmentShader = glCreateShader(GL_FRAGMENT_SHADER));
    GL(glShaderSource(program->fragmentShader, 1, (const char **)&fragmentSourceData, 0));
    GL(glCompileShader(program->fragmentShader));
    GL(glGetShaderiv(program->fragmentShader, GL_COMPILE_STATUS, &r));
    if (r == GL_FALSE) {
        GLchar msg[4096];
        GLsizei length;
        GL(glGetShaderInfoLog(program->fragmentShader, sizeof(msg), &length, msg));
        Error("%s\nlength=%d\n%s\n", (const char *)fragmentSourceData, length, msg);
        return false;
    }

    GL(program->program = glCreateProgram());
    GL(glAttachShader(program->program, program->vertexShader));
    GL(glAttachShader(program->program, program->fragmentShader));

    // Bind the vertex attribute locations before linking.
    GLuint location = 0;
    for (int i = 0; vertexLayout[i].attributeFlag != 0; i++) {
        if ((vertexLayout[i].attributeFlag & vertexAttribsFlags) != 0) {
            GL(glBindAttribLocation(program->program, location, vertexLayout[i].name));
            location += vertexLayout[i].locationCount;
        }
    }

    GL(glLinkProgram(program->program));
    GL(glGetProgramiv(program->program, GL_LINK_STATUS, &r));
    if (r == GL_FALSE) {
        GLchar msg[4096];
        GL(glGetProgramInfoLog(program->program, sizeof(msg), 0, msg));
        Error("Linking program failed: %s\n", msg);
        return false;
    }

    // Verify the attributes.
    for (int i = 0; vertexLayout[i].attributeFlag != 0; i++) {
        if ((vertexLayout[i].attributeFlag & vertexAttribsFlags) != 0) {
            assert(glGetAttribLocation(program->program, vertexLayout[i].name) != -1);
        }
    }

    ksGpuProgramParmLayout_Create(context, &program->parmLayout, parms, numParms, program->program);

    // Calculate a hash of the vertex and fragment program source.
    ksStringHash_Init(&program->hash);
    ksStringHash_Update(&program->hash, (const char *)vertexSourceData);
    ksStringHash_Update(&program->hash, (const char *)fragmentSourceData);

    return true;
}

static void ksGpuGraphicsProgram_Destroy(ksGpuContext *context, ksGpuGraphicsProgram *program) {
    ksGpuProgramParmLayout_Destroy(context, &program->parmLayout);
    if (program->program != 0) {
        GL(glDeleteProgram(program->program));
        program->program = 0;
    }
    if (program->vertexShader != 0) {
        GL(glDeleteShader(program->vertexShader));
        program->vertexShader = 0;
    }
    if (program->fragmentShader != 0) {
        GL(glDeleteShader(program->fragmentShader));
        program->fragmentShader = 0;
    }
}

/*
================================================================================================================================

GPU compute program.

For optimal performance a compute program should only be created at load time, not at runtime.

ksGpuComputeProgram

static bool ksGpuComputeProgram_Create( ksGpuContext * context, ksGpuComputeProgram * program,
                                                                                const void * computeSourceData, const size_t
computeSourceSize, const ksGpuProgramParm * parms, const int numParms ); static void ksGpuComputeProgram_Destroy( ksGpuContext *
context, ksGpuComputeProgram * program );

================================================================================================================================
*/

typedef struct {
    GLuint computeShader;
    GLuint program;
    ksGpuProgramParmLayout parmLayout;
    ksStringHash hash;
} ksGpuComputeProgram;

static bool ksGpuComputeProgram_Create(ksGpuContext *context, ksGpuComputeProgram *program, const void *computeSourceData,
                                       const size_t computeSourceSize, const ksGpuProgramParm *parms, const int numParms) {
    UNUSED_PARM(context);
    UNUSED_PARM(computeSourceSize);

    GLint r;
    GL(program->computeShader = glCreateShader(GL_COMPUTE_SHADER));
    GL(glShaderSource(program->computeShader, 1, (const char **)&computeSourceData, 0));
    GL(glCompileShader(program->computeShader));
    GL(glGetShaderiv(program->computeShader, GL_COMPILE_STATUS, &r));
    if (r == GL_FALSE) {
        GLchar msg[4096];
        GL(glGetShaderInfoLog(program->computeShader, sizeof(msg), 0, msg));
        Error("%s\n%s\n", (const char *)computeSourceData, msg);
        return false;
    }

    GL(program->program = glCreateProgram());
    GL(glAttachShader(program->program, program->computeShader));

    GL(glLinkProgram(program->program));
    GL(glGetProgramiv(program->program, GL_LINK_STATUS, &r));
    if (r == GL_FALSE) {
        GLchar msg[4096];
        GL(glGetProgramInfoLog(program->program, sizeof(msg), 0, msg));
        Error("Linking program failed: %s\n", msg);
        return false;
    }

    ksGpuProgramParmLayout_Create(context, &program->parmLayout, parms, numParms, program->program);

    // Calculate a hash of the shader source.
    ksStringHash_Init(&program->hash);
    ksStringHash_Update(&program->hash, (const char *)computeSourceData);

    return true;
}

static void ksGpuComputeProgram_Destroy(ksGpuContext *context, ksGpuComputeProgram *program) {
    ksGpuProgramParmLayout_Destroy(context, &program->parmLayout);

    if (program->program != 0) {
        GL(glDeleteProgram(program->program));
        program->program = 0;
    }
    if (program->computeShader != 0) {
        GL(glDeleteShader(program->computeShader));
        program->computeShader = 0;
    }
}

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

static bool ksGpuGraphicsPipeline_Create( ksGpuContext * context, ksGpuGraphicsPipeline * pipeline, const ksGpuGraphicsPipelineParms
* parms ); static void ksGpuGraphicsPipeline_Destroy( ksGpuContext * context, ksGpuGraphicsPipeline * pipeline );

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

static void ksGpuGraphicsPipelineParms_Init(ksGpuGraphicsPipelineParms *parms) {
    parms->rop.blendEnable = false;
    parms->rop.redWriteEnable = true;
    parms->rop.blueWriteEnable = true;
    parms->rop.greenWriteEnable = true;
    parms->rop.alphaWriteEnable = false;
    parms->rop.depthTestEnable = true;
    parms->rop.depthWriteEnable = true;
    parms->rop.frontFace = KS_GPU_FRONT_FACE_COUNTER_CLOCKWISE;
    parms->rop.cullMode = KS_GPU_CULL_MODE_BACK;
    parms->rop.depthCompare = KS_GPU_COMPARE_OP_LESS_OR_EQUAL;
    parms->rop.blendColor.x = 0.0f;
    parms->rop.blendColor.y = 0.0f;
    parms->rop.blendColor.z = 0.0f;
    parms->rop.blendColor.w = 0.0f;
    parms->rop.blendOpColor = KS_GPU_BLEND_OP_ADD;
    parms->rop.blendSrcColor = KS_GPU_BLEND_FACTOR_ONE;
    parms->rop.blendDstColor = KS_GPU_BLEND_FACTOR_ZERO;
    parms->rop.blendOpAlpha = KS_GPU_BLEND_OP_ADD;
    parms->rop.blendSrcAlpha = KS_GPU_BLEND_FACTOR_ONE;
    parms->rop.blendDstAlpha = KS_GPU_BLEND_FACTOR_ZERO;
    parms->renderPass = NULL;
    parms->program = NULL;
    parms->geometry = NULL;
}

static void InitVertexAttributes(const bool instance, const ksGpuVertexAttribute *vertexLayout, const int numAttribs,
                                 const int storedAttribsFlags, const int usedAttribsFlags, GLuint *attribLocationCount) {
    size_t offset = 0;
    for (int i = 0; vertexLayout[i].attributeFlag != 0; i++) {
        const ksGpuVertexAttribute *v = &vertexLayout[i];
        if ((v->attributeFlag & storedAttribsFlags) != 0) {
            if ((v->attributeFlag & usedAttribsFlags) != 0) {
                const size_t attribLocationSize = v->attributeSize / v->locationCount;
                const size_t attribStride = v->attributeSize;
                for (int location = 0; location < v->locationCount; location++) {
                    GL(glEnableVertexAttribArray(*attribLocationCount + location));
                    GL(glVertexAttribPointer(*attribLocationCount + location, v->attributeFormat >> 16, v->attributeFormat & 0xFFFF,
                                             GL_FALSE, (GLsizei)attribStride, (void *)(offset + location * attribLocationSize)));
                    GL(glVertexAttribDivisor(*attribLocationCount + location, instance ? 1 : 0));
                }
                *attribLocationCount += v->locationCount;
            }
            offset += numAttribs * v->attributeSize;
        }
    }
}

static bool ksGpuGraphicsPipeline_Create(ksGpuContext *context, ksGpuGraphicsPipeline *pipeline,
                                         const ksGpuGraphicsPipelineParms *parms) {
    UNUSED_PARM(context);

    // Make sure the geometry provides all the attributes needed by the program.
    assert(((parms->geometry->vertexAttribsFlags | parms->geometry->instanceAttribsFlags) & parms->program->vertexAttribsFlags) ==
           parms->program->vertexAttribsFlags);

    memset(pipeline, 0, sizeof(ksGpuGraphicsPipeline));

    pipeline->rop = parms->rop;
    pipeline->program = parms->program;
    pipeline->geometry = parms->geometry;

    assert(pipeline->vertexArrayObject == 0);

    GL(glGenVertexArrays(1, &pipeline->vertexArrayObject));
    GL(glBindVertexArray(pipeline->vertexArrayObject));

    GLuint attribLocationCount = 0;

    GL(glBindBuffer(parms->geometry->vertexBuffer.target, parms->geometry->vertexBuffer.buffer));
    InitVertexAttributes(false, parms->geometry->layout, parms->geometry->vertexCount, parms->geometry->vertexAttribsFlags,
                         parms->program->vertexAttribsFlags, &attribLocationCount);

    if (parms->geometry->instanceBuffer.buffer != 0) {
        GL(glBindBuffer(parms->geometry->instanceBuffer.target, parms->geometry->instanceBuffer.buffer));
        InitVertexAttributes(true, parms->geometry->layout, parms->geometry->instanceCount, parms->geometry->instanceAttribsFlags,
                             parms->program->vertexAttribsFlags, &attribLocationCount);
    }

    GL(glBindBuffer(parms->geometry->indexBuffer.target, parms->geometry->indexBuffer.buffer));

    GL(glBindVertexArray(0));

    return true;
}

static void ksGpuGraphicsPipeline_Destroy(ksGpuContext *context, ksGpuGraphicsPipeline *pipeline) {
    UNUSED_PARM(context);

    if (pipeline->vertexArrayObject != 0) {
        GL(glDeleteVertexArrays(1, &pipeline->vertexArrayObject));
        pipeline->vertexArrayObject = 0;
    }
}

/*
================================================================================================================================

GPU compute pipeline.

A compute pipeline encapsulates a compute program.
For optimal performance a compute pipeline should only be created at load time, not at runtime.

ksGpuComputePipeline

static bool ksGpuComputePipeline_Create( ksGpuContext * context, ksGpuComputePipeline * pipeline, const ksGpuComputeProgram *
program ); static void ksGpuComputePipeline_Destroy( ksGpuContext * context, ksGpuComputePipeline * pipeline );

================================================================================================================================
*/

typedef struct {
    const ksGpuComputeProgram *program;
} ksGpuComputePipeline;

static bool ksGpuComputePipeline_Create(ksGpuContext *context, ksGpuComputePipeline *pipeline, const ksGpuComputeProgram *program) {
    UNUSED_PARM(context);

    pipeline->program = program;

    return true;
}

static void ksGpuComputePipeline_Destroy(ksGpuContext *context, ksGpuComputePipeline *pipeline) {
    UNUSED_PARM(context);
    UNUSED_PARM(pipeline);
}

/*
================================================================================================================================

GPU fence.

A fence is used to notify completion of a command buffer.
For optimal performance a fence should only be created at load time, not at runtime.

ksGpuFence

static void ksGpuFence_Create( ksGpuContext * context, ksGpuFence * fence );
static void ksGpuFence_Destroy( ksGpuContext * context, ksGpuFence * fence );
static void ksGpuFence_Submit( ksGpuContext * context, ksGpuFence * fence );
static void ksGpuFence_IsSignalled( ksGpuContext * context, ksGpuFence * fence );

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

static void ksGpuFence_Create(ksGpuContext *context, ksGpuFence *fence) {
    UNUSED_PARM(context);

#if USE_SYNC_OBJECT == 0
    fence->sync = 0;
#elif USE_SYNC_OBJECT == 1
    fence->display = 0;
    fence->sync = EGL_NO_SYNC_KHR;
#elif USE_SYNC_OBJECT == 2
    static const char syncComputeProgramGLSL[] = "#version " GLSL_VERSION
                                                 "\n"
                                                 "\n"
                                                 "layout( local_size_x = 1, local_size_y = 1 ) in;\n"
                                                 "\n"
                                                 "layout( std430, binding = 0 ) buffer syncBuffer { int sync[]; };\n"
                                                 "\n"
                                                 "void main()\n"
                                                 "{\n"
                                                 "    sync[0] = 0;\n"
                                                 "}\n";
    const char *source = syncComputeProgramGLSL;
    // Create a compute program to write to a storage buffer.
    GLint r;
    GL(fence->computeShader = glCreateShader(GL_COMPUTE_SHADER));
    GL(glShaderSource(fence->computeShader, 1, (const char **)&source, 0));
    GL(glCompileShader(fence->computeShader));
    GL(glGetShaderiv(fence->computeShader, GL_COMPILE_STATUS, &r));
    assert(r != GL_FALSE);
    GL(fence->computeProgram = glCreateProgram());
    GL(glAttachShader(fence->computeProgram, fence->computeShader));
    GL(glLinkProgram(fence->computeProgram));
    GL(glGetProgramiv(fence->computeProgram, GL_LINK_STATUS, &r));
    assert(r != GL_FALSE);
    // Create the persistently mapped storage buffer.
    GL(glGenBuffers(1, &fence->storageBuffer));
    GL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, fence->storageBuffer));
    GL(glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint), NULL,
                       GL_DYNAMIC_STORAGE_BIT | GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT));
    GL(fence->mappedBuffer =
           (GLuint *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint),
                                      GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT));
    GL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
#else
#error "invalid USE_SYNC_OBJECT setting"
#endif
}

static void ksGpuFence_Destroy(ksGpuContext *context, ksGpuFence *fence) {
    UNUSED_PARM(context);

#if USE_SYNC_OBJECT == 0
    if (fence->sync != 0) {
        GL(glDeleteSync(fence->sync));
        fence->sync = 0;
    }
#elif USE_SYNC_OBJECT == 1
    if (fence->sync != EGL_NO_SYNC_KHR) {
        EGL(eglDestroySyncKHR(fence->display, fence->sync));
        fence->display = 0;
        fence->sync = EGL_NO_SYNC_KHR;
    }
#elif USE_SYNC_OBJECT == 2
    GL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, fence->storageBuffer));
    GL(glUnmapBuffer(GL_SHADER_STORAGE_BUFFER));
    GL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));

    GL(glDeleteBuffers(1, &fence->storageBuffer));
    GL(glDeleteProgram(fence->computeProgram));
    GL(glDeleteShader(fence->computeShader));
#else
#error "invalid USE_SYNC_OBJECT setting"
#endif
}

static void ksGpuFence_Submit(ksGpuContext *context, ksGpuFence *fence) {
    UNUSED_PARM(context);

#if USE_SYNC_OBJECT == 0
    // Destroy any old sync object.
    if (fence->sync != 0) {
        GL(glDeleteSync(fence->sync));
        fence->sync = 0;
    }
    // Create and insert a new sync object.
    GL(fence->sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0));
    // Force flushing the commands.
    // Note that some drivers will already flush when calling glFenceSync.
    GL(glClientWaitSync(fence->sync, GL_SYNC_FLUSH_COMMANDS_BIT, 0));
#elif USE_SYNC_OBJECT == 1
    // Destroy any old sync object.
    if (fence->sync != EGL_NO_SYNC_KHR) {
        EGL(eglDestroySyncKHR(fence->display, fence->sync));
        fence->display = 0;
        fence->sync = EGL_NO_SYNC_KHR;
    }
    // Create and insert a new sync object.
    fence->display = eglGetCurrentDisplay();
    fence->sync = eglCreateSyncKHR(fence->display, EGL_SYNC_FENCE_KHR, NULL);
    if (fence->sync == EGL_NO_SYNC_KHR) {
        Error("eglCreateSyncKHR() : EGL_NO_SYNC_KHR");
    }
    // Force flushing the commands.
    // Note that some drivers will already flush when calling eglCreateSyncKHR.
    EGL(eglClientWaitSyncKHR(fence->display, fence->sync, EGL_SYNC_FLUSH_COMMANDS_BIT_KHR, 0));
#elif USE_SYNC_OBJECT == 2
    // Initialize the storage buffer to 1 on the client side.
    fence->mappedBuffer[0] = 1;
    // Use a compute shader to clear the persistently mapped storage buffer.
    // This relies on a compute shader only being executed after all other work has completed.
    GL(glUseProgram(fence->computeProgram));
    GL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, fence->storageBuffer));
    GL(glDispatchCompute(1, 1, 1));
    GL(glUseProgram(0));
    // Flush the commands.
    // Note that some drivers may ignore a flush which could result in the storage buffer never being updated.
    GL(glFlush());
#else
#error "invalid USE_SYNC_OBJECT setting"
#endif
}

static bool ksGpuFence_IsSignalled(ksGpuContext *context, ksGpuFence *fence) {
    UNUSED_PARM(context);

    if (fence == NULL) {
        return false;
    }
#if USE_SYNC_OBJECT == 0
    if (glIsSync(fence->sync)) {
        GL(const GLenum result = glClientWaitSync(fence->sync, 0, 0));
        if (result == GL_WAIT_FAILED) {
            Error("glClientWaitSync() : GL_WAIT_FAILED");
        }
        if (result != GL_TIMEOUT_EXPIRED) {
            return true;
        }
    }
#elif USE_SYNC_OBJECT == 1
    if (fence->sync != EGL_NO_SYNC_KHR) {
        const EGLint result = eglClientWaitSyncKHR(fence->display, fence->sync, 0, 0);
        if (result == EGL_FALSE) {
            Error("eglClientWaitSyncKHR() : EGL_FALSE");
        }
        if (result != EGL_TIMEOUT_EXPIRED_KHR) {
            return true;
        }
    }
#elif USE_SYNC_OBJECT == 2
    if (fence->mappedBuffer[0] == 0) {
        return true;
    }
#else
#error "invalid USE_SYNC_OBJECT setting"
#endif
    return false;
}

/*
================================================================================================================================

GPU timer.

A timer is used to measure the amount of time it takes to complete GPU commands.
For optimal performance a timer should only be created at load time, not at runtime.
To avoid synchronization, ksGpuTimer_GetNanoseconds() reports the time from KS_GPU_TIMER_FRAMES_DELAYED frames ago.
Timer queries are allowed to overlap and can be nested.
Timer queries that are issued inside a render pass may not produce accurate times on tiling GPUs.

ksGpuTimer

static void ksGpuTimer_Create( ksGpuContext * context, ksGpuTimer * timer );
static void ksGpuTimer_Destroy( ksGpuContext * context, ksGpuTimer * timer );
static ksNanoseconds ksGpuTimer_GetNanoseconds( ksGpuTimer * timer );

================================================================================================================================
*/

#define KS_GPU_TIMER_FRAMES_DELAYED 2

typedef struct {
    GLuint beginQueries[KS_GPU_TIMER_FRAMES_DELAYED];
    GLuint endQueries[KS_GPU_TIMER_FRAMES_DELAYED];
    int queryIndex;
    ksNanoseconds gpuTime;
} ksGpuTimer;

static void ksGpuTimer_Create(ksGpuContext *context, ksGpuTimer *timer) {
    UNUSED_PARM(context);

    if (glExtensions.timer_query) {
        GL(glGenQueries(KS_GPU_TIMER_FRAMES_DELAYED, timer->beginQueries));
        GL(glGenQueries(KS_GPU_TIMER_FRAMES_DELAYED, timer->endQueries));
        timer->queryIndex = 0;
        timer->gpuTime = 0;
    }
}

static void ksGpuTimer_Destroy(ksGpuContext *context, ksGpuTimer *timer) {
    UNUSED_PARM(context);

    if (glExtensions.timer_query) {
        GL(glDeleteQueries(KS_GPU_TIMER_FRAMES_DELAYED, timer->beginQueries));
        GL(glDeleteQueries(KS_GPU_TIMER_FRAMES_DELAYED, timer->endQueries));
    }
}

static ksNanoseconds ksGpuTimer_GetNanoseconds(ksGpuTimer *timer) {
    if (glExtensions.timer_query) {
        return timer->gpuTime;
    } else {
        return 0;
    }
}

/*
================================================================================================================================

GPU program parm state.

ksGpuProgramParmState

================================================================================================================================
*/

#define SAVE_PUSH_CONSTANT_STATE 1
#define MAX_SAVED_PUSH_CONSTANT_BYTES 512

typedef struct {
    const void *parms[MAX_PROGRAM_PARMS];
#if SAVE_PUSH_CONSTANT_STATE == 1
    unsigned char data[MAX_SAVED_PUSH_CONSTANT_BYTES];
#endif
} ksGpuProgramParmState;

static void ksGpuProgramParmState_SetParm(ksGpuProgramParmState *parmState, const ksGpuProgramParmLayout *parmLayout,
                                          const int index, const ksGpuProgramParmType parmType, const void *pointer) {
    assert(index >= 0 && index < MAX_PROGRAM_PARMS);
    if (pointer != NULL) {
        bool found = false;
        for (int i = 0; i < parmLayout->numParms; i++) {
            if (parmLayout->parms[i].index == index) {
                assert(parmLayout->parms[i].type == parmType);
                found = true;
                break;
            }
        }
        // Currently parms can be set even if they are not used by the program.
        // assert( found );
        UNUSED_PARM(found);
    }

    parmState->parms[index] = pointer;

#if SAVE_PUSH_CONSTANT_STATE == 1
    const int pushConstantSize = ksGpuProgramParm_GetPushConstantSize(parmType);
    if (pushConstantSize > 0) {
        assert(parmLayout->offsetForIndex[index] >= 0);
        assert(parmLayout->offsetForIndex[index] + pushConstantSize <= MAX_SAVED_PUSH_CONSTANT_BYTES);
        memcpy(&parmState->data[parmLayout->offsetForIndex[index]], pointer, pushConstantSize);
    }
#endif
}

static const void *ksGpuProgramParmState_NewPushConstantData(const ksGpuProgramParmLayout *newLayout, const int newParmIndex,
                                                             const ksGpuProgramParmState *newParmState,
                                                             const ksGpuProgramParmLayout *oldLayout, const int oldParmIndex,
                                                             const ksGpuProgramParmState *oldParmState, const bool force) {
#if SAVE_PUSH_CONSTANT_STATE == 1
    const ksGpuProgramParm *newParm = &newLayout->parms[newParmIndex];
    const unsigned char *newData = &newParmState->data[newLayout->offsetForIndex[newParm->index]];
    if (force || oldLayout == NULL || oldParmIndex >= oldLayout->numParms) {
        return newData;
    }
    const ksGpuProgramParm *oldParm = &oldLayout->parms[oldParmIndex];
    const unsigned char *oldData = &oldParmState->data[oldLayout->offsetForIndex[oldParm->index]];
    if (newParm->type != oldParm->type || newLayout->parmBindings[newParmIndex] != oldLayout->parmBindings[oldParmIndex]) {
        return newData;
    }
    const int pushConstantSize = ksGpuProgramParm_GetPushConstantSize(newParm->type);
    if (memcmp(newData, oldData, pushConstantSize) != 0) {
        return newData;
    }
    return NULL;
#else
    if (force || oldLayout == NULL || oldParmIndex >= oldLayout->numParms ||
        newLayout->parmBindings[newParmIndex] != oldLayout->parmBindings[oldParmIndex] ||
        newLayout->parms[newParmIndex].type != oldLayout->parms[oldParmIndex].type ||
        newParmState->parms[newLayout->parms[newParmIndex].index] != oldParmState->parms[oldLayout->parms[oldParmIndex].index]) {
        return newParmState->parms[newLayout->parms[newParmIndex].index];
    }
    return NULL;
#endif
}

/*
================================================================================================================================

GPU graphics commands.

A graphics command encapsulates all GPU state associated with a single draw call.
The pointers passed in as parameters are expected to point to unique objects that persist
at least past the submission of the command buffer into which the graphics command is
submitted. Because pointers are maintained as state, DO NOT use pointers to local
variables that will go out of scope before the command buffer is submitted.

ksGpuGraphicsCommand

static void ksGpuGraphicsCommand_Init( ksGpuGraphicsCommand * command );
static void ksGpuGraphicsCommand_SetPipeline( ksGpuGraphicsCommand * command, const ksGpuGraphicsPipeline * pipeline );
static void ksGpuGraphicsCommand_SetVertexBuffer( ksGpuGraphicsCommand * command, const ksGpuBuffer * vertexBuffer );
static void ksGpuGraphicsCommand_SetInstanceBuffer( ksGpuGraphicsCommand * command, const ksGpuBuffer * instanceBuffer );
static void ksGpuGraphicsCommand_SetParmTextureSampled( ksGpuGraphicsCommand * command, const int index, const ksGpuTexture *
texture ); static void ksGpuGraphicsCommand_SetParmTextureStorage( ksGpuGraphicsCommand * command, const int index, const
ksGpuTexture * texture ); static void ksGpuGraphicsCommand_SetParmBufferUniform( ksGpuGraphicsCommand * command, const int index,
const ksGpuBuffer * buffer ); static void ksGpuGraphicsCommand_SetParmBufferStorage( ksGpuGraphicsCommand * command, const int
index, const ksGpuBuffer * buffer ); static void ksGpuGraphicsCommand_SetParmInt( ksGpuGraphicsCommand * command, const int index,
const int * value ); static void ksGpuGraphicsCommand_SetParmIntVector2( ksGpuGraphicsCommand * command, const int index, const
ksVector2i * value ); static void ksGpuGraphicsCommand_SetParmIntVector3( ksGpuGraphicsCommand * command, const int index, const
ksVector3i * value ); static void ksGpuGraphicsCommand_SetParmIntVector4( ksGpuGraphicsCommand * command, const int index, const
ksVector4i * value ); static void ksGpuGraphicsCommand_SetParmFloat( ksGpuGraphicsCommand * command, const int index, const float *
value ); static void ksGpuGraphicsCommand_SetParmFloatVector2( ksGpuGraphicsCommand * command, const int index, const ksVector2f *
value ); static void ksGpuGraphicsCommand_SetParmFloatVector3( ksGpuGraphicsCommand * command, const int index, const ksVector3f *
value ); static void ksGpuGraphicsCommand_SetParmFloatVector4( ksGpuGraphicsCommand * command, const int index, const ksVector3f *
value ); static void ksGpuGraphicsCommand_SetParmFloatMatrix2x2( ksGpuGraphicsCommand * command, const int index, const ksMatrix2x2f
* value ); static void ksGpuGraphicsCommand_SetParmFloatMatrix2x3( ksGpuGraphicsCommand * command, const int index, const
ksMatrix2x3f * value ); static void ksGpuGraphicsCommand_SetParmFloatMatrix2x4( ksGpuGraphicsCommand * command, const int index,
const ksMatrix2x4f * value ); static void ksGpuGraphicsCommand_SetParmFloatMatrix3x2( ksGpuGraphicsCommand * command, const int
index, const ksMatrix3x2f * value ); static void ksGpuGraphicsCommand_SetParmFloatMatrix3x3( ksGpuGraphicsCommand * command, const
int index, const ksMatrix3x3f * value ); static void ksGpuGraphicsCommand_SetParmFloatMatrix3x4( ksGpuGraphicsCommand * command,
const int index, const ksMatrix3x4f * value ); static void ksGpuGraphicsCommand_SetParmFloatMatrix4x2( ksGpuGraphicsCommand *
command, const int index, const ksMatrix4x2f * value ); static void ksGpuGraphicsCommand_SetParmFloatMatrix4x3( ksGpuGraphicsCommand
* command, const int index, const ksMatrix4x3f * value ); static void ksGpuGraphicsCommand_SetParmFloatMatrix4x4(
ksGpuGraphicsCommand * command, const int index, const ksMatrix4x4f * value ); static void ksGpuGraphicsCommand_SetNumInstances(
ksGpuGraphicsCommand * command, const int numInstances );

================================================================================================================================
*/

typedef struct {
    const ksGpuGraphicsPipeline *pipeline;
    const ksGpuBuffer *vertexBuffer;    // vertex buffer returned by ksGpuCommandBuffer_MapVertexAttributes
    const ksGpuBuffer *instanceBuffer;  // instance buffer returned by ksGpuCommandBuffer_MapInstanceAttributes
    ksGpuProgramParmState parmState;
    int numInstances;
} ksGpuGraphicsCommand;

static void ksGpuGraphicsCommand_Init(ksGpuGraphicsCommand *command) {
    command->pipeline = NULL;
    command->vertexBuffer = NULL;
    command->instanceBuffer = NULL;
    memset((void *)&command->parmState, 0, sizeof(command->parmState));
    command->numInstances = 1;
}

static void ksGpuGraphicsCommand_SetPipeline(ksGpuGraphicsCommand *command, const ksGpuGraphicsPipeline *pipeline) {
    command->pipeline = pipeline;
}

static void ksGpuGraphicsCommand_SetVertexBuffer(ksGpuGraphicsCommand *command, const ksGpuBuffer *vertexBuffer) {
    command->vertexBuffer = vertexBuffer;
}

static void ksGpuGraphicsCommand_SetInstanceBuffer(ksGpuGraphicsCommand *command, const ksGpuBuffer *instanceBuffer) {
    command->instanceBuffer = instanceBuffer;
}

static void ksGpuGraphicsCommand_SetParmTextureSampled(ksGpuGraphicsCommand *command, const int index,
                                                       const ksGpuTexture *texture) {
    ksGpuProgramParmState_SetParm(&command->parmState, &command->pipeline->program->parmLayout, index,
                                  KS_GPU_PROGRAM_PARM_TYPE_TEXTURE_SAMPLED, texture);
}

static void ksGpuGraphicsCommand_SetParmTextureStorage(ksGpuGraphicsCommand *command, const int index,
                                                       const ksGpuTexture *texture) {
    ksGpuProgramParmState_SetParm(&command->parmState, &command->pipeline->program->parmLayout, index,
                                  KS_GPU_PROGRAM_PARM_TYPE_TEXTURE_STORAGE, texture);
}

static void ksGpuGraphicsCommand_SetParmBufferUniform(ksGpuGraphicsCommand *command, const int index, const ksGpuBuffer *buffer) {
    ksGpuProgramParmState_SetParm(&command->parmState, &command->pipeline->program->parmLayout, index,
                                  KS_GPU_PROGRAM_PARM_TYPE_BUFFER_UNIFORM, buffer);
}

static void ksGpuGraphicsCommand_SetParmBufferStorage(ksGpuGraphicsCommand *command, const int index, const ksGpuBuffer *buffer) {
    ksGpuProgramParmState_SetParm(&command->parmState, &command->pipeline->program->parmLayout, index,
                                  KS_GPU_PROGRAM_PARM_TYPE_BUFFER_STORAGE, buffer);
}

static void ksGpuGraphicsCommand_SetParmInt(ksGpuGraphicsCommand *command, const int index, const int *value) {
    ksGpuProgramParmState_SetParm(&command->parmState, &command->pipeline->program->parmLayout, index,
                                  KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_INT, value);
}

static void ksGpuGraphicsCommand_SetParmIntVector2(ksGpuGraphicsCommand *command, const int index, const ksVector2i *value) {
    ksGpuProgramParmState_SetParm(&command->parmState, &command->pipeline->program->parmLayout, index,
                                  KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_INT_VECTOR2, value);
}

static void ksGpuGraphicsCommand_SetParmIntVector3(ksGpuGraphicsCommand *command, const int index, const ksVector3i *value) {
    ksGpuProgramParmState_SetParm(&command->parmState, &command->pipeline->program->parmLayout, index,
                                  KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_INT_VECTOR3, value);
}

static void ksGpuGraphicsCommand_SetParmIntVector4(ksGpuGraphicsCommand *command, const int index, const ksVector4i *value) {
    ksGpuProgramParmState_SetParm(&command->parmState, &command->pipeline->program->parmLayout, index,
                                  KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_INT_VECTOR4, value);
}

static void ksGpuGraphicsCommand_SetParmFloat(ksGpuGraphicsCommand *command, const int index, const float *value) {
    ksGpuProgramParmState_SetParm(&command->parmState, &command->pipeline->program->parmLayout, index,
                                  KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT, value);
}

static void ksGpuGraphicsCommand_SetParmFloatVector2(ksGpuGraphicsCommand *command, const int index, const ksVector2f *value) {
    ksGpuProgramParmState_SetParm(&command->parmState, &command->pipeline->program->parmLayout, index,
                                  KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_VECTOR2, value);
}

static void ksGpuGraphicsCommand_SetParmFloatVector3(ksGpuGraphicsCommand *command, const int index, const ksVector3f *value) {
    ksGpuProgramParmState_SetParm(&command->parmState, &command->pipeline->program->parmLayout, index,
                                  KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_VECTOR3, value);
}

static void ksGpuGraphicsCommand_SetParmFloatVector4(ksGpuGraphicsCommand *command, const int index, const ksVector4f *value) {
    ksGpuProgramParmState_SetParm(&command->parmState, &command->pipeline->program->parmLayout, index,
                                  KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_VECTOR4, value);
}

static void ksGpuGraphicsCommand_SetParmFloatMatrix2x2(ksGpuGraphicsCommand *command, const int index, const ksMatrix2x2f *value) {
    ksGpuProgramParmState_SetParm(&command->parmState, &command->pipeline->program->parmLayout, index,
                                  KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_MATRIX2X2, value);
}

static void ksGpuGraphicsCommand_SetParmFloatMatrix2x3(ksGpuGraphicsCommand *command, const int index, const ksMatrix2x3f *value) {
    ksGpuProgramParmState_SetParm(&command->parmState, &command->pipeline->program->parmLayout, index,
                                  KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_MATRIX2X3, value);
}

static void ksGpuGraphicsCommand_SetParmFloatMatrix2x4(ksGpuGraphicsCommand *command, const int index, const ksMatrix2x4f *value) {
    ksGpuProgramParmState_SetParm(&command->parmState, &command->pipeline->program->parmLayout, index,
                                  KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_MATRIX2X4, value);
}

static void ksGpuGraphicsCommand_SetParmFloatMatrix3x2(ksGpuGraphicsCommand *command, const int index, const ksMatrix3x2f *value) {
    ksGpuProgramParmState_SetParm(&command->parmState, &command->pipeline->program->parmLayout, index,
                                  KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_MATRIX3X2, value);
}

static void ksGpuGraphicsCommand_SetParmFloatMatrix3x3(ksGpuGraphicsCommand *command, const int index, const ksMatrix3x3f *value) {
    ksGpuProgramParmState_SetParm(&command->parmState, &command->pipeline->program->parmLayout, index,
                                  KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_MATRIX3X3, value);
}

static void ksGpuGraphicsCommand_SetParmFloatMatrix3x4(ksGpuGraphicsCommand *command, const int index, const ksMatrix3x4f *value) {
    ksGpuProgramParmState_SetParm(&command->parmState, &command->pipeline->program->parmLayout, index,
                                  KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_MATRIX3X4, value);
}

static void ksGpuGraphicsCommand_SetParmFloatMatrix4x2(ksGpuGraphicsCommand *command, const int index, const ksMatrix4x2f *value) {
    ksGpuProgramParmState_SetParm(&command->parmState, &command->pipeline->program->parmLayout, index,
                                  KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_MATRIX4X2, value);
}

static void ksGpuGraphicsCommand_SetParmFloatMatrix4x3(ksGpuGraphicsCommand *command, const int index, const ksMatrix4x3f *value) {
    ksGpuProgramParmState_SetParm(&command->parmState, &command->pipeline->program->parmLayout, index,
                                  KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_MATRIX4X3, value);
}

static void ksGpuGraphicsCommand_SetParmFloatMatrix4x4(ksGpuGraphicsCommand *command, const int index, const ksMatrix4x4f *value) {
    ksGpuProgramParmState_SetParm(&command->parmState, &command->pipeline->program->parmLayout, index,
                                  KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_MATRIX4X4, value);
}

static void ksGpuGraphicsCommand_SetNumInstances(ksGpuGraphicsCommand *command, const int numInstances) {
    command->numInstances = numInstances;
}

/*
================================================================================================================================

GPU compute commands.

A compute command encapsulates all GPU state associated with a single dispatch.
The pointers passed in as parameters are expected to point to unique objects that persist
at least past the submission of the command buffer into which the compute command is
submitted. Because various pointer are maintained as state, DO NOT use pointers to local
variables that will go out of scope before the command buffer is submitted.

ksGpuComputeCommand

static void ksGpuComputeCommand_Init( ksGpuComputeCommand * command );
static void ksGpuComputeCommand_SetPipeline( ksGpuComputeCommand * command, const ksGpuComputePipeline * pipeline );
static void ksGpuComputeCommand_SetParmTextureSampled( ksGpuComputeCommand * command, const int index, const ksGpuTexture * texture
); static void ksGpuComputeCommand_SetParmTextureStorage( ksGpuComputeCommand * command, const int index, const ksGpuTexture *
texture ); static void ksGpuComputeCommand_SetParmBufferUniform( ksGpuComputeCommand * command, const int index, const ksGpuBuffer *
buffer ); static void ksGpuComputeCommand_SetParmBufferStorage( ksGpuComputeCommand * command, const int index, const ksGpuBuffer *
buffer ); static void ksGpuComputeCommand_SetParmInt( ksGpuComputeCommand * command, const int index, const int * value ); static
void ksGpuComputeCommand_SetParmIntVector2( ksGpuComputeCommand * command, const int index, const ksVector2i * value ); static void
ksGpuComputeCommand_SetParmIntVector3( ksGpuComputeCommand * command, const int index, const ksVector3i * value ); static void
ksGpuComputeCommand_SetParmIntVector4( ksGpuComputeCommand * command, const int index, const ksVector4i * value ); static void
ksGpuComputeCommand_SetParmFloat( ksGpuComputeCommand * command, const int index, const float * value ); static void
ksGpuComputeCommand_SetParmFloatVector2( ksGpuComputeCommand * command, const int index, const ksVector2f * value ); static void
ksGpuComputeCommand_SetParmFloatVector3( ksGpuComputeCommand * command, const int index, const ksVector3f * value ); static void
ksGpuComputeCommand_SetParmFloatVector4( ksGpuComputeCommand * command, const int index, const ksVector3f * value ); static void
ksGpuComputeCommand_SetParmFloatMatrix2x2( ksGpuComputeCommand * command, const int index, const ksMatrix2x2f * value ); static void
ksGpuComputeCommand_SetParmFloatMatrix2x3( ksGpuComputeCommand * command, const int index, const ksMatrix2x3f * value ); static void
ksGpuComputeCommand_SetParmFloatMatrix2x4( ksGpuComputeCommand * command, const int index, const ksMatrix2x4f * value ); static void
ksGpuComputeCommand_SetParmFloatMatrix3x2( ksGpuComputeCommand * command, const int index, const ksMatrix3x2f * value ); static void
ksGpuComputeCommand_SetParmFloatMatrix3x3( ksGpuComputeCommand * command, const int index, const ksMatrix3x3f * value ); static void
ksGpuComputeCommand_SetParmFloatMatrix3x4( ksGpuComputeCommand * command, const int index, const ksMatrix3x4f * value ); static void
ksGpuComputeCommand_SetParmFloatMatrix4x2( ksGpuComputeCommand * command, const int index, const ksMatrix4x2f * value ); static void
ksGpuComputeCommand_SetParmFloatMatrix4x3( ksGpuComputeCommand * command, const int index, const ksMatrix4x3f * value ); static void
ksGpuComputeCommand_SetParmFloatMatrix4x4( ksGpuComputeCommand * command, const int index, const ksMatrix4x4f * value ); static void
ksGpuComputeCommand_SetDimensions( ksGpuComputeCommand * command, const int x, const int y, const int z );

================================================================================================================================
*/

typedef struct {
    const ksGpuComputePipeline *pipeline;
    ksGpuProgramParmState parmState;
    int x;
    int y;
    int z;
} ksGpuComputeCommand;

static void ksGpuComputeCommand_Init(ksGpuComputeCommand *command) {
    command->pipeline = NULL;
    memset((void *)&command->parmState, 0, sizeof(command->parmState));
    command->x = 1;
    command->y = 1;
    command->z = 1;
}

static void ksGpuComputeCommand_SetPipeline(ksGpuComputeCommand *command, const ksGpuComputePipeline *pipeline) {
    command->pipeline = pipeline;
}

static void ksGpuComputeCommand_SetParmTextureSampled(ksGpuComputeCommand *command, const int index, const ksGpuTexture *texture) {
    ksGpuProgramParmState_SetParm(&command->parmState, &command->pipeline->program->parmLayout, index,
                                  KS_GPU_PROGRAM_PARM_TYPE_TEXTURE_SAMPLED, texture);
}

static void ksGpuComputeCommand_SetParmTextureStorage(ksGpuComputeCommand *command, const int index, const ksGpuTexture *texture) {
    ksGpuProgramParmState_SetParm(&command->parmState, &command->pipeline->program->parmLayout, index,
                                  KS_GPU_PROGRAM_PARM_TYPE_TEXTURE_STORAGE, texture);
}

static void ksGpuComputeCommand_SetParmBufferUniform(ksGpuComputeCommand *command, const int index, const ksGpuBuffer *buffer) {
    ksGpuProgramParmState_SetParm(&command->parmState, &command->pipeline->program->parmLayout, index,
                                  KS_GPU_PROGRAM_PARM_TYPE_BUFFER_UNIFORM, buffer);
}

static void ksGpuComputeCommand_SetParmBufferStorage(ksGpuComputeCommand *command, const int index, const ksGpuBuffer *buffer) {
    ksGpuProgramParmState_SetParm(&command->parmState, &command->pipeline->program->parmLayout, index,
                                  KS_GPU_PROGRAM_PARM_TYPE_BUFFER_STORAGE, buffer);
}

static void ksGpuComputeCommand_SetParmInt(ksGpuComputeCommand *command, const int index, const int *value) {
    ksGpuProgramParmState_SetParm(&command->parmState, &command->pipeline->program->parmLayout, index,
                                  KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_INT, value);
}

static void ksGpuComputeCommand_SetParmIntVector2(ksGpuComputeCommand *command, const int index, const ksVector2i *value) {
    ksGpuProgramParmState_SetParm(&command->parmState, &command->pipeline->program->parmLayout, index,
                                  KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_INT_VECTOR2, value);
}

static void ksGpuComputeCommand_SetParmIntVector3(ksGpuComputeCommand *command, const int index, const ksVector3i *value) {
    ksGpuProgramParmState_SetParm(&command->parmState, &command->pipeline->program->parmLayout, index,
                                  KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_INT_VECTOR3, value);
}

static void ksGpuComputeCommand_SetParmIntVector4(ksGpuComputeCommand *command, const int index, const ksVector4i *value) {
    ksGpuProgramParmState_SetParm(&command->parmState, &command->pipeline->program->parmLayout, index,
                                  KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_INT_VECTOR4, value);
}

static void ksGpuComputeCommand_SetParmFloat(ksGpuComputeCommand *command, const int index, const float *value) {
    ksGpuProgramParmState_SetParm(&command->parmState, &command->pipeline->program->parmLayout, index,
                                  KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT, value);
}

static void ksGpuComputeCommand_SetParmFloatVector2(ksGpuComputeCommand *command, const int index, const ksVector2f *value) {
    ksGpuProgramParmState_SetParm(&command->parmState, &command->pipeline->program->parmLayout, index,
                                  KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_VECTOR2, value);
}

static void ksGpuComputeCommand_SetParmFloatVector3(ksGpuComputeCommand *command, const int index, const ksVector3f *value) {
    ksGpuProgramParmState_SetParm(&command->parmState, &command->pipeline->program->parmLayout, index,
                                  KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_VECTOR3, value);
}

static void ksGpuComputeCommand_SetParmFloatVector4(ksGpuComputeCommand *command, const int index, const ksVector4f *value) {
    ksGpuProgramParmState_SetParm(&command->parmState, &command->pipeline->program->parmLayout, index,
                                  KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_VECTOR4, value);
}

static void ksGpuComputeCommand_SetParmFloatMatrix2x2(ksGpuComputeCommand *command, const int index, const ksMatrix2x2f *value) {
    ksGpuProgramParmState_SetParm(&command->parmState, &command->pipeline->program->parmLayout, index,
                                  KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_MATRIX2X2, value);
}

static void ksGpuComputeCommand_SetParmFloatMatrix2x3(ksGpuComputeCommand *command, const int index, const ksMatrix2x3f *value) {
    ksGpuProgramParmState_SetParm(&command->parmState, &command->pipeline->program->parmLayout, index,
                                  KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_MATRIX2X3, value);
}

static void ksGpuComputeCommand_SetParmFloatMatrix2x4(ksGpuComputeCommand *command, const int index, const ksMatrix2x4f *value) {
    ksGpuProgramParmState_SetParm(&command->parmState, &command->pipeline->program->parmLayout, index,
                                  KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_MATRIX2X4, value);
}

static void ksGpuComputeCommand_SetParmFloatMatrix3x2(ksGpuComputeCommand *command, const int index, const ksMatrix3x2f *value) {
    ksGpuProgramParmState_SetParm(&command->parmState, &command->pipeline->program->parmLayout, index,
                                  KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_MATRIX3X2, value);
}

static void ksGpuComputeCommand_SetParmFloatMatrix3x3(ksGpuComputeCommand *command, const int index, const ksMatrix3x3f *value) {
    ksGpuProgramParmState_SetParm(&command->parmState, &command->pipeline->program->parmLayout, index,
                                  KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_MATRIX3X3, value);
}

static void ksGpuComputeCommand_SetParmFloatMatrix3x4(ksGpuComputeCommand *command, const int index, const ksMatrix3x4f *value) {
    ksGpuProgramParmState_SetParm(&command->parmState, &command->pipeline->program->parmLayout, index,
                                  KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_MATRIX3X4, value);
}

static void ksGpuComputeCommand_SetParmFloatMatrix4x2(ksGpuComputeCommand *command, const int index, const ksMatrix4x2f *value) {
    ksGpuProgramParmState_SetParm(&command->parmState, &command->pipeline->program->parmLayout, index,
                                  KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_MATRIX4X2, value);
}

static void ksGpuComputeCommand_SetParmFloatMatrix4x3(ksGpuComputeCommand *command, const int index, const ksMatrix4x3f *value) {
    ksGpuProgramParmState_SetParm(&command->parmState, &command->pipeline->program->parmLayout, index,
                                  KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_MATRIX4X3, value);
}

static void ksGpuComputeCommand_SetParmFloatMatrix4x4(ksGpuComputeCommand *command, const int index, const ksMatrix4x4f *value) {
    ksGpuProgramParmState_SetParm(&command->parmState, &command->pipeline->program->parmLayout, index,
                                  KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_MATRIX4X4, value);
}

static void ksGpuComputeCommand_SetDimensions(ksGpuComputeCommand *command, const int x, const int y, const int z) {
    command->x = x;
    command->y = y;
    command->z = z;
}

/*
================================================================================================================================

GPU command buffer.

A command buffer is used to record graphics and compute commands.
For optimal performance a command buffer should only be created at load time, not at runtime.
When a command is submitted, the state of the command is compared with the currently saved state,
and only the state that has changed translates into graphics API function calls.

ksGpuCommandBuffer
ksGpuCommandBufferType
ksGpuBufferUnmapType

static void ksGpuCommandBuffer_Create( ksGpuContext * context, ksGpuCommandBuffer * commandBuffer, const ksGpuCommandBufferType
type, const int numBuffers ); static void ksGpuCommandBuffer_Destroy( ksGpuContext * context, ksGpuCommandBuffer * commandBuffer );

static void ksGpuCommandBuffer_BeginPrimary( ksGpuCommandBuffer * commandBuffer );
static void ksGpuCommandBuffer_EndPrimary( ksGpuCommandBuffer * commandBuffer );
static ksGpuFence * ksGpuCommandBuffer_SubmitPrimary( ksGpuCommandBuffer * commandBuffer );

static void ksGpuCommandBuffer_ChangeTextureUsage( ksGpuCommandBuffer * commandBuffer, ksGpuTexture * texture, const
ksGpuTextureUsage usage );

static void ksGpuCommandBuffer_BeginFramebuffer( ksGpuCommandBuffer * commandBuffer, ksGpuFramebuffer * framebuffer, const int
arrayLayer, const ksGpuTextureUsage usage ); static void ksGpuCommandBuffer_EndFramebuffer( ksGpuCommandBuffer * commandBuffer,
ksGpuFramebuffer * framebuffer, const int arrayLayer, const ksGpuTextureUsage usage );

static void ksGpuCommandBuffer_BeginTimer( ksGpuCommandBuffer * commandBuffer, ksGpuTimer * timer );
static void ksGpuCommandBuffer_EndTimer( ksGpuCommandBuffer * commandBuffer, ksGpuTimer * timer );

static void ksGpuCommandBuffer_BeginRenderPass( ksGpuCommandBuffer * commandBuffer, ksGpuRenderPass * renderPass, ksGpuFramebuffer *
framebuffer, const ksScreenRect * rect ); static void ksGpuCommandBuffer_EndRenderPass( ksGpuCommandBuffer * commandBuffer,
ksGpuRenderPass * renderPass );

static void ksGpuCommandBuffer_SetViewport( ksGpuCommandBuffer * commandBuffer, const ksScreenRect * rect );
static void ksGpuCommandBuffer_SetScissor( ksGpuCommandBuffer * commandBuffer, const ksScreenRect * rect );

static void ksGpuCommandBuffer_SubmitGraphicsCommand( ksGpuCommandBuffer * commandBuffer, const ksGpuGraphicsCommand * command );
static void ksGpuCommandBuffer_SubmitComputeCommand( ksGpuCommandBuffer * commandBuffer, const ksGpuComputeCommand * command );

static ksGpuBuffer * ksGpuCommandBuffer_MapBuffer( ksGpuCommandBuffer * commandBuffer, ksGpuBuffer * buffer, void ** data );
static void ksGpuCommandBuffer_UnmapBuffer( ksGpuCommandBuffer * commandBuffer, ksGpuBuffer * buffer, ksGpuBuffer * mappedBuffer,
const ksGpuBufferUnmapType type );

static ksGpuBuffer * ksGpuCommandBuffer_MapVertexAttributes( ksGpuCommandBuffer * commandBuffer, ksGpuGeometry * geometry,
ksGpuVertexAttributeArrays * attribs ); static void ksGpuCommandBuffer_UnmapVertexAttributes( ksGpuCommandBuffer * commandBuffer,
ksGpuGeometry * geometry, ksGpuBuffer * mappedVertexBuffer, const ksGpuBufferUnmapType type );

static ksGpuBuffer * ksGpuCommandBuffer_MapInstanceAttributes( ksGpuCommandBuffer * commandBuffer, ksGpuGeometry * geometry,
ksGpuVertexAttributeArrays * attribs ); static void ksGpuCommandBuffer_UnmapInstanceAttributes( ksGpuCommandBuffer * commandBuffer,
ksGpuGeometry * geometry, ksGpuBuffer * mappedInstanceBuffer, const ksGpuBufferUnmapType type );

================================================================================================================================
*/

typedef enum {
    KS_GPU_BUFFER_UNMAP_TYPE_USE_ALLOCATED,  // use the newly allocated (host visible) buffer
    KS_GPU_BUFFER_UNMAP_TYPE_COPY_BACK       // copy back to the original buffer
} ksGpuBufferUnmapType;

typedef enum {
    KS_GPU_COMMAND_BUFFER_TYPE_PRIMARY,
    KS_GPU_COMMAND_BUFFER_TYPE_SECONDARY,
    KS_GPU_COMMAND_BUFFER_TYPE_SECONDARY_CONTINUE_RENDER_PASS
} ksGpuCommandBufferType;

typedef struct {
    ksGpuCommandBufferType type;
    int numBuffers;
    int currentBuffer;
    ksGpuFence *fences;
    ksGpuContext *context;
    ksGpuGraphicsCommand currentGraphicsState;
    ksGpuComputeCommand currentComputeState;
    ksGpuFramebuffer *currentFramebuffer;
    ksGpuRenderPass *currentRenderPass;
    ksGpuTextureUsage currentTextureUsage;
} ksGpuCommandBuffer;

static void ksGpuCommandBuffer_Create(ksGpuContext *context, ksGpuCommandBuffer *commandBuffer, const ksGpuCommandBufferType type,
                                      const int numBuffers) {
    assert(type == KS_GPU_COMMAND_BUFFER_TYPE_PRIMARY);

    memset(commandBuffer, 0, sizeof(ksGpuCommandBuffer));

    commandBuffer->type = type;
    commandBuffer->numBuffers = numBuffers;
    commandBuffer->currentBuffer = 0;
    commandBuffer->context = context;

    commandBuffer->fences = (ksGpuFence *)malloc(numBuffers * sizeof(ksGpuFence));

    for (int i = 0; i < numBuffers; i++) {
        ksGpuFence_Create(context, &commandBuffer->fences[i]);
    }
}

static void ksGpuCommandBuffer_Destroy(ksGpuContext *context, ksGpuCommandBuffer *commandBuffer) {
    assert(context == commandBuffer->context);

    for (int i = 0; i < commandBuffer->numBuffers; i++) {
        ksGpuFence_Destroy(context, &commandBuffer->fences[i]);
    }

    free(commandBuffer->fences);

    memset(commandBuffer, 0, sizeof(ksGpuCommandBuffer));
}

void ksGpuCommandBuffer_ChangeRopState(const ksGpuRasterOperations *cmdRop, const ksGpuRasterOperations *stateRop) {
    // Set front face.
    if (stateRop == NULL || cmdRop->frontFace != stateRop->frontFace) {
        GL(glFrontFace(cmdRop->frontFace));
    }
    // Set face culling.
    if (stateRop == NULL || cmdRop->cullMode != stateRop->cullMode) {
        if (cmdRop->cullMode != KS_GPU_CULL_MODE_NONE) {
            GL(glEnable(GL_CULL_FACE));
            GL(glCullFace(cmdRop->cullMode));
        } else {
            GL(glDisable(GL_CULL_FACE));
        }
    }
    // Enable / disable depth testing.
    if (stateRop == NULL || cmdRop->depthTestEnable != stateRop->depthTestEnable) {
        if (cmdRop->depthTestEnable) {
            GL(glEnable(GL_DEPTH_TEST));
        } else {
            GL(glDisable(GL_DEPTH_TEST));
        }
    }
    // The depth test function is only used when depth testing is enabled.
    if (stateRop == NULL || cmdRop->depthCompare != stateRop->depthCompare) {
        GL(glDepthFunc(cmdRop->depthCompare));
    }
    // Depth is only written when depth testing is enabled.
    // Set the depth function to GL_ALWAYS to write depth without actually testing depth.
    if (stateRop == NULL || cmdRop->depthWriteEnable != stateRop->depthWriteEnable) {
        if (cmdRop->depthWriteEnable) {
            GL(glDepthMask(GL_TRUE));
        } else {
            GL(glDepthMask(GL_FALSE));
        }
    }
    // Enable / disable blending.
    if (stateRop == NULL || cmdRop->blendEnable != stateRop->blendEnable) {
        if (cmdRop->blendEnable) {
            GL(glEnable(GL_BLEND));
        } else {
            GL(glDisable(GL_BLEND));
        }
    }
    // Enable / disable writing alpha.
    if (stateRop == NULL || cmdRop->redWriteEnable != stateRop->redWriteEnable ||
        cmdRop->blueWriteEnable != stateRop->blueWriteEnable || cmdRop->greenWriteEnable != stateRop->greenWriteEnable ||
        cmdRop->alphaWriteEnable != stateRop->alphaWriteEnable) {
        GL(glColorMask(cmdRop->redWriteEnable ? GL_TRUE : GL_FALSE, cmdRop->blueWriteEnable ? GL_TRUE : GL_FALSE,
                       cmdRop->greenWriteEnable ? GL_TRUE : GL_FALSE, cmdRop->alphaWriteEnable ? GL_TRUE : GL_FALSE));
    }
    // The blend equation is only used when blending is enabled.
    if (stateRop == NULL || cmdRop->blendOpColor != stateRop->blendOpColor || cmdRop->blendOpAlpha != stateRop->blendOpAlpha) {
        GL(glBlendEquationSeparate(cmdRop->blendOpColor, cmdRop->blendOpAlpha));
    }
    // The blend function is only used when blending is enabled.
    if (stateRop == NULL || cmdRop->blendSrcColor != stateRop->blendSrcColor || cmdRop->blendDstColor != stateRop->blendDstColor ||
        cmdRop->blendSrcAlpha != stateRop->blendSrcAlpha || cmdRop->blendDstAlpha != stateRop->blendDstAlpha) {
        GL(glBlendFuncSeparate(cmdRop->blendSrcColor, cmdRop->blendDstColor, cmdRop->blendSrcAlpha, cmdRop->blendDstAlpha));
    }
    // The blend color is only used when blending is enabled.
    if (stateRop == NULL || cmdRop->blendColor.x != stateRop->blendColor.x || cmdRop->blendColor.y != stateRop->blendColor.y ||
        cmdRop->blendColor.z != stateRop->blendColor.z || cmdRop->blendColor.w != stateRop->blendColor.w) {
        GL(glBlendColor(cmdRop->blendColor.x, cmdRop->blendColor.y, cmdRop->blendColor.z, cmdRop->blendColor.w));
    }
}

static void ksGpuCommandBuffer_BeginPrimary(ksGpuCommandBuffer *commandBuffer) {
    assert(commandBuffer->type == KS_GPU_COMMAND_BUFFER_TYPE_PRIMARY);
    assert(commandBuffer->currentFramebuffer == NULL);
    assert(commandBuffer->currentRenderPass == NULL);

    commandBuffer->currentBuffer = (commandBuffer->currentBuffer + 1) % commandBuffer->numBuffers;

    ksGpuGraphicsCommand_Init(&commandBuffer->currentGraphicsState);
    ksGpuComputeCommand_Init(&commandBuffer->currentComputeState);
    commandBuffer->currentTextureUsage = KS_GPU_TEXTURE_USAGE_UNDEFINED;

    ksGpuGraphicsPipelineParms parms;
    ksGpuGraphicsPipelineParms_Init(&parms);
    ksGpuCommandBuffer_ChangeRopState(&parms.rop, NULL);

    GL(glUseProgram(0));
    GL(glBindVertexArray(0));
}

static void ksGpuCommandBuffer_EndPrimary(ksGpuCommandBuffer *commandBuffer) {
    assert(commandBuffer->type == KS_GPU_COMMAND_BUFFER_TYPE_PRIMARY);
    assert(commandBuffer->currentFramebuffer == NULL);
    assert(commandBuffer->currentRenderPass == NULL);

    UNUSED_PARM(commandBuffer);
}

static ksGpuFence *ksGpuCommandBuffer_SubmitPrimary(ksGpuCommandBuffer *commandBuffer) {
    assert(commandBuffer->type == KS_GPU_COMMAND_BUFFER_TYPE_PRIMARY);
    assert(commandBuffer->currentFramebuffer == NULL);
    assert(commandBuffer->currentRenderPass == NULL);

    ksGpuFence *fence = &commandBuffer->fences[commandBuffer->currentBuffer];
    ksGpuFence_Submit(commandBuffer->context, fence);

    return fence;
}

static void ksGpuCommandBuffer_ChangeTextureUsage(ksGpuCommandBuffer *commandBuffer, ksGpuTexture *texture,
                                                  const ksGpuTextureUsage usage) {
    assert((texture->usageFlags & usage) != 0);

    texture->usage = usage;

    if (usage == commandBuffer->currentTextureUsage) {
        return;
    }

    const GLbitfield barriers =
        ((usage == KS_GPU_TEXTURE_USAGE_TRANSFER_SRC)
             ? GL_TEXTURE_UPDATE_BARRIER_BIT
             : ((usage == KS_GPU_TEXTURE_USAGE_TRANSFER_DST)
                    ? GL_TEXTURE_UPDATE_BARRIER_BIT
                    : ((usage == KS_GPU_TEXTURE_USAGE_SAMPLED)
                           ? GL_TEXTURE_FETCH_BARRIER_BIT
                           : ((usage == KS_GPU_TEXTURE_USAGE_STORAGE)
                                  ? GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
                                  : ((usage == KS_GPU_TEXTURE_USAGE_COLOR_ATTACHMENT)
                                         ? GL_FRAMEBUFFER_BARRIER_BIT
                                         : ((usage == KS_GPU_TEXTURE_USAGE_PRESENTATION) ? GL_ALL_BARRIER_BITS
                                                                                         : GL_ALL_BARRIER_BITS))))));

    GL(glMemoryBarrier(barriers));

    commandBuffer->currentTextureUsage = usage;
}

static void ksGpuCommandBuffer_BeginFramebuffer(ksGpuCommandBuffer *commandBuffer, ksGpuFramebuffer *framebuffer,
                                                const int arrayLayer, const ksGpuTextureUsage usage) {
    assert(commandBuffer->type == KS_GPU_COMMAND_BUFFER_TYPE_PRIMARY);
    assert(commandBuffer->currentFramebuffer == NULL);
    assert(commandBuffer->currentRenderPass == NULL);
    assert(arrayLayer >= 0 && arrayLayer < framebuffer->numFramebuffersPerTexture);

    // Only advance when rendering to the first layer.
    if (arrayLayer == 0) {
        framebuffer->currentBuffer = (framebuffer->currentBuffer + 1) % framebuffer->numBuffers;
    }

    GL(glBindFramebuffer(
        GL_DRAW_FRAMEBUFFER,
        framebuffer->renderBuffers[framebuffer->currentBuffer * framebuffer->numFramebuffersPerTexture + arrayLayer]));

    if (framebuffer->colorTextures != NULL) {
        framebuffer->colorTextures[framebuffer->currentBuffer].usage = usage;
    }
    commandBuffer->currentFramebuffer = framebuffer;
}

static void ksGpuCommandBuffer_EndFramebuffer(ksGpuCommandBuffer *commandBuffer, ksGpuFramebuffer *framebuffer,
                                              const int arrayLayer, const ksGpuTextureUsage usage) {
    assert(commandBuffer->type == KS_GPU_COMMAND_BUFFER_TYPE_PRIMARY);
    assert(commandBuffer->currentFramebuffer == framebuffer);
    assert(commandBuffer->currentRenderPass == NULL);
    assert(arrayLayer >= 0 && arrayLayer < framebuffer->numFramebuffersPerTexture);

    UNUSED_PARM(framebuffer);
    UNUSED_PARM(arrayLayer);

    // If clamp to border is not available.
    if (!glExtensions.texture_clamp_to_border) {
        // If rendering to a texture.
        if (framebuffer->renderBuffers[framebuffer->currentBuffer * framebuffer->numFramebuffersPerTexture + arrayLayer] != 0) {
            // Explicitly clear the border texels to black if the texture has clamp-to-border set.
            const ksGpuTexture *texture = &framebuffer->colorTextures[framebuffer->currentBuffer];
            if (texture->wrapMode == KS_GPU_TEXTURE_WRAP_MODE_CLAMP_TO_BORDER) {
                // Clear to fully opaque black.
                GL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
                // bottom
                GL(glScissor(0, 0, texture->width, 1));
                GL(glClear(GL_COLOR_BUFFER_BIT));
                // top
                GL(glScissor(0, texture->height - 1, texture->width, 1));
                GL(glClear(GL_COLOR_BUFFER_BIT));
                // left
                GL(glScissor(0, 0, 1, texture->height));
                GL(glClear(GL_COLOR_BUFFER_BIT));
                // right
                GL(glScissor(texture->width - 1, 0, 1, texture->height));
                GL(glClear(GL_COLOR_BUFFER_BIT));
            }
        }
    }

#if defined(OS_ANDROID)
    // If this framebuffer has a depth buffer.
    if (framebuffer->depthBuffer != 0) {
        // Discard the depth buffer, so a tiler won't need to write it back out to memory.
        const GLenum depthAttachment[1] = {GL_DEPTH_ATTACHMENT};
        GL(glInvalidateFramebuffer(GL_DRAW_FRAMEBUFFER, 1, depthAttachment));
    }
#endif

    if (framebuffer->resolveBuffers != framebuffer->renderBuffers) {
        const ksScreenRect rect = ksGpuFramebuffer_GetRect(framebuffer);
        glBindFramebuffer(
            GL_READ_FRAMEBUFFER,
            framebuffer->renderBuffers[framebuffer->currentBuffer * framebuffer->numFramebuffersPerTexture + arrayLayer]);
        glBindFramebuffer(
            GL_DRAW_FRAMEBUFFER,
            framebuffer->resolveBuffers[framebuffer->currentBuffer * framebuffer->numFramebuffersPerTexture + arrayLayer]);
        glBlitFramebuffer(rect.x, rect.y, rect.width, rect.height, rect.x, rect.y, rect.width, rect.height, GL_COLOR_BUFFER_BIT,
                          GL_NEAREST);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    }

    GL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));

    if (framebuffer->colorTextures != NULL) {
        framebuffer->colorTextures[framebuffer->currentBuffer].usage = usage;
    }
    commandBuffer->currentFramebuffer = NULL;
}

static void ksGpuCommandBuffer_BeginTimer(ksGpuCommandBuffer *commandBuffer, ksGpuTimer *timer) {
    UNUSED_PARM(commandBuffer);

    if (glExtensions.timer_query) {
        if (timer->queryIndex >= KS_GPU_TIMER_FRAMES_DELAYED) {
            GLuint64 beginGpuTime = 0;
            GL(glGetQueryObjectui64v(timer->beginQueries[timer->queryIndex % KS_GPU_TIMER_FRAMES_DELAYED], GL_QUERY_RESULT,
                                     &beginGpuTime));
            GLuint64 endGpuTime = 0;
            GL(glGetQueryObjectui64v(timer->endQueries[timer->queryIndex % KS_GPU_TIMER_FRAMES_DELAYED], GL_QUERY_RESULT,
                                     &endGpuTime));

            timer->gpuTime = (endGpuTime - beginGpuTime);
        }

        GL(glQueryCounter(timer->beginQueries[timer->queryIndex % KS_GPU_TIMER_FRAMES_DELAYED], GL_TIMESTAMP));
    }
}

static void ksGpuCommandBuffer_EndTimer(ksGpuCommandBuffer *commandBuffer, ksGpuTimer *timer) {
    UNUSED_PARM(commandBuffer);

    if (glExtensions.timer_query) {
        GL(glQueryCounter(timer->endQueries[timer->queryIndex % KS_GPU_TIMER_FRAMES_DELAYED], GL_TIMESTAMP));
        timer->queryIndex++;
    }
}

static void ksGpuCommandBuffer_BeginRenderPass(ksGpuCommandBuffer *commandBuffer, ksGpuRenderPass *renderPass,
                                               ksGpuFramebuffer *framebuffer, const ksScreenRect *rect) {
    assert(commandBuffer->type == KS_GPU_COMMAND_BUFFER_TYPE_PRIMARY);
    assert(commandBuffer->currentRenderPass == NULL);
    assert(commandBuffer->currentFramebuffer == framebuffer);

    UNUSED_PARM(framebuffer);

    if ((renderPass->flags & (KS_GPU_RENDERPASS_FLAG_CLEAR_COLOR_BUFFER | KS_GPU_RENDERPASS_FLAG_CLEAR_DEPTH_BUFFER)) != 0) {
        GL(glEnable(GL_SCISSOR_TEST));
        GL(glScissor(rect->x, rect->y, rect->width, rect->height));
        GL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
        GL(glClear((((renderPass->flags & KS_GPU_RENDERPASS_FLAG_CLEAR_COLOR_BUFFER) != 0) ? GL_COLOR_BUFFER_BIT : 0) |
                   (((renderPass->flags & KS_GPU_RENDERPASS_FLAG_CLEAR_DEPTH_BUFFER) != 0) ? GL_DEPTH_BUFFER_BIT : 0)));
    }

    commandBuffer->currentRenderPass = renderPass;
}

static void ksGpuCommandBuffer_EndRenderPass(ksGpuCommandBuffer *commandBuffer, ksGpuRenderPass *renderPass) {
    assert(commandBuffer->type == KS_GPU_COMMAND_BUFFER_TYPE_PRIMARY);
    assert(commandBuffer->currentRenderPass == renderPass);

    UNUSED_PARM(renderPass);

    commandBuffer->currentRenderPass = NULL;
}

static void ksGpuCommandBuffer_SetViewport(ksGpuCommandBuffer *commandBuffer, const ksScreenRect *rect) {
    UNUSED_PARM(commandBuffer);

    GL(glViewport(rect->x, rect->y, rect->width, rect->height));
}

static void ksGpuCommandBuffer_SetScissor(ksGpuCommandBuffer *commandBuffer, const ksScreenRect *rect) {
    UNUSED_PARM(commandBuffer);

    GL(glEnable(GL_SCISSOR_TEST));
    GL(glScissor(rect->x, rect->y, rect->width, rect->height));
}

static void ksGpuCommandBuffer_UpdateProgramParms(const ksGpuProgramParmLayout *newLayout, const ksGpuProgramParmLayout *oldLayout,
                                                  const ksGpuProgramParmState *newParmState,
                                                  const ksGpuProgramParmState *oldParmState, const bool force) {
    const ksGpuTexture *oldSampledTextures[MAX_PROGRAM_PARMS] = {0};
    const ksGpuTexture *oldStorageTextures[MAX_PROGRAM_PARMS] = {0};
    const ksGpuBuffer *oldUniformBuffers[MAX_PROGRAM_PARMS] = {0};
    const ksGpuBuffer *oldStorageBuffers[MAX_PROGRAM_PARMS] = {0};
    int oldPushConstantParms[MAX_PROGRAM_PARMS] = {0};

    if (oldLayout != NULL) {
        // Unbind from the bind points that will no longer be used, and gather
        // the objects that are bound at the bind points that will be used.
        for (int i = 0; i < oldLayout->numParms; i++) {
            const int index = oldLayout->parms[i].index;
            const int binding = oldLayout->parmBindings[i];

            if (oldLayout->parms[i].type == KS_GPU_PROGRAM_PARM_TYPE_TEXTURE_SAMPLED) {
                if (binding >= newLayout->numSampledTextureBindings) {
                    const ksGpuTexture *stateTexture = (const ksGpuTexture *)oldParmState->parms[index];
                    GL(glActiveTexture(GL_TEXTURE0 + binding));
                    GL(glBindTexture(stateTexture->target, 0));
                } else {
                    oldSampledTextures[binding] = (const ksGpuTexture *)oldParmState->parms[index];
                }
            } else if (oldLayout->parms[i].type == KS_GPU_PROGRAM_PARM_TYPE_TEXTURE_STORAGE) {
                if (binding >= newLayout->numStorageTextureBindings) {
                    GL(glBindImageTexture(binding, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8));
                } else {
                    oldStorageTextures[binding] = (const ksGpuTexture *)oldParmState->parms[index];
                }
            } else if (oldLayout->parms[i].type == KS_GPU_PROGRAM_PARM_TYPE_BUFFER_UNIFORM) {
                if (binding >= newLayout->numUniformBufferBindings) {
                    GL(glBindBufferBase(GL_UNIFORM_BUFFER, binding, 0));
                } else {
                    oldUniformBuffers[binding] = (const ksGpuBuffer *)oldParmState->parms[index];
                }
            } else if (oldLayout->parms[i].type == KS_GPU_PROGRAM_PARM_TYPE_BUFFER_STORAGE) {
                if (binding >= newLayout->numStorageBufferBindings) {
                    GL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, 0));
                } else {
                    oldStorageBuffers[binding] = (const ksGpuBuffer *)oldParmState->parms[index];
                }
            } else {
                oldPushConstantParms[binding] = i;
            }
        }
    }

    // Update the bind points.
    for (int i = 0; i < newLayout->numParms; i++) {
        const int index = newLayout->parms[i].index;
        const int binding = newLayout->parmBindings[i];

        assert(newParmState->parms[index] != NULL);
        if (newLayout->parms[i].type == KS_GPU_PROGRAM_PARM_TYPE_TEXTURE_SAMPLED) {
            const ksGpuTexture *texture = (const ksGpuTexture *)newParmState->parms[index];
            assert(texture->usage == KS_GPU_TEXTURE_USAGE_SAMPLED);
            if (force || texture != oldSampledTextures[binding]) {
                GL(glActiveTexture(GL_TEXTURE0 + binding));
                GL(glBindTexture(texture->target, texture->texture));
            }
        } else if (newLayout->parms[i].type == KS_GPU_PROGRAM_PARM_TYPE_TEXTURE_STORAGE) {
            const ksGpuTexture *texture = (const ksGpuTexture *)newParmState->parms[index];
            assert(texture->usage == KS_GPU_TEXTURE_USAGE_STORAGE);
            if (force || texture != oldStorageTextures[binding]) {
                const GLenum access =
                    ((newLayout->parms[i].access == KS_GPU_PROGRAM_PARM_ACCESS_READ_ONLY)
                         ? GL_READ_ONLY
                         : ((newLayout->parms[i].access == KS_GPU_PROGRAM_PARM_ACCESS_WRITE_ONLY)
                                ? GL_WRITE_ONLY
                                : ((newLayout->parms[i].access == KS_GPU_PROGRAM_PARM_ACCESS_READ_WRITE) ? GL_READ_WRITE : 0)));
                GL(glBindImageTexture(binding, texture->texture, 0, GL_FALSE, 0, access, texture->format));
            }
        } else if (newLayout->parms[i].type == KS_GPU_PROGRAM_PARM_TYPE_BUFFER_UNIFORM) {
            const ksGpuBuffer *buffer = (const ksGpuBuffer *)newParmState->parms[index];
            assert(buffer->target == GL_UNIFORM_BUFFER);
            if (force || buffer != oldUniformBuffers[binding]) {
                GL(glBindBufferBase(GL_UNIFORM_BUFFER, binding, buffer->buffer));
            }
        } else if (newLayout->parms[i].type == KS_GPU_PROGRAM_PARM_TYPE_BUFFER_STORAGE) {
            const ksGpuBuffer *buffer = (const ksGpuBuffer *)newParmState->parms[index];
            assert(buffer->target == GL_SHADER_STORAGE_BUFFER);
            if (force || buffer != oldStorageBuffers[binding]) {
                GL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, buffer->buffer));
            }
        } else {
            const void *newData = ksGpuProgramParmState_NewPushConstantData(newLayout, i, newParmState, oldLayout,
                                                                            oldPushConstantParms[binding], oldParmState, force);
            if (newData != NULL) {
                const GLint location = newLayout->parmLocations[i];
                switch (newLayout->parms[i].type) {
                    case KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_INT:
                        GL(glUniform1iv(location, 1, (const GLint *)newData));
                        break;
                    case KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_INT_VECTOR2:
                        GL(glUniform2iv(location, 1, (const GLint *)newData));
                        break;
                    case KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_INT_VECTOR3:
                        GL(glUniform3iv(location, 1, (const GLint *)newData));
                        break;
                    case KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_INT_VECTOR4:
                        GL(glUniform4iv(location, 1, (const GLint *)newData));
                        break;
                    case KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT:
                        GL(glUniform1fv(location, 1, (const GLfloat *)newData));
                        break;
                    case KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_VECTOR2:
                        GL(glUniform2fv(location, 1, (const GLfloat *)newData));
                        break;
                    case KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_VECTOR3:
                        GL(glUniform3fv(location, 1, (const GLfloat *)newData));
                        break;
                    case KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_VECTOR4:
                        GL(glUniform4fv(location, 1, (const GLfloat *)newData));
                        break;
                    case KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_MATRIX2X2:
                        GL(glUniformMatrix2fv(location, 1, GL_FALSE, (const GLfloat *)newData));
                        break;
                    case KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_MATRIX2X3:
                        GL(glUniformMatrix2x3fv(location, 1, GL_FALSE, (const GLfloat *)newData));
                        break;
                    case KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_MATRIX2X4:
                        GL(glUniformMatrix2x4fv(location, 1, GL_FALSE, (const GLfloat *)newData));
                        break;
                    case KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_MATRIX3X2:
                        GL(glUniformMatrix3x2fv(location, 1, GL_FALSE, (const GLfloat *)newData));
                        break;
                    case KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_MATRIX3X3:
                        GL(glUniformMatrix3fv(location, 1, GL_FALSE, (const GLfloat *)newData));
                        break;
                    case KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_MATRIX3X4:
                        GL(glUniformMatrix3x4fv(location, 1, GL_FALSE, (const GLfloat *)newData));
                        break;
                    case KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_MATRIX4X2:
                        GL(glUniformMatrix4x2fv(location, 1, GL_FALSE, (const GLfloat *)newData));
                        break;
                    case KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_MATRIX4X3:
                        GL(glUniformMatrix4x3fv(location, 1, GL_FALSE, (const GLfloat *)newData));
                        break;
                    case KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_MATRIX4X4:
                        GL(glUniformMatrix4fv(location, 1, GL_FALSE, (const GLfloat *)newData));
                        break;
                    case KS_GPU_PROGRAM_PARM_TYPE_TEXTURE_SAMPLED:
                    case KS_GPU_PROGRAM_PARM_TYPE_TEXTURE_STORAGE:
                    case KS_GPU_PROGRAM_PARM_TYPE_BUFFER_STORAGE:
                    case KS_GPU_PROGRAM_PARM_TYPE_BUFFER_UNIFORM:
                        // these were handled above, not possible to reach these.
                    case KS_GPU_PROGRAM_PARM_TYPE_MAX:
                    default:
                        assert(false);
                        break;
                }
            }
        }
    }
}

static void ksGpuCommandBuffer_SubmitGraphicsCommand(ksGpuCommandBuffer *commandBuffer, const ksGpuGraphicsCommand *command) {
    assert(commandBuffer->currentRenderPass != NULL);

    const ksGpuGraphicsCommand *state = &commandBuffer->currentGraphicsState;

    ksGpuCommandBuffer_ChangeRopState(&command->pipeline->rop, (state->pipeline != NULL) ? &state->pipeline->rop : NULL);

    // Compare programs based on a vertex/fragment source code hash value to minimize state changes when
    // the same source code is compiled to programs in different locations.
    const bool differentProgram = (state->pipeline == NULL || command->pipeline->program->hash != state->pipeline->program->hash);

    if (differentProgram) {
        GL(glUseProgram(command->pipeline->program->program));
    }

    ksGpuCommandBuffer_UpdateProgramParms(&command->pipeline->program->parmLayout,
                                          (state->pipeline != NULL) ? &state->pipeline->program->parmLayout : NULL,
                                          &command->parmState, &state->parmState, differentProgram);

    if (command->pipeline != state->pipeline) {
        GL(glBindVertexArray(command->pipeline->vertexArrayObject));
    }

    const GLenum indexType = (sizeof(ksGpuTriangleIndex) == sizeof(GLuint)) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;
    if (command->numInstances > 1) {
        GL(glDrawElementsInstanced(GL_TRIANGLES, command->pipeline->geometry->indexCount, indexType, NULL, command->numInstances));
    } else {
        GL(glDrawElements(GL_TRIANGLES, command->pipeline->geometry->indexCount, indexType, NULL));
    }

    commandBuffer->currentGraphicsState = *command;
    commandBuffer->currentTextureUsage = KS_GPU_TEXTURE_USAGE_UNDEFINED;
}

static void ksGpuCommandBuffer_SubmitComputeCommand(ksGpuCommandBuffer *commandBuffer, const ksGpuComputeCommand *command) {
    assert(commandBuffer->currentRenderPass == NULL);

    const ksGpuComputeCommand *state = &commandBuffer->currentComputeState;

    // Compare programs based on a kernel source code hash value to minimize state changes when
    // the same source code is compiled to programs in different locations.
    const bool differentProgram = (state->pipeline == NULL || command->pipeline->program->hash != state->pipeline->program->hash);

    if (differentProgram) {
        GL(glUseProgram(command->pipeline->program->program));
    }

    ksGpuCommandBuffer_UpdateProgramParms(&command->pipeline->program->parmLayout,
                                          (state->pipeline != NULL) ? &state->pipeline->program->parmLayout : NULL,
                                          &command->parmState, &state->parmState, differentProgram);

    GL(glDispatchCompute(command->x, command->y, command->z));

    commandBuffer->currentComputeState = *command;
    commandBuffer->currentTextureUsage = KS_GPU_TEXTURE_USAGE_UNDEFINED;
}

static ksGpuBuffer *ksGpuCommandBuffer_MapBuffer(ksGpuCommandBuffer *commandBuffer, ksGpuBuffer *buffer, void **data) {
    UNUSED_PARM(commandBuffer);

    GL(glBindBuffer(buffer->target, buffer->buffer));
    GL(*data = glMapBufferRange(buffer->target, 0, buffer->size,
                                GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT));
    GL(glBindBuffer(buffer->target, 0));

    return buffer;
}

static void ksGpuCommandBuffer_UnmapBuffer(ksGpuCommandBuffer *commandBuffer, ksGpuBuffer *buffer, ksGpuBuffer *mappedBuffer,
                                           const ksGpuBufferUnmapType type) {
    UNUSED_PARM(commandBuffer);
    UNUSED_PARM(buffer);

    assert(buffer == mappedBuffer);

    GL(glBindBuffer(mappedBuffer->target, mappedBuffer->buffer));
    GL(glUnmapBuffer(mappedBuffer->target));
    GL(glBindBuffer(mappedBuffer->target, 0));

    if (type == KS_GPU_BUFFER_UNMAP_TYPE_COPY_BACK) {
        // Can only copy outside a render pass.
        assert(commandBuffer->currentRenderPass == NULL);
    }
}

static ksGpuBuffer *ksGpuCommandBuffer_MapVertexAttributes(ksGpuCommandBuffer *commandBuffer, ksGpuGeometry *geometry,
                                                           ksGpuVertexAttributeArrays *attribs) {
    void *data = NULL;
    ksGpuBuffer *buffer = ksGpuCommandBuffer_MapBuffer(commandBuffer, &geometry->vertexBuffer, &data);

    attribs->layout = geometry->layout;
    ksGpuVertexAttributeArrays_Map(attribs, data, buffer->size, geometry->vertexCount, geometry->vertexAttribsFlags);

    return buffer;
}

static void ksGpuCommandBuffer_UnmapVertexAttributes(ksGpuCommandBuffer *commandBuffer, ksGpuGeometry *geometry,
                                                     ksGpuBuffer *mappedVertexBuffer, const ksGpuBufferUnmapType type) {
    ksGpuCommandBuffer_UnmapBuffer(commandBuffer, &geometry->vertexBuffer, mappedVertexBuffer, type);
}

static ksGpuBuffer *ksGpuCommandBuffer_MapInstanceAttributes(ksGpuCommandBuffer *commandBuffer, ksGpuGeometry *geometry,
                                                             ksGpuVertexAttributeArrays *attribs) {
    void *data = NULL;
    ksGpuBuffer *buffer = ksGpuCommandBuffer_MapBuffer(commandBuffer, &geometry->instanceBuffer, &data);

    attribs->layout = geometry->layout;
    ksGpuVertexAttributeArrays_Map(attribs, data, buffer->size, geometry->instanceCount, geometry->instanceAttribsFlags);

    return buffer;
}

static void ksGpuCommandBuffer_UnmapInstanceAttributes(ksGpuCommandBuffer *commandBuffer, ksGpuGeometry *geometry,
                                                       ksGpuBuffer *mappedInstanceBuffer, const ksGpuBufferUnmapType type) {
    ksGpuCommandBuffer_UnmapBuffer(commandBuffer, &geometry->instanceBuffer, mappedInstanceBuffer, type);
}

static void ksGpuCommandBuffer_Blit(ksGpuCommandBuffer *commandBuffer, ksGpuFramebuffer *srcFramebuffer,
                                    ksGpuFramebuffer *dstFramebuffer) {
    UNUSED_PARM(commandBuffer);

    ksGpuTexture *srcTexture = &srcFramebuffer->colorTextures[srcFramebuffer->currentBuffer];
    ksGpuTexture *dstTexture = &dstFramebuffer->colorTextures[dstFramebuffer->currentBuffer];
    assert(srcTexture->width == dstTexture->width && srcTexture->height == dstTexture->height);
    UNUSED_PARM(dstTexture);

    GL(glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFramebuffer->renderBuffers[srcFramebuffer->currentBuffer]));
    GL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstFramebuffer->renderBuffers[dstFramebuffer->currentBuffer]));
    GL(glBlitFramebuffer(0, 0, srcTexture->width, srcTexture->height, 0, 0, srcTexture->width, srcTexture->height,
                         GL_COLOR_BUFFER_BIT, GL_NEAREST));
    GL(glBindFramebuffer(GL_READ_FRAMEBUFFER, 0));
    GL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
}

/*
================================================================================================================================

Bar graph.

Real-time bar graph where new bars scroll in on the right and old bars scroll out on the left.
Optionally supports stacking of bars. A bar value is in the range [0, 1] where 1 is a full height bar.
The bar graph position x,y,width,height is specified in clip coordinates in the range [-1, 1].

ksBarGraph

static void ksBarGraph_Create( ksGpuContext * context, ksBarGraph * barGraph, ksGpuRenderPass * renderPass,
                                                                const float x, const float y, const float width, const float height,
                                                                const int numBars, const int numStacked, const ksVector4f *
backgroundColor ); static void ksBarGraph_Destroy( ksGpuContext * context, ksBarGraph * barGraph ); static void ksBarGraph_AddBar(
ksBarGraph * barGraph, const int stackedBar, const float value, const ksVector4f * color, const bool advance );

static void ksBarGraph_UpdateGraphics( ksGpuCommandBuffer * commandBuffer, ksBarGraph * barGraph );
static void ksBarGraph_RenderGraphics( ksGpuCommandBuffer * commandBuffer, ksBarGraph * barGraph );

static void ksBarGraph_UpdateCompute( ksGpuCommandBuffer * commandBuffer, ksBarGraph * barGraph );
static void ksBarGraph_RenderCompute( ksGpuCommandBuffer * commandBuffer, ksBarGraph * barGraph, ksGpuFramebuffer * framebuffer );

================================================================================================================================
*/

typedef struct {
    ksClipRect clipRect;
    int numBars;
    int numStacked;
    int barIndex;
    float *barValues;
    ksVector4f *barColors;
    ksVector4f backgroundColor;
    struct {
        ksGpuGeometry quad;
        ksGpuGraphicsProgram program;
        ksGpuGraphicsPipeline pipeline;
        int numInstances;
    } graphics;
#if OPENGL_COMPUTE_ENABLED == 1
    struct {
        ksGpuBuffer barValueBuffer;
        ksGpuBuffer barColorBuffer;
        ksVector2i barGraphOffset;
        ksGpuComputeProgram program;
        ksGpuComputePipeline pipeline;
    } compute;
#endif
} ksBarGraph;

static const ksGpuProgramParm barGraphGraphicsProgramParms[] = {{0}};

static const char barGraphVertexProgramGLSL[] = "#version " GLSL_VERSION "\n" GLSL_EXTENSIONS
                                                "in vec3 vertexPosition;\n"
                                                "in mat4 vertexTransform;\n"
                                                "out vec4 fragmentColor;\n"
                                                "out gl_PerVertex { vec4 gl_Position; };\n"
                                                "vec3 multiply4x3( mat4 m, vec3 v )\n"
                                                "{\n"
                                                "    return vec3(\n"
                                                "        m[0].x * v.x + m[1].x * v.y + m[2].x * v.z + m[3].x,\n"
                                                "        m[0].y * v.x + m[1].y * v.y + m[2].y * v.z + m[3].y,\n"
                                                "        m[0].z * v.x + m[1].z * v.y + m[2].z * v.z + m[3].z );\n"
                                                "}\n"
                                                "void main( void )\n"
                                                "{\n"
                                                "    gl_Position.xyz = multiply4x3( vertexTransform, vertexPosition );\n"
                                                "    gl_Position.w = 1.0;\n"
                                                "    fragmentColor.r = vertexTransform[0][3];\n"
                                                "    fragmentColor.g = vertexTransform[1][3];\n"
                                                "    fragmentColor.b = vertexTransform[2][3];\n"
                                                "    fragmentColor.a = vertexTransform[3][3];\n"
                                                "}\n";

static const char barGraphFragmentProgramGLSL[] = "#version " GLSL_VERSION "\n" GLSL_EXTENSIONS
                                                  "in lowp vec4 fragmentColor;\n"
                                                  "out lowp vec4 outColor;\n"
                                                  "void main()\n"
                                                  "{\n"
                                                  "    outColor = fragmentColor;\n"
                                                  "}\n";

#if OPENGL_COMPUTE_ENABLED == 1

enum {
    COMPUTE_PROGRAM_TEXTURE_BAR_GRAPH_DEST,
    COMPUTE_PROGRAM_BUFFER_BAR_GRAPH_BAR_VALUES,
    COMPUTE_PROGRAM_BUFFER_BAR_GRAPH_BAR_COLORS,
    COMPUTE_PROGRAM_UNIFORM_BAR_GRAPH_NUM_BARS,
    COMPUTE_PROGRAM_UNIFORM_BAR_GRAPH_NUM_STACKED,
    COMPUTE_PROGRAM_UNIFORM_BAR_GRAPH_BAR_INDEX,
    COMPUTE_PROGRAM_UNIFORM_BAR_GRAPH_BAR_GRAPH_OFFSET,
    COMPUTE_PROGRAM_UNIFORM_BAR_GRAPH_BACK_GROUND_COLOR
};

static const ksGpuProgramParm barGraphComputeProgramParms[] = {
    {KS_GPU_PROGRAM_STAGE_FLAG_COMPUTE, KS_GPU_PROGRAM_PARM_TYPE_TEXTURE_STORAGE, KS_GPU_PROGRAM_PARM_ACCESS_WRITE_ONLY,
     COMPUTE_PROGRAM_TEXTURE_BAR_GRAPH_DEST, "dest", 0},
    {KS_GPU_PROGRAM_STAGE_FLAG_COMPUTE, KS_GPU_PROGRAM_PARM_TYPE_BUFFER_STORAGE, KS_GPU_PROGRAM_PARM_ACCESS_READ_ONLY,
     COMPUTE_PROGRAM_BUFFER_BAR_GRAPH_BAR_VALUES, "barValueBuffer", 0},
    {KS_GPU_PROGRAM_STAGE_FLAG_COMPUTE, KS_GPU_PROGRAM_PARM_TYPE_BUFFER_STORAGE, KS_GPU_PROGRAM_PARM_ACCESS_READ_ONLY,
     COMPUTE_PROGRAM_BUFFER_BAR_GRAPH_BAR_COLORS, "barColorBuffer", 1},
    {KS_GPU_PROGRAM_STAGE_FLAG_COMPUTE, KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_VECTOR4, KS_GPU_PROGRAM_PARM_ACCESS_READ_ONLY,
     COMPUTE_PROGRAM_UNIFORM_BAR_GRAPH_BACK_GROUND_COLOR, "backgroundColor", 0},
    {KS_GPU_PROGRAM_STAGE_FLAG_COMPUTE, KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_INT_VECTOR2, KS_GPU_PROGRAM_PARM_ACCESS_READ_ONLY,
     COMPUTE_PROGRAM_UNIFORM_BAR_GRAPH_BAR_GRAPH_OFFSET, "barGraphOffset", 1},
    {KS_GPU_PROGRAM_STAGE_FLAG_COMPUTE, KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_INT, KS_GPU_PROGRAM_PARM_ACCESS_READ_ONLY,
     COMPUTE_PROGRAM_UNIFORM_BAR_GRAPH_NUM_BARS, "numBars", 2},
    {KS_GPU_PROGRAM_STAGE_FLAG_COMPUTE, KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_INT, KS_GPU_PROGRAM_PARM_ACCESS_READ_ONLY,
     COMPUTE_PROGRAM_UNIFORM_BAR_GRAPH_NUM_STACKED, "numStacked", 3},
    {KS_GPU_PROGRAM_STAGE_FLAG_COMPUTE, KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_INT, KS_GPU_PROGRAM_PARM_ACCESS_READ_ONLY,
     COMPUTE_PROGRAM_UNIFORM_BAR_GRAPH_BAR_INDEX, "barIndex", 4}};

#define BARGRAPH_LOCAL_SIZE_X 8
#define BARGRAPH_LOCAL_SIZE_Y 8

static const char barGraphComputeProgramGLSL[] =
    "#version " GLSL_VERSION "\n"
    GLSL_EXTENSIONS
    "\n"
    "layout( local_size_x = " STRINGIFY( BARGRAPH_LOCAL_SIZE_X ) ", local_size_y = " STRINGIFY( BARGRAPH_LOCAL_SIZE_Y ) " ) in;\n"
    "\n"
    "layout( rgba8, binding = 0 ) uniform writeonly " ES_HIGHP " image2D dest;\n"
    "layout( std430, binding = 0 ) buffer barValueBuffer { float barValues[]; };\n"
    "layout( std430, binding = 1 ) buffer barColorBuffer { vec4 barColors[]; };\n"
    "uniform lowp vec4 backgroundColor;\n"
    "uniform ivec2 barGraphOffset;\n"
    "uniform int numBars;\n"
    "uniform int numStacked;\n"
     "uniform int barIndex;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    ivec2 barGraph = ivec2( gl_GlobalInvocationID.xy );\n"
    "    ivec2 barGraphSize = ivec2( gl_NumWorkGroups.xy * gl_WorkGroupSize.xy );\n"
    "\n"
    "    int index = barGraph.x * numBars / barGraphSize.x;\n"
    "    int barOffset = ( ( barIndex + index ) % numBars ) * numStacked;\n"
    "    float barColorScale = ( ( index & 1 ) != 0 ) ? 0.75f : 1.0f;\n"
    "\n"
    "    vec4 rgba = backgroundColor;\n"
    "    float localY = float( barGraph.y );\n"
    "    float stackedBarValue = 0.0f;\n"
    "    for ( int i = 0; i < numStacked; i++ )\n"
    "    {\n"
    "        stackedBarValue += barValues[barOffset + i];\n"
    "        if ( localY < stackedBarValue * float( barGraphSize.y ) )\n"
    "        {\n"
    "            rgba = barColors[barOffset + i] * barColorScale;\n"
    "            break;\n"
    "        }\n"
    "    }\n"
    "\n"
    "    imageStore( dest, barGraphOffset + barGraph, rgba );\n"
    "}\n";

#endif

static void ksBarGraph_Create(ksGpuContext *context, ksBarGraph *barGraph, ksGpuRenderPass *renderPass, const float x,
                              const float y, const float width, const float height, const int numBars, const int numStacked,
                              const ksVector4f *backgroundColor) {
    barGraph->clipRect.x = x;
    barGraph->clipRect.y = y;
    barGraph->clipRect.width = width;
    barGraph->clipRect.height = height;
    barGraph->numBars = numBars;
    barGraph->numStacked = numStacked;
    barGraph->barIndex = 0;
    barGraph->barValues = (float *)AllocAlignedMemory(numBars * numStacked * sizeof(barGraph->barValues[0]), sizeof(void *));
    barGraph->barColors =
        (ksVector4f *)AllocAlignedMemory(numBars * numStacked * sizeof(barGraph->barColors[0]), sizeof(ksVector4f));

    for (int i = 0; i < numBars * numStacked; i++) {
        barGraph->barValues[i] = 0.0f;
        barGraph->barColors[i] = ksColorGreen;
    }

    barGraph->backgroundColor = *backgroundColor;

    // graphics
    {
        ksGpuGeometry_CreateQuad(context, &barGraph->graphics.quad, 1.0f, 0.5f);
        ksGpuGeometry_AddInstanceAttributes(context, &barGraph->graphics.quad, numBars * numStacked + 1,
                                            VERTEX_ATTRIBUTE_FLAG_TRANSFORM);

        ksGpuGraphicsProgram_Create(
            context, &barGraph->graphics.program, PROGRAM(barGraphVertexProgram), sizeof(PROGRAM(barGraphVertexProgram)),
            PROGRAM(barGraphFragmentProgram), sizeof(PROGRAM(barGraphFragmentProgram)), barGraphGraphicsProgramParms, 0,
            barGraph->graphics.quad.layout, VERTEX_ATTRIBUTE_FLAG_POSITION | VERTEX_ATTRIBUTE_FLAG_TRANSFORM);

        ksGpuGraphicsPipelineParms pipelineParms;
        ksGpuGraphicsPipelineParms_Init(&pipelineParms);

        pipelineParms.rop.depthTestEnable = false;
        pipelineParms.rop.depthWriteEnable = false;
        pipelineParms.renderPass = renderPass;
        pipelineParms.program = &barGraph->graphics.program;
        pipelineParms.geometry = &barGraph->graphics.quad;

        ksGpuGraphicsPipeline_Create(context, &barGraph->graphics.pipeline, &pipelineParms);

        barGraph->graphics.numInstances = 0;
    }

#if OPENGL_COMPUTE_ENABLED == 1
    // compute
    {
        ksGpuBuffer_Create(context, &barGraph->compute.barValueBuffer, KS_GPU_BUFFER_TYPE_STORAGE,
                           barGraph->numBars * barGraph->numStacked * sizeof(barGraph->barValues[0]), NULL, false);
        ksGpuBuffer_Create(context, &barGraph->compute.barColorBuffer, KS_GPU_BUFFER_TYPE_STORAGE,
                           barGraph->numBars * barGraph->numStacked * sizeof(barGraph->barColors[0]), NULL, false);

        ksGpuComputeProgram_Create(context, &barGraph->compute.program, PROGRAM(barGraphComputeProgram),
                                   sizeof(PROGRAM(barGraphComputeProgram)), barGraphComputeProgramParms,
                                   ARRAY_SIZE(barGraphComputeProgramParms));

        ksGpuComputePipeline_Create(context, &barGraph->compute.pipeline, &barGraph->compute.program);
    }
#endif
}

static void ksBarGraph_Destroy(ksGpuContext *context, ksBarGraph *barGraph) {
    FreeAlignedMemory(barGraph->barValues);
    FreeAlignedMemory(barGraph->barColors);

    // graphics
    {
        ksGpuGraphicsPipeline_Destroy(context, &barGraph->graphics.pipeline);
        ksGpuGraphicsProgram_Destroy(context, &barGraph->graphics.program);
        ksGpuGeometry_Destroy(context, &barGraph->graphics.quad);
    }

#if OPENGL_COMPUTE_ENABLED == 1
    // compute
    {
        ksGpuComputePipeline_Destroy(context, &barGraph->compute.pipeline);
        ksGpuComputeProgram_Destroy(context, &barGraph->compute.program);
        ksGpuBuffer_Destroy(context, &barGraph->compute.barValueBuffer);
        ksGpuBuffer_Destroy(context, &barGraph->compute.barColorBuffer);
    }
#endif
}

static void ksBarGraph_AddBar(ksBarGraph *barGraph, const int stackedBar, const float value, const ksVector4f *color,
                              const bool advance) {
    assert(stackedBar >= 0 && stackedBar < barGraph->numStacked);
    barGraph->barValues[barGraph->barIndex * barGraph->numStacked + stackedBar] = value;
    barGraph->barColors[barGraph->barIndex * barGraph->numStacked + stackedBar] = *color;
    if (advance) {
        barGraph->barIndex = (barGraph->barIndex + 1) % barGraph->numBars;
    }
}

static void ksBarGraph_UpdateGraphics(ksGpuCommandBuffer *commandBuffer, ksBarGraph *barGraph) {
    ksDefaultVertexAttributeArrays attribs;
    ksGpuBuffer *instanceBuffer = ksGpuCommandBuffer_MapInstanceAttributes(commandBuffer, &barGraph->graphics.quad, &attribs.base);

#if defined(GRAPHICS_API_VULKAN)
    const float flipY = -1.0f;
#else
    const float flipY = 1.0f;
#endif

    int numInstances = 0;
    ksMatrix4x4f *backgroundMatrix = &attribs.transform[numInstances++];

    // Write in order to write-combined memory.
    backgroundMatrix->m[0][0] = barGraph->clipRect.width;
    backgroundMatrix->m[0][1] = 0.0f;
    backgroundMatrix->m[0][2] = 0.0f;
    backgroundMatrix->m[0][3] = barGraph->backgroundColor.x;

    backgroundMatrix->m[1][0] = 0.0f;
    backgroundMatrix->m[1][1] = barGraph->clipRect.height * flipY;
    backgroundMatrix->m[1][2] = 0.0f;
    backgroundMatrix->m[1][3] = barGraph->backgroundColor.y;

    backgroundMatrix->m[2][0] = 0.0f;
    backgroundMatrix->m[2][1] = 0.0f;
    backgroundMatrix->m[2][2] = 0.0f;
    backgroundMatrix->m[2][3] = barGraph->backgroundColor.z;

    backgroundMatrix->m[3][0] = barGraph->clipRect.x;
    backgroundMatrix->m[3][1] = barGraph->clipRect.y * flipY;
    backgroundMatrix->m[3][2] = 0.0f;
    backgroundMatrix->m[3][3] = barGraph->backgroundColor.w;

    const float barWidth = barGraph->clipRect.width / barGraph->numBars;

    for (int i = 0; i < barGraph->numBars; i++) {
        const int barIndex = ((barGraph->barIndex + i) % barGraph->numBars) * barGraph->numStacked;
        const float barColorScale = (i & 1) ? 0.75f : 1.0f;

        float stackedBarValue = 0.0f;
        for (int j = 0; j < barGraph->numStacked; j++) {
            float value = barGraph->barValues[barIndex + j];
            if (stackedBarValue + value > 1.0f) {
                value = 1.0f - stackedBarValue;
            }
            if (value <= 0.0f) {
                continue;
            }

            ksMatrix4x4f *barMatrix = &attribs.transform[numInstances++];

            // Write in order to write-combined memory.
            barMatrix->m[0][0] = barWidth;
            barMatrix->m[0][1] = 0.0f;
            barMatrix->m[0][2] = 0.0f;
            barMatrix->m[0][3] = barGraph->barColors[barIndex + j].x * barColorScale;

            barMatrix->m[1][0] = 0.0f;
            barMatrix->m[1][1] = value * barGraph->clipRect.height * flipY;
            barMatrix->m[1][2] = 0.0f;
            barMatrix->m[1][3] = barGraph->barColors[barIndex + j].y * barColorScale;

            barMatrix->m[2][0] = 0.0f;
            barMatrix->m[2][1] = 0.0f;
            barMatrix->m[2][2] = 1.0f;
            barMatrix->m[2][3] = barGraph->barColors[barIndex + j].z * barColorScale;

            barMatrix->m[3][0] = barGraph->clipRect.x + i * barWidth;
            barMatrix->m[3][1] = (barGraph->clipRect.y + stackedBarValue * barGraph->clipRect.height) * flipY;
            barMatrix->m[3][2] = 0.0f;
            barMatrix->m[3][3] = barGraph->barColors[barIndex + j].w;

            stackedBarValue += value;
        }
    }

    ksGpuCommandBuffer_UnmapInstanceAttributes(commandBuffer, &barGraph->graphics.quad, instanceBuffer,
                                               KS_GPU_BUFFER_UNMAP_TYPE_COPY_BACK);

    assert(numInstances <= barGraph->numBars * barGraph->numStacked + 1);
    barGraph->graphics.numInstances = numInstances;
}

static void ksBarGraph_RenderGraphics(ksGpuCommandBuffer *commandBuffer, ksBarGraph *barGraph) {
    ksGpuGraphicsCommand command;
    ksGpuGraphicsCommand_Init(&command);
    ksGpuGraphicsCommand_SetPipeline(&command, &barGraph->graphics.pipeline);
    ksGpuGraphicsCommand_SetNumInstances(&command, barGraph->graphics.numInstances);

    ksGpuCommandBuffer_SubmitGraphicsCommand(commandBuffer, &command);
}

static void ksBarGraph_UpdateCompute(ksGpuCommandBuffer *commandBuffer, ksBarGraph *barGraph) {
#if OPENGL_COMPUTE_ENABLED == 1
    void *barValues = NULL;
    ksGpuBuffer *mappedBarValueBuffer = ksGpuCommandBuffer_MapBuffer(commandBuffer, &barGraph->compute.barValueBuffer, &barValues);
    memcpy(barValues, barGraph->barValues, barGraph->numBars * barGraph->numStacked * sizeof(barGraph->barValues[0]));
    ksGpuCommandBuffer_UnmapBuffer(commandBuffer, &barGraph->compute.barValueBuffer, mappedBarValueBuffer,
                                   KS_GPU_BUFFER_UNMAP_TYPE_COPY_BACK);

    void *barColors = NULL;
    ksGpuBuffer *mappedBarColorBuffer = ksGpuCommandBuffer_MapBuffer(commandBuffer, &barGraph->compute.barColorBuffer, &barColors);
    memcpy(barColors, barGraph->barColors, barGraph->numBars * barGraph->numStacked * sizeof(barGraph->barColors[0]));
    ksGpuCommandBuffer_UnmapBuffer(commandBuffer, &barGraph->compute.barColorBuffer, mappedBarColorBuffer,
                                   KS_GPU_BUFFER_UNMAP_TYPE_COPY_BACK);
#else
    UNUSED_PARM(commandBuffer);
    UNUSED_PARM(barGraph);
#endif
}

static void ksBarGraph_RenderCompute(ksGpuCommandBuffer *commandBuffer, ksBarGraph *barGraph, ksGpuFramebuffer *framebuffer) {
#if OPENGL_COMPUTE_ENABLED == 1
    const int screenWidth = ksGpuFramebuffer_GetWidth(framebuffer);
    const int screenHeight = ksGpuFramebuffer_GetHeight(framebuffer);
    ksScreenRect screenRect = ksClipRect_ToScreenRect(&barGraph->clipRect, screenWidth, screenHeight);
    barGraph->compute.barGraphOffset.x = screenRect.x;
#if defined(GRAPHICS_API_VULKAN)
    barGraph->compute.barGraphOffset.y = screenHeight - 1 - screenRect.y;
#else
    barGraph->compute.barGraphOffset.y = screenRect.y;
#endif

    screenRect.width = ROUNDUP(screenRect.width, 8);
    screenRect.height = ROUNDUP(screenRect.height, 8);

    assert(screenRect.width % BARGRAPH_LOCAL_SIZE_X == 0);
    assert(screenRect.height % BARGRAPH_LOCAL_SIZE_Y == 0);

    ksGpuComputeCommand command;
    ksGpuComputeCommand_Init(&command);
    ksGpuComputeCommand_SetPipeline(&command, &barGraph->compute.pipeline);
    ksGpuComputeCommand_SetParmTextureStorage(&command, COMPUTE_PROGRAM_TEXTURE_BAR_GRAPH_DEST,
                                              ksGpuFramebuffer_GetColorTexture(framebuffer));
    ksGpuComputeCommand_SetParmBufferStorage(&command, COMPUTE_PROGRAM_BUFFER_BAR_GRAPH_BAR_VALUES,
                                             &barGraph->compute.barValueBuffer);
    ksGpuComputeCommand_SetParmBufferStorage(&command, COMPUTE_PROGRAM_BUFFER_BAR_GRAPH_BAR_COLORS,
                                             &barGraph->compute.barColorBuffer);
    ksGpuComputeCommand_SetParmFloatVector4(&command, COMPUTE_PROGRAM_UNIFORM_BAR_GRAPH_BACK_GROUND_COLOR,
                                            &barGraph->backgroundColor);
    ksGpuComputeCommand_SetParmIntVector2(&command, COMPUTE_PROGRAM_UNIFORM_BAR_GRAPH_BAR_GRAPH_OFFSET,
                                          &barGraph->compute.barGraphOffset);
    ksGpuComputeCommand_SetParmInt(&command, COMPUTE_PROGRAM_UNIFORM_BAR_GRAPH_NUM_BARS, &barGraph->numBars);
    ksGpuComputeCommand_SetParmInt(&command, COMPUTE_PROGRAM_UNIFORM_BAR_GRAPH_NUM_STACKED, &barGraph->numStacked);
    ksGpuComputeCommand_SetParmInt(&command, COMPUTE_PROGRAM_UNIFORM_BAR_GRAPH_BAR_INDEX, &barGraph->barIndex);
    ksGpuComputeCommand_SetDimensions(&command, screenRect.width / BARGRAPH_LOCAL_SIZE_X, screenRect.height / BARGRAPH_LOCAL_SIZE_Y,
                                      1);

    ksGpuCommandBuffer_SubmitComputeCommand(commandBuffer, &command);
#else
    UNUSED_PARM(commandBuffer);
    UNUSED_PARM(barGraph);
    UNUSED_PARM(framebuffer);
#endif
}

/*
================================================================================================================================

Time warp bar graphs.

ksTimeWarpBarGraphs

static void ksTimeWarpBarGraphs_Create( ksGpuContext * context, ksTimeWarpBarGraphs * bargraphs, ksGpuFramebuffer * framebuffer );
static void ksTimeWarpBarGraphs_Destroy( ksGpuContext * context, ksTimeWarpBarGraphs * bargraphs );

static void ksTimeWarpBarGraphs_UpdateGraphics( ksGpuCommandBuffer * commandBuffer, ksTimeWarpBarGraphs * bargraphs );
static void ksTimeWarpBarGraphs_RenderGraphics( ksGpuCommandBuffer * commandBuffer, ksTimeWarpBarGraphs * bargraphs );

static void ksTimeWarpBarGraphs_UpdateCompute( ksGpuCommandBuffer * commandBuffer, ksTimeWarpBarGraphs * bargraphs );
static void ksTimeWarpBarGraphs_RenderCompute( ksGpuCommandBuffer * commandBuffer, ksTimeWarpBarGraphs * bargraphs, ksGpuFramebuffer
* framebuffer );

static ksNanoseconds ksTimeWarpBarGraphs_GetGpuNanosecondsGraphics( ksTimeWarpBarGraphs * bargraphs );
static ksNanoseconds ksTimeWarpBarGraphs_GetGpuNanosecondsCompute( ksTimeWarpBarGraphs * bargraphs );

================================================================================================================================
*/

#define BARGRAPH_VIRTUAL_PIXELS_WIDE 1920
#define BARGRAPH_VIRTUAL_PIXELS_HIGH 1080

#if defined(OS_ANDROID)
#define BARGRAPH_INSET 64
#else
#define BARGRAPH_INSET 16
#endif

static const ksScreenRect eyeTextureFrameRateBarGraphRect = {BARGRAPH_INSET + 0 * 264, BARGRAPH_INSET, 256, 128};
static const ksScreenRect timeWarpFrameRateBarGraphRect = {BARGRAPH_INSET + 1 * 264, BARGRAPH_INSET, 256, 128};
static const ksScreenRect frameCpuTimeBarGraphRect = {BARGRAPH_INSET + 2 * 264, BARGRAPH_INSET, 256, 128};
static const ksScreenRect frameGpuTimeBarGraphRect = {BARGRAPH_INSET + 3 * 264, BARGRAPH_INSET, 256, 128};

static const ksScreenRect multiViewBarGraphRect = {2 * BARGRAPH_VIRTUAL_PIXELS_WIDE / 3 + 0 * 40, BARGRAPH_INSET, 32, 32};
static const ksScreenRect correctChromaticAberrationBarGraphRect = {2 * BARGRAPH_VIRTUAL_PIXELS_WIDE / 3 + 1 * 40, BARGRAPH_INSET,
                                                                    32, 32};
static const ksScreenRect timeWarpImplementationBarGraphRect = {2 * BARGRAPH_VIRTUAL_PIXELS_WIDE / 3 + 2 * 40, BARGRAPH_INSET, 32,
                                                                32};

static const ksScreenRect displayResolutionLevelBarGraphRect = {BARGRAPH_VIRTUAL_PIXELS_WIDE - 7 * 40 - BARGRAPH_INSET,
                                                                BARGRAPH_INSET, 32, 128};
static const ksScreenRect eyeImageResolutionLevelBarGraphRect = {BARGRAPH_VIRTUAL_PIXELS_WIDE - 6 * 40 - BARGRAPH_INSET,
                                                                 BARGRAPH_INSET, 32, 128};
static const ksScreenRect eyeImageSamplesLevelBarGraphRect = {BARGRAPH_VIRTUAL_PIXELS_WIDE - 5 * 40 - BARGRAPH_INSET,
                                                              BARGRAPH_INSET, 32, 128};

static const ksScreenRect sceneDrawCallLevelBarGraphRect = {BARGRAPH_VIRTUAL_PIXELS_WIDE - 3 * 40 - BARGRAPH_INSET, BARGRAPH_INSET,
                                                            32, 128};
static const ksScreenRect sceneTriangleLevelBarGraphRect = {BARGRAPH_VIRTUAL_PIXELS_WIDE - 2 * 40 - BARGRAPH_INSET, BARGRAPH_INSET,
                                                            32, 128};
static const ksScreenRect sceneFragmentLevelBarGraphRect = {BARGRAPH_VIRTUAL_PIXELS_WIDE - 1 * 40 - BARGRAPH_INSET, BARGRAPH_INSET,
                                                            32, 128};

typedef enum { BAR_GRAPH_HIDDEN, BAR_GRAPH_VISIBLE, BAR_GRAPH_PAUSED } ksBarGraphState;

typedef struct {
    ksBarGraphState barGraphState;

    ksBarGraph applicationFrameRateGraph;
    ksBarGraph timeWarpFrameRateGraph;
    ksBarGraph frameCpuTimeBarGraph;
    ksBarGraph frameGpuTimeBarGraph;

    ksBarGraph multiViewBarGraph;
    ksBarGraph correctChromaticAberrationBarGraph;
    ksBarGraph timeWarpImplementationBarGraph;

    ksBarGraph displayResolutionLevelBarGraph;
    ksBarGraph eyeImageResolutionLevelBarGraph;
    ksBarGraph eyeImageSamplesLevelBarGraph;

    ksBarGraph sceneDrawCallLevelBarGraph;
    ksBarGraph sceneTriangleLevelBarGraph;
    ksBarGraph sceneFragmentLevelBarGraph;

    ksGpuTimer barGraphTimer;
} ksTimeWarpBarGraphs;

enum {
    PROFILE_TIME_APPLICATION,
    PROFILE_TIME_TIME_WARP,
    PROFILE_TIME_BAR_GRAPHS,
    PROFILE_TIME_BLIT,
    PROFILE_TIME_OVERFLOW,
    PROFILE_TIME_MAX
};

static const ksVector4f *profileTimeBarColors[] = {&ksColorPurple, &ksColorGreen, &ksColorYellow, &ksColorBlue, &ksColorRed};

static void ksBarGraph_CreateVirtualRect(ksGpuContext *context, ksBarGraph *barGraph, ksGpuRenderPass *renderPass,
                                         const ksScreenRect *virtualRect, const int numBars, const int numStacked,
                                         const ksVector4f *backgroundColor) {
    const ksClipRect clipRect = ksScreenRect_ToClipRect(virtualRect, BARGRAPH_VIRTUAL_PIXELS_WIDE, BARGRAPH_VIRTUAL_PIXELS_HIGH);
    ksBarGraph_Create(context, barGraph, renderPass, clipRect.x, clipRect.y, clipRect.width, clipRect.height, numBars, numStacked,
                      backgroundColor);
}

static void ksTimeWarpBarGraphs_Create(ksGpuContext *context, ksTimeWarpBarGraphs *bargraphs, ksGpuRenderPass *renderPass) {
    bargraphs->barGraphState = BAR_GRAPH_VISIBLE;

    ksBarGraph_CreateVirtualRect(context, &bargraphs->applicationFrameRateGraph, renderPass, &eyeTextureFrameRateBarGraphRect, 64,
                                 1, &ksColorDarkGrey);
    ksBarGraph_CreateVirtualRect(context, &bargraphs->timeWarpFrameRateGraph, renderPass, &timeWarpFrameRateBarGraphRect, 64, 1,
                                 &ksColorDarkGrey);
    ksBarGraph_CreateVirtualRect(context, &bargraphs->frameCpuTimeBarGraph, renderPass, &frameCpuTimeBarGraphRect, 64,
                                 PROFILE_TIME_MAX, &ksColorDarkGrey);
    ksBarGraph_CreateVirtualRect(context, &bargraphs->frameGpuTimeBarGraph, renderPass, &frameGpuTimeBarGraphRect, 64,
                                 PROFILE_TIME_MAX, &ksColorDarkGrey);

    ksBarGraph_CreateVirtualRect(context, &bargraphs->multiViewBarGraph, renderPass, &multiViewBarGraphRect, 1, 1,
                                 &ksColorDarkGrey);
    ksBarGraph_CreateVirtualRect(context, &bargraphs->correctChromaticAberrationBarGraph, renderPass,
                                 &correctChromaticAberrationBarGraphRect, 1, 1, &ksColorDarkGrey);
    ksBarGraph_CreateVirtualRect(context, &bargraphs->timeWarpImplementationBarGraph, renderPass,
                                 &timeWarpImplementationBarGraphRect, 1, 1, &ksColorDarkGrey);

    ksBarGraph_CreateVirtualRect(context, &bargraphs->displayResolutionLevelBarGraph, renderPass,
                                 &displayResolutionLevelBarGraphRect, 1, 4, &ksColorDarkGrey);
    ksBarGraph_CreateVirtualRect(context, &bargraphs->eyeImageResolutionLevelBarGraph, renderPass,
                                 &eyeImageResolutionLevelBarGraphRect, 1, 4, &ksColorDarkGrey);
    ksBarGraph_CreateVirtualRect(context, &bargraphs->eyeImageSamplesLevelBarGraph, renderPass, &eyeImageSamplesLevelBarGraphRect,
                                 1, 4, &ksColorDarkGrey);

    ksBarGraph_CreateVirtualRect(context, &bargraphs->sceneDrawCallLevelBarGraph, renderPass, &sceneDrawCallLevelBarGraphRect, 1, 4,
                                 &ksColorDarkGrey);
    ksBarGraph_CreateVirtualRect(context, &bargraphs->sceneTriangleLevelBarGraph, renderPass, &sceneTriangleLevelBarGraphRect, 1, 4,
                                 &ksColorDarkGrey);
    ksBarGraph_CreateVirtualRect(context, &bargraphs->sceneFragmentLevelBarGraph, renderPass, &sceneFragmentLevelBarGraphRect, 1, 4,
                                 &ksColorDarkGrey);

    ksBarGraph_AddBar(&bargraphs->displayResolutionLevelBarGraph, 0, 0.25f, &ksColorBlue, false);
    ksBarGraph_AddBar(&bargraphs->eyeImageResolutionLevelBarGraph, 0, 0.25f, &ksColorBlue, false);
    ksBarGraph_AddBar(&bargraphs->eyeImageSamplesLevelBarGraph, 0, 0.25f, &ksColorBlue, false);

    ksBarGraph_AddBar(&bargraphs->sceneDrawCallLevelBarGraph, 0, 0.25f, &ksColorBlue, false);
    ksBarGraph_AddBar(&bargraphs->sceneTriangleLevelBarGraph, 0, 0.25f, &ksColorBlue, false);
    ksBarGraph_AddBar(&bargraphs->sceneFragmentLevelBarGraph, 0, 0.25f, &ksColorBlue, false);

    ksGpuTimer_Create(context, &bargraphs->barGraphTimer);
}

static void ksTimeWarpBarGraphs_Destroy(ksGpuContext *context, ksTimeWarpBarGraphs *bargraphs) {
    ksBarGraph_Destroy(context, &bargraphs->applicationFrameRateGraph);
    ksBarGraph_Destroy(context, &bargraphs->timeWarpFrameRateGraph);
    ksBarGraph_Destroy(context, &bargraphs->frameCpuTimeBarGraph);
    ksBarGraph_Destroy(context, &bargraphs->frameGpuTimeBarGraph);

    ksBarGraph_Destroy(context, &bargraphs->multiViewBarGraph);
    ksBarGraph_Destroy(context, &bargraphs->correctChromaticAberrationBarGraph);
    ksBarGraph_Destroy(context, &bargraphs->timeWarpImplementationBarGraph);

    ksBarGraph_Destroy(context, &bargraphs->displayResolutionLevelBarGraph);
    ksBarGraph_Destroy(context, &bargraphs->eyeImageResolutionLevelBarGraph);
    ksBarGraph_Destroy(context, &bargraphs->eyeImageSamplesLevelBarGraph);

    ksBarGraph_Destroy(context, &bargraphs->sceneDrawCallLevelBarGraph);
    ksBarGraph_Destroy(context, &bargraphs->sceneTriangleLevelBarGraph);
    ksBarGraph_Destroy(context, &bargraphs->sceneFragmentLevelBarGraph);

    ksGpuTimer_Destroy(context, &bargraphs->barGraphTimer);
}

static void ksTimeWarpBarGraphs_UpdateGraphics(ksGpuCommandBuffer *commandBuffer, ksTimeWarpBarGraphs *bargraphs) {
    if (bargraphs->barGraphState != BAR_GRAPH_HIDDEN) {
        ksBarGraph_UpdateGraphics(commandBuffer, &bargraphs->applicationFrameRateGraph);
        ksBarGraph_UpdateGraphics(commandBuffer, &bargraphs->timeWarpFrameRateGraph);
        ksBarGraph_UpdateGraphics(commandBuffer, &bargraphs->frameCpuTimeBarGraph);
        ksBarGraph_UpdateGraphics(commandBuffer, &bargraphs->frameGpuTimeBarGraph);

        ksBarGraph_UpdateGraphics(commandBuffer, &bargraphs->multiViewBarGraph);
        ksBarGraph_UpdateGraphics(commandBuffer, &bargraphs->correctChromaticAberrationBarGraph);
        ksBarGraph_UpdateGraphics(commandBuffer, &bargraphs->timeWarpImplementationBarGraph);

        ksBarGraph_UpdateGraphics(commandBuffer, &bargraphs->displayResolutionLevelBarGraph);
        ksBarGraph_UpdateGraphics(commandBuffer, &bargraphs->eyeImageResolutionLevelBarGraph);
        ksBarGraph_UpdateGraphics(commandBuffer, &bargraphs->eyeImageSamplesLevelBarGraph);

        ksBarGraph_UpdateGraphics(commandBuffer, &bargraphs->sceneDrawCallLevelBarGraph);
        ksBarGraph_UpdateGraphics(commandBuffer, &bargraphs->sceneTriangleLevelBarGraph);
        ksBarGraph_UpdateGraphics(commandBuffer, &bargraphs->sceneFragmentLevelBarGraph);
    }
}

static void ksTimeWarpBarGraphs_RenderGraphics(ksGpuCommandBuffer *commandBuffer, ksTimeWarpBarGraphs *bargraphs) {
    if (bargraphs->barGraphState != BAR_GRAPH_HIDDEN) {
        ksGpuCommandBuffer_BeginTimer(commandBuffer, &bargraphs->barGraphTimer);

        ksBarGraph_RenderGraphics(commandBuffer, &bargraphs->applicationFrameRateGraph);
        ksBarGraph_RenderGraphics(commandBuffer, &bargraphs->timeWarpFrameRateGraph);
        ksBarGraph_RenderGraphics(commandBuffer, &bargraphs->frameCpuTimeBarGraph);
        ksBarGraph_RenderGraphics(commandBuffer, &bargraphs->frameGpuTimeBarGraph);

        ksBarGraph_RenderGraphics(commandBuffer, &bargraphs->multiViewBarGraph);
        ksBarGraph_RenderGraphics(commandBuffer, &bargraphs->correctChromaticAberrationBarGraph);
        ksBarGraph_RenderGraphics(commandBuffer, &bargraphs->timeWarpImplementationBarGraph);

        ksBarGraph_RenderGraphics(commandBuffer, &bargraphs->displayResolutionLevelBarGraph);
        ksBarGraph_RenderGraphics(commandBuffer, &bargraphs->eyeImageResolutionLevelBarGraph);
        ksBarGraph_RenderGraphics(commandBuffer, &bargraphs->eyeImageSamplesLevelBarGraph);

        ksBarGraph_RenderGraphics(commandBuffer, &bargraphs->sceneDrawCallLevelBarGraph);
        ksBarGraph_RenderGraphics(commandBuffer, &bargraphs->sceneTriangleLevelBarGraph);
        ksBarGraph_RenderGraphics(commandBuffer, &bargraphs->sceneFragmentLevelBarGraph);

        ksGpuCommandBuffer_EndTimer(commandBuffer, &bargraphs->barGraphTimer);
    }
}

static void ksTimeWarpBarGraphs_UpdateCompute(ksGpuCommandBuffer *commandBuffer, ksTimeWarpBarGraphs *bargraphs) {
    if (bargraphs->barGraphState != BAR_GRAPH_HIDDEN) {
        ksBarGraph_UpdateCompute(commandBuffer, &bargraphs->applicationFrameRateGraph);
        ksBarGraph_UpdateCompute(commandBuffer, &bargraphs->timeWarpFrameRateGraph);
        ksBarGraph_UpdateCompute(commandBuffer, &bargraphs->frameCpuTimeBarGraph);
        ksBarGraph_UpdateCompute(commandBuffer, &bargraphs->frameGpuTimeBarGraph);

        ksBarGraph_UpdateCompute(commandBuffer, &bargraphs->multiViewBarGraph);
        ksBarGraph_UpdateCompute(commandBuffer, &bargraphs->correctChromaticAberrationBarGraph);
        ksBarGraph_UpdateCompute(commandBuffer, &bargraphs->timeWarpImplementationBarGraph);

        ksBarGraph_UpdateCompute(commandBuffer, &bargraphs->displayResolutionLevelBarGraph);
        ksBarGraph_UpdateCompute(commandBuffer, &bargraphs->eyeImageResolutionLevelBarGraph);
        ksBarGraph_UpdateCompute(commandBuffer, &bargraphs->eyeImageSamplesLevelBarGraph);

        ksBarGraph_UpdateCompute(commandBuffer, &bargraphs->sceneDrawCallLevelBarGraph);
        ksBarGraph_UpdateCompute(commandBuffer, &bargraphs->sceneTriangleLevelBarGraph);
        ksBarGraph_UpdateCompute(commandBuffer, &bargraphs->sceneFragmentLevelBarGraph);
    }
}

static void ksTimeWarpBarGraphs_RenderCompute(ksGpuCommandBuffer *commandBuffer, ksTimeWarpBarGraphs *bargraphs,
                                              ksGpuFramebuffer *framebuffer) {
    if (bargraphs->barGraphState != BAR_GRAPH_HIDDEN) {
        ksGpuCommandBuffer_BeginTimer(commandBuffer, &bargraphs->barGraphTimer);

        ksBarGraph_RenderCompute(commandBuffer, &bargraphs->applicationFrameRateGraph, framebuffer);
        ksBarGraph_RenderCompute(commandBuffer, &bargraphs->timeWarpFrameRateGraph, framebuffer);
        ksBarGraph_RenderCompute(commandBuffer, &bargraphs->frameCpuTimeBarGraph, framebuffer);
        ksBarGraph_RenderCompute(commandBuffer, &bargraphs->frameGpuTimeBarGraph, framebuffer);

        ksBarGraph_RenderCompute(commandBuffer, &bargraphs->multiViewBarGraph, framebuffer);
        ksBarGraph_RenderCompute(commandBuffer, &bargraphs->correctChromaticAberrationBarGraph, framebuffer);
        ksBarGraph_RenderCompute(commandBuffer, &bargraphs->timeWarpImplementationBarGraph, framebuffer);

        ksBarGraph_RenderCompute(commandBuffer, &bargraphs->displayResolutionLevelBarGraph, framebuffer);
        ksBarGraph_RenderCompute(commandBuffer, &bargraphs->eyeImageResolutionLevelBarGraph, framebuffer);
        ksBarGraph_RenderCompute(commandBuffer, &bargraphs->eyeImageSamplesLevelBarGraph, framebuffer);

        ksBarGraph_RenderCompute(commandBuffer, &bargraphs->sceneDrawCallLevelBarGraph, framebuffer);
        ksBarGraph_RenderCompute(commandBuffer, &bargraphs->sceneTriangleLevelBarGraph, framebuffer);
        ksBarGraph_RenderCompute(commandBuffer, &bargraphs->sceneFragmentLevelBarGraph, framebuffer);

        ksGpuCommandBuffer_EndTimer(commandBuffer, &bargraphs->barGraphTimer);
    }
}

static ksNanoseconds ksTimeWarpBarGraphs_GetGpuNanosecondsGraphics(ksTimeWarpBarGraphs *bargraphs) {
    if (bargraphs->barGraphState != BAR_GRAPH_HIDDEN) {
        return ksGpuTimer_GetNanoseconds(&bargraphs->barGraphTimer);
    }
    return 0;
}

static ksNanoseconds ksTimeWarpBarGraphs_GetGpuNanosecondsCompute(ksTimeWarpBarGraphs *bargraphs) {
    if (bargraphs->barGraphState != BAR_GRAPH_HIDDEN) {
        return ksGpuTimer_GetNanoseconds(&bargraphs->barGraphTimer);
    }
    return 0;
}

/*
================================================================================================================================

HMD

ksHmdInfo
ksBodyInfo

================================================================================================================================
*/

#define NUM_EYES 2
#define NUM_COLOR_CHANNELS 3

typedef struct {
    int displayPixelsWide;
    int displayPixelsHigh;
    int tilePixelsWide;
    int tilePixelsHigh;
    int eyeTilesWide;
    int eyeTilesHigh;
    int visiblePixelsWide;
    int visiblePixelsHigh;
    float visibleMetersWide;
    float visibleMetersHigh;
    float lensSeparationInMeters;
    float metersPerTanAngleAtCenter;
    int numKnots;
    float K[11];
    float chromaticAberration[4];
} ksHmdInfo;

typedef struct {
    float interpupillaryDistance;
} ksBodyInfo;

static const ksHmdInfo *GetDefaultHmdInfo(const int displayPixelsWide, const int displayPixelsHigh) {
    static ksHmdInfo hmdInfo;
    hmdInfo.displayPixelsWide = displayPixelsWide;
    hmdInfo.displayPixelsHigh = displayPixelsHigh;
    hmdInfo.tilePixelsWide = 32;
    hmdInfo.tilePixelsHigh = 32;
    hmdInfo.eyeTilesWide = displayPixelsWide / hmdInfo.tilePixelsWide / NUM_EYES;
    hmdInfo.eyeTilesHigh = displayPixelsHigh / hmdInfo.tilePixelsHigh;
    hmdInfo.visiblePixelsWide = hmdInfo.eyeTilesWide * hmdInfo.tilePixelsWide * NUM_EYES;
    hmdInfo.visiblePixelsHigh = hmdInfo.eyeTilesHigh * hmdInfo.tilePixelsHigh;
    hmdInfo.visibleMetersWide = 0.11047f * (hmdInfo.eyeTilesWide * hmdInfo.tilePixelsWide * NUM_EYES) / displayPixelsWide;
    hmdInfo.visibleMetersHigh = 0.06214f * (hmdInfo.eyeTilesHigh * hmdInfo.tilePixelsHigh) / displayPixelsHigh;
    hmdInfo.lensSeparationInMeters = hmdInfo.visibleMetersWide / NUM_EYES;
    hmdInfo.metersPerTanAngleAtCenter = 0.037f;
    hmdInfo.numKnots = 11;
    hmdInfo.K[0] = 1.0f;
    hmdInfo.K[1] = 1.021f;
    hmdInfo.K[2] = 1.051f;
    hmdInfo.K[3] = 1.086f;
    hmdInfo.K[4] = 1.128f;
    hmdInfo.K[5] = 1.177f;
    hmdInfo.K[6] = 1.232f;
    hmdInfo.K[7] = 1.295f;
    hmdInfo.K[8] = 1.368f;
    hmdInfo.K[9] = 1.452f;
    hmdInfo.K[10] = 1.560f;
    hmdInfo.chromaticAberration[0] = -0.006f;
    hmdInfo.chromaticAberration[1] = 0.0f;
    hmdInfo.chromaticAberration[2] = 0.014f;
    hmdInfo.chromaticAberration[3] = 0.0f;
    return &hmdInfo;
}

static const ksBodyInfo *GetDefaultBodyInfo() {
    static ksBodyInfo bodyInfo;
    bodyInfo.interpupillaryDistance = 0.0640f;  // average interpupillary distance
    return &bodyInfo;
}

static bool hmd_headRotationDisabled = false;

static void GetHmdViewMatrixForTime(ksMatrix4x4f *viewMatrix, const ksNanoseconds time) {
    if (hmd_headRotationDisabled) {
        ksMatrix4x4f_CreateIdentity(viewMatrix);
        return;
    }

    // FIXME: use double?
    const float offset = time * (MATH_PI / 1000.0f / 1000.0f / 1000.0f);
    const float degrees = 10.0f;
    const float degreesX = sinf(offset) * degrees;
    const float degreesY = cosf(offset) * degrees;

    ksMatrix4x4f_CreateRotation(viewMatrix, degreesX, degreesY, 0.0f);
}

static void CalculateTimeWarpTransform(ksMatrix4x4f *transform, const ksMatrix4x4f *renderProjectionMatrix,
                                       const ksMatrix4x4f *renderViewMatrix, const ksMatrix4x4f *newViewMatrix) {
    // Convert the projection matrix from [-1, 1] space to [0, 1] space.
    const ksMatrix4x4f texCoordProjection = {
        {{0.5f * renderProjectionMatrix->m[0][0], 0.0f, 0.0f, 0.0f},
         {0.0f, 0.5f * renderProjectionMatrix->m[1][1], 0.0f, 0.0f},
         {0.5f * renderProjectionMatrix->m[2][0] - 0.5f, 0.5f * renderProjectionMatrix->m[2][1] - 0.5f, -1.0f, 0.0f},
         {0.0f, 0.0f, 0.0f, 1.0f}}};

    // Calculate the delta between the view matrix used for rendering and
    // a more recent or predicted view matrix based on new sensor input.
    ksMatrix4x4f inverseRenderViewMatrix;
    ksMatrix4x4f_InvertHomogeneous(&inverseRenderViewMatrix, renderViewMatrix);

    ksMatrix4x4f deltaViewMatrix;
    ksMatrix4x4f_Multiply(&deltaViewMatrix, &inverseRenderViewMatrix, newViewMatrix);

    ksMatrix4x4f inverseDeltaViewMatrix;
    ksMatrix4x4f_InvertHomogeneous(&inverseDeltaViewMatrix, &deltaViewMatrix);

    // Make the delta rotation only.
    inverseDeltaViewMatrix.m[3][0] = 0.0f;
    inverseDeltaViewMatrix.m[3][1] = 0.0f;
    inverseDeltaViewMatrix.m[3][2] = 0.0f;

    // Accumulate the transforms.
    ksMatrix4x4f_Multiply(transform, &texCoordProjection, &inverseDeltaViewMatrix);
}

/*
================================================================================================================================

Distortion meshes.

ksMeshCoord

================================================================================================================================
*/

typedef struct {
    float x;
    float y;
} ksMeshCoord;

static float MaxFloat(float x, float y) { return (x > y) ? x : y; }
static float MinFloat(float x, float y) { return (x < y) ? x : y; }

// A Catmull-Rom spline through the values K[0], K[1], K[2] ... K[numKnots-1] evenly spaced from 0.0 to 1.0
static float EvaluateCatmullRomSpline(const float value, float const *K, const int numKnots) {
    const float scaledValue = (float)(numKnots - 1) * value;
    const float scaledValueFloor = MaxFloat(0.0f, MinFloat((float)(numKnots - 1), floorf(scaledValue)));
    const float t = scaledValue - scaledValueFloor;
    const int k = (int)scaledValueFloor;

    float p0 = 0.0f;
    float p1 = 0.0f;
    float m0 = 0.0f;
    float m1 = 0.0f;

    if (k == 0) {
        p0 = K[0];
        m0 = K[1] - K[0];
        p1 = K[1];
        m1 = 0.5f * (K[2] - K[0]);
    } else if (k < numKnots - 2) {
        p0 = K[k];
        m0 = 0.5f * (K[k + 1] - K[k - 1]);
        p1 = K[k + 1];
        m1 = 0.5f * (K[k + 2] - K[k]);
    } else if (k == numKnots - 2) {
        p0 = K[k];
        m0 = 0.5f * (K[k + 1] - K[k - 1]);
        p1 = K[k + 1];
        m1 = K[k + 1] - K[k];
    } else if (k == numKnots - 1) {
        p0 = K[k];
        m0 = K[k] - K[k - 1];
        p1 = p0 + m0;
        m1 = m0;
    }

    const float omt = 1.0f - t;
    const float res = (p0 * (1.0f + 2.0f * t) + m0 * t) * omt * omt + (p1 * (1.0f + 2.0f * omt) - m1 * omt) * t * t;
    return res;
}

static void BuildDistortionMeshes(ksMeshCoord *meshCoords[NUM_EYES][NUM_COLOR_CHANNELS], const ksHmdInfo *hmdInfo) {
    const float horizontalShiftMeters = (hmdInfo->lensSeparationInMeters / 2) - (hmdInfo->visibleMetersWide / 4);
    const float horizontalShiftView = horizontalShiftMeters / (hmdInfo->visibleMetersWide / 2);

    for (int eye = 0; eye < NUM_EYES; eye++) {
        for (int y = 0; y <= hmdInfo->eyeTilesHigh; y++) {
            const float yf = 1.0f - (float)y / (float)hmdInfo->eyeTilesHigh;

            for (int x = 0; x <= hmdInfo->eyeTilesWide; x++) {
                const float xf = (float)x / (float)hmdInfo->eyeTilesWide;

                const float in[2] = {(eye ? -horizontalShiftView : horizontalShiftView) + xf, yf};
                const float ndcToPixels[2] = {hmdInfo->visiblePixelsWide * 0.25f, hmdInfo->visiblePixelsHigh * 0.5f};
                const float pixelsToMeters[2] = {hmdInfo->visibleMetersWide / hmdInfo->visiblePixelsWide,
                                                 hmdInfo->visibleMetersHigh / hmdInfo->visiblePixelsHigh};

                float theta[2];
                for (int i = 0; i < 2; i++) {
                    const float unit = in[i];
                    const float ndc = 2.0f * unit - 1.0f;
                    const float pixels = ndc * ndcToPixels[i];
                    const float meters = pixels * pixelsToMeters[i];
                    const float tanAngle = meters / hmdInfo->metersPerTanAngleAtCenter;
                    theta[i] = tanAngle;
                }

                const float rsq = theta[0] * theta[0] + theta[1] * theta[1];
                const float scale = EvaluateCatmullRomSpline(rsq, hmdInfo->K, hmdInfo->numKnots);
                const float chromaScale[NUM_COLOR_CHANNELS] = {
                    scale * (1.0f + hmdInfo->chromaticAberration[0] + rsq * hmdInfo->chromaticAberration[1]), scale,
                    scale * (1.0f + hmdInfo->chromaticAberration[2] + rsq * hmdInfo->chromaticAberration[3])};

                const int vertNum = y * (hmdInfo->eyeTilesWide + 1) + x;
                for (int channel = 0; channel < NUM_COLOR_CHANNELS; channel++) {
                    meshCoords[eye][channel][vertNum].x = chromaScale[channel] * theta[0];
                    meshCoords[eye][channel][vertNum].y = chromaScale[channel] * theta[1];
                }
            }
        }
    }
}

/*
================================================================================================================================

Time warp graphics rendering.

ksTimeWarpGraphics

static void ksTimeWarpGraphics_Create( ksGpuContext * context, ksTimeWarpGraphics * graphics,
                                                                        const ksHmdInfo * hmdInfo, ksGpuRenderPass * renderPass );
static void ksTimeWarpGraphics_Destroy( ksGpuContext * context, ksTimeWarpGraphics * graphics );
static void ksTimeWarpGraphics_Render( ksGpuCommandBuffer * commandBuffer, ksTimeWarpGraphics * graphics,
                                                                        ksGpuFramebuffer * framebuffer, ksGpuRenderPass *
renderPass, const ksNanoseconds refreshStartTime, const ksNanoseconds refreshEndTime, const ksMatrix4x4f * projectionMatrix, const
ksMatrix4x4f * viewMatrix, ksGpuTexture * const eyeTexture[NUM_EYES], const int eyeArrayLayer[NUM_EYES], const bool
correctChromaticAberration, ksTimeWarpBarGraphs * bargraphs, ksNanoseconds cpuTimes[PROFILE_TIME_MAX], ksNanoseconds
gpuTimes[PROFILE_TIME_MAX] );

================================================================================================================================
*/

typedef struct {
    ksHmdInfo hmdInfo;
    ksGpuGeometry distortionMesh[NUM_EYES];
    ksGpuGraphicsProgram timeWarpSpatialProgram;
    ksGpuGraphicsProgram timeWarpChromaticProgram;
    ksGpuGraphicsPipeline timeWarpSpatialPipeline[NUM_EYES];
    ksGpuGraphicsPipeline timeWarpChromaticPipeline[NUM_EYES];
    ksGpuTimer timeWarpGpuTime;
} ksTimeWarpGraphics;

enum {
    GRAPHICS_PROGRAM_UNIFORM_TIMEWARP_START_TRANSFORM,
    GRAPHICS_PROGRAM_UNIFORM_TIMEWARP_END_TRANSFORM,
    GRAPHICS_PROGRAM_UNIFORM_TIMEWARP_ARRAY_LAYER,
    GRAPHICS_PROGRAM_TEXTURE_TIMEWARP_SOURCE
};

static const ksGpuProgramParm timeWarpSpatialGraphicsProgramParms[] = {
    {KS_GPU_PROGRAM_STAGE_FLAG_VERTEX, KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_MATRIX3X4, KS_GPU_PROGRAM_PARM_ACCESS_READ_ONLY,
     GRAPHICS_PROGRAM_UNIFORM_TIMEWARP_START_TRANSFORM, "TimeWarpStartTransform", 0},
    {KS_GPU_PROGRAM_STAGE_FLAG_VERTEX, KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_MATRIX3X4, KS_GPU_PROGRAM_PARM_ACCESS_READ_ONLY,
     GRAPHICS_PROGRAM_UNIFORM_TIMEWARP_END_TRANSFORM, "TimeWarpEndTransform", 1},
    {KS_GPU_PROGRAM_STAGE_FLAG_FRAGMENT, KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_INT, KS_GPU_PROGRAM_PARM_ACCESS_READ_ONLY,
     GRAPHICS_PROGRAM_UNIFORM_TIMEWARP_ARRAY_LAYER, "ArrayLayer", 2},
    {KS_GPU_PROGRAM_STAGE_FLAG_FRAGMENT, KS_GPU_PROGRAM_PARM_TYPE_TEXTURE_SAMPLED, KS_GPU_PROGRAM_PARM_ACCESS_READ_ONLY,
     GRAPHICS_PROGRAM_TEXTURE_TIMEWARP_SOURCE, "Texture", 0}};

static const char timeWarpSpatialVertexProgramGLSL[] =
    "#version " GLSL_VERSION "\n" GLSL_EXTENSIONS
    "uniform highp mat3x4 TimeWarpStartTransform;\n"
    "uniform highp mat3x4 TimeWarpEndTransform;\n"
    "in highp vec3 vertexPosition;\n"
    "in highp vec2 vertexUv1;\n"
    "out mediump vec2 fragmentUv1;\n"
    "out gl_PerVertex { vec4 gl_Position; };\n"
    "void main( void )\n"
    "{\n"
    "    gl_Position = vec4( vertexPosition, 1.0 );\n"
    "\n"
    "    float displayFraction = vertexPosition.x * 0.5 + 0.5;\n"  // landscape left-to-right
    "\n"
    "    vec3 startUv1 = vec4( vertexUv1, -1, 1 ) * TimeWarpStartTransform;\n"
    "    vec3 endUv1 = vec4( vertexUv1, -1, 1 ) * TimeWarpEndTransform;\n"
    "    vec3 curUv1 = mix( startUv1, endUv1, displayFraction );\n"
    "    fragmentUv1 = curUv1.xy * ( 1.0 / max( curUv1.z, 0.00001 ) );\n"
    "}\n";

static const char timeWarpSpatialFragmentProgramGLSL[] = "#version " GLSL_VERSION "\n" GLSL_EXTENSIONS
                                                         "uniform int ArrayLayer;\n"
                                                         "uniform highp sampler2DArray Texture;\n"
                                                         "in mediump vec2 fragmentUv1;\n"
                                                         "out lowp vec4 outColor;\n"
                                                         "void main()\n"
                                                         "{\n"
                                                         "    outColor = texture( Texture, vec3( fragmentUv1, ArrayLayer ) );\n"
                                                         "}\n";

static const ksGpuProgramParm timeWarpChromaticGraphicsProgramParms[] = {
    {KS_GPU_PROGRAM_STAGE_FLAG_VERTEX, KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_MATRIX3X4, KS_GPU_PROGRAM_PARM_ACCESS_READ_ONLY,
     GRAPHICS_PROGRAM_UNIFORM_TIMEWARP_START_TRANSFORM, "TimeWarpStartTransform", 0},
    {KS_GPU_PROGRAM_STAGE_FLAG_VERTEX, KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_MATRIX3X4, KS_GPU_PROGRAM_PARM_ACCESS_READ_ONLY,
     GRAPHICS_PROGRAM_UNIFORM_TIMEWARP_END_TRANSFORM, "TimeWarpEndTransform", 1},
    {KS_GPU_PROGRAM_STAGE_FLAG_FRAGMENT, KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_INT, KS_GPU_PROGRAM_PARM_ACCESS_READ_ONLY,
     GRAPHICS_PROGRAM_UNIFORM_TIMEWARP_ARRAY_LAYER, "ArrayLayer", 2},
    {KS_GPU_PROGRAM_STAGE_FLAG_FRAGMENT, KS_GPU_PROGRAM_PARM_TYPE_TEXTURE_SAMPLED, KS_GPU_PROGRAM_PARM_ACCESS_READ_ONLY,
     GRAPHICS_PROGRAM_TEXTURE_TIMEWARP_SOURCE, "Texture", 0}};

static const char timeWarpChromaticVertexProgramGLSL[] =
    "#version " GLSL_VERSION "\n" GLSL_EXTENSIONS
    "uniform highp mat3x4 TimeWarpStartTransform;\n"
    "uniform highp mat3x4 TimeWarpEndTransform;\n"
    "in highp vec3 vertexPosition;\n"
    "in highp vec2 vertexUv0;\n"
    "in highp vec2 vertexUv1;\n"
    "in highp vec2 vertexUv2;\n"
    "out mediump vec2 fragmentUv0;\n"
    "out mediump vec2 fragmentUv1;\n"
    "out mediump vec2 fragmentUv2;\n"
    "out gl_PerVertex { vec4 gl_Position; };\n"
    "void main( void )\n"
    "{\n"
    "    gl_Position = vec4( vertexPosition, 1.0 );\n"
    "\n"
    "    float displayFraction = vertexPosition.x * 0.5 + 0.5;\n"  // landscape left-to-right
    "\n"
    "    vec3 startUv0 = vec4( vertexUv0, -1, 1 ) * TimeWarpStartTransform;\n"
    "    vec3 startUv1 = vec4( vertexUv1, -1, 1 ) * TimeWarpStartTransform;\n"
    "    vec3 startUv2 = vec4( vertexUv2, -1, 1 ) * TimeWarpStartTransform;\n"
    "\n"
    "    vec3 endUv0 = vec4( vertexUv0, -1, 1 ) * TimeWarpEndTransform;\n"
    "    vec3 endUv1 = vec4( vertexUv1, -1, 1 ) * TimeWarpEndTransform;\n"
    "    vec3 endUv2 = vec4( vertexUv2, -1, 1 ) * TimeWarpEndTransform;\n"
    "\n"
    "    vec3 curUv0 = mix( startUv0, endUv0, displayFraction );\n"
    "    vec3 curUv1 = mix( startUv1, endUv1, displayFraction );\n"
    "    vec3 curUv2 = mix( startUv2, endUv2, displayFraction );\n"
    "\n"
    "    fragmentUv0 = curUv0.xy * ( 1.0 / max( curUv0.z, 0.00001 ) );\n"
    "    fragmentUv1 = curUv1.xy * ( 1.0 / max( curUv1.z, 0.00001 ) );\n"
    "    fragmentUv2 = curUv2.xy * ( 1.0 / max( curUv2.z, 0.00001 ) );\n"
    "}\n";

static const char timeWarpChromaticFragmentProgramGLSL[] =
    "#version " GLSL_VERSION "\n" GLSL_EXTENSIONS
    "uniform int ArrayLayer;\n"
    "uniform highp sampler2DArray Texture;\n"
    "in mediump vec2 fragmentUv0;\n"
    "in mediump vec2 fragmentUv1;\n"
    "in mediump vec2 fragmentUv2;\n"
    "out lowp vec4 outColor;\n"
    "void main()\n"
    "{\n"
    "    outColor.r = texture( Texture, vec3( fragmentUv0, ArrayLayer ) ).r;\n"
    "    outColor.g = texture( Texture, vec3( fragmentUv1, ArrayLayer ) ).g;\n"
    "    outColor.b = texture( Texture, vec3( fragmentUv2, ArrayLayer ) ).b;\n"
    "    outColor.a = 1.0;\n"
    "}\n";

static void ksTimeWarpGraphics_Create(ksGpuContext *context, ksTimeWarpGraphics *graphics, const ksHmdInfo *hmdInfo,
                                      ksGpuRenderPass *renderPass) {
    memset(graphics, 0, sizeof(ksTimeWarpGraphics));

    graphics->hmdInfo = *hmdInfo;

    const int vertexCount = (hmdInfo->eyeTilesHigh + 1) * (hmdInfo->eyeTilesWide + 1);
    const int indexCount = hmdInfo->eyeTilesHigh * hmdInfo->eyeTilesWide * 6;

    ksGpuTriangleIndexArray indices;
    ksGpuTriangleIndexArray_Alloc(&indices, indexCount, NULL);

    for (int y = 0; y < hmdInfo->eyeTilesHigh; y++) {
        for (int x = 0; x < hmdInfo->eyeTilesWide; x++) {
            const int offset = (y * hmdInfo->eyeTilesWide + x) * 6;

            indices.indexArray[offset + 0] = (ksGpuTriangleIndex)((y + 0) * (hmdInfo->eyeTilesWide + 1) + (x + 0));
            indices.indexArray[offset + 1] = (ksGpuTriangleIndex)((y + 1) * (hmdInfo->eyeTilesWide + 1) + (x + 0));
            indices.indexArray[offset + 2] = (ksGpuTriangleIndex)((y + 0) * (hmdInfo->eyeTilesWide + 1) + (x + 1));

            indices.indexArray[offset + 3] = (ksGpuTriangleIndex)((y + 0) * (hmdInfo->eyeTilesWide + 1) + (x + 1));
            indices.indexArray[offset + 4] = (ksGpuTriangleIndex)((y + 1) * (hmdInfo->eyeTilesWide + 1) + (x + 0));
            indices.indexArray[offset + 5] = (ksGpuTriangleIndex)((y + 1) * (hmdInfo->eyeTilesWide + 1) + (x + 1));
        }
    }

    ksDefaultVertexAttributeArrays vertexAttribs;
    ksGpuVertexAttributeArrays_Alloc(
        &vertexAttribs.base, DefaultVertexAttributeLayout, vertexCount,
        VERTEX_ATTRIBUTE_FLAG_POSITION | VERTEX_ATTRIBUTE_FLAG_UV0 | VERTEX_ATTRIBUTE_FLAG_UV1 | VERTEX_ATTRIBUTE_FLAG_UV2);

    const int numMeshCoords = (hmdInfo->eyeTilesWide + 1) * (hmdInfo->eyeTilesHigh + 1);
    ksMeshCoord *meshCoordsBasePtr = (ksMeshCoord *)malloc(NUM_EYES * NUM_COLOR_CHANNELS * numMeshCoords * sizeof(ksMeshCoord));
    ksMeshCoord *meshCoords[NUM_EYES][NUM_COLOR_CHANNELS] = {
        {meshCoordsBasePtr + 0 * numMeshCoords, meshCoordsBasePtr + 1 * numMeshCoords, meshCoordsBasePtr + 2 * numMeshCoords},
        {meshCoordsBasePtr + 3 * numMeshCoords, meshCoordsBasePtr + 4 * numMeshCoords, meshCoordsBasePtr + 5 * numMeshCoords}};
    BuildDistortionMeshes(meshCoords, hmdInfo);

#if defined(GRAPHICS_API_VULKAN)
    const float flipY = -1.0f;
#else
    const float flipY = 1.0f;
#endif

    for (int eye = 0; eye < NUM_EYES; eye++) {
        for (int y = 0; y <= hmdInfo->eyeTilesHigh; y++) {
            for (int x = 0; x <= hmdInfo->eyeTilesWide; x++) {
                const int index = y * (hmdInfo->eyeTilesWide + 1) + x;
                vertexAttribs.position[index].x = (-1.0f + eye + ((float)x / hmdInfo->eyeTilesWide));
                vertexAttribs.position[index].y =
                    (-1.0f + 2.0f * ((hmdInfo->eyeTilesHigh - (float)y) / hmdInfo->eyeTilesHigh) *
                                 ((float)(hmdInfo->eyeTilesHigh * hmdInfo->tilePixelsHigh) / hmdInfo->displayPixelsHigh)) *
                    flipY;
                vertexAttribs.position[index].z = 0.0f;
                vertexAttribs.uv0[index].x = meshCoords[eye][0][index].x;
                vertexAttribs.uv0[index].y = meshCoords[eye][0][index].y;
                vertexAttribs.uv1[index].x = meshCoords[eye][1][index].x;
                vertexAttribs.uv1[index].y = meshCoords[eye][1][index].y;
                vertexAttribs.uv2[index].x = meshCoords[eye][2][index].x;
                vertexAttribs.uv2[index].y = meshCoords[eye][2][index].y;
            }
        }

        ksGpuGeometry_Create(context, &graphics->distortionMesh[eye], &vertexAttribs.base, &indices);
    }

    free(meshCoordsBasePtr);
    ksGpuVertexAttributeArrays_Free(&vertexAttribs.base);
    ksGpuTriangleIndexArray_Free(&indices);

    ksGpuGraphicsProgram_Create(context, &graphics->timeWarpSpatialProgram, PROGRAM(timeWarpSpatialVertexProgram),
                                sizeof(PROGRAM(timeWarpSpatialVertexProgram)), PROGRAM(timeWarpSpatialFragmentProgram),
                                sizeof(PROGRAM(timeWarpSpatialFragmentProgram)), timeWarpSpatialGraphicsProgramParms,
                                ARRAY_SIZE(timeWarpSpatialGraphicsProgramParms), graphics->distortionMesh[0].layout,
                                VERTEX_ATTRIBUTE_FLAG_POSITION | VERTEX_ATTRIBUTE_FLAG_UV1);
    ksGpuGraphicsProgram_Create(
        context, &graphics->timeWarpChromaticProgram, PROGRAM(timeWarpChromaticVertexProgram),
        sizeof(PROGRAM(timeWarpChromaticVertexProgram)), PROGRAM(timeWarpChromaticFragmentProgram),
        sizeof(PROGRAM(timeWarpChromaticFragmentProgram)), timeWarpChromaticGraphicsProgramParms,
        ARRAY_SIZE(timeWarpChromaticGraphicsProgramParms), graphics->distortionMesh[0].layout,
        VERTEX_ATTRIBUTE_FLAG_POSITION | VERTEX_ATTRIBUTE_FLAG_UV0 | VERTEX_ATTRIBUTE_FLAG_UV1 | VERTEX_ATTRIBUTE_FLAG_UV2);

    for (int eye = 0; eye < NUM_EYES; eye++) {
        ksGpuGraphicsPipelineParms pipelineParms;
        ksGpuGraphicsPipelineParms_Init(&pipelineParms);

        pipelineParms.rop.depthTestEnable = false;
        pipelineParms.rop.depthWriteEnable = false;
        pipelineParms.renderPass = renderPass;
        pipelineParms.program = &graphics->timeWarpSpatialProgram;
        pipelineParms.geometry = &graphics->distortionMesh[eye];

        ksGpuGraphicsPipeline_Create(context, &graphics->timeWarpSpatialPipeline[eye], &pipelineParms);

        pipelineParms.program = &graphics->timeWarpChromaticProgram;
        pipelineParms.geometry = &graphics->distortionMesh[eye];

        ksGpuGraphicsPipeline_Create(context, &graphics->timeWarpChromaticPipeline[eye], &pipelineParms);
    }

    ksGpuTimer_Create(context, &graphics->timeWarpGpuTime);
}

static void ksTimeWarpGraphics_Destroy(ksGpuContext *context, ksTimeWarpGraphics *graphics) {
    ksGpuTimer_Destroy(context, &graphics->timeWarpGpuTime);

    for (int eye = 0; eye < NUM_EYES; eye++) {
        ksGpuGraphicsPipeline_Destroy(context, &graphics->timeWarpSpatialPipeline[eye]);
        ksGpuGraphicsPipeline_Destroy(context, &graphics->timeWarpChromaticPipeline[eye]);
    }

    ksGpuGraphicsProgram_Destroy(context, &graphics->timeWarpSpatialProgram);
    ksGpuGraphicsProgram_Destroy(context, &graphics->timeWarpChromaticProgram);

    for (int eye = 0; eye < NUM_EYES; eye++) {
        ksGpuGeometry_Destroy(context, &graphics->distortionMesh[eye]);
    }
}

static void ksTimeWarpGraphics_Render(ksGpuCommandBuffer *commandBuffer, ksTimeWarpGraphics *graphics,
                                      ksGpuFramebuffer *framebuffer, ksGpuRenderPass *renderPass,
                                      const ksNanoseconds refreshStartTime, const ksNanoseconds refreshEndTime,
                                      const ksMatrix4x4f *projectionMatrix, const ksMatrix4x4f *viewMatrix,
                                      ksGpuTexture *const eyeTexture[NUM_EYES], const int eyeArrayLayer[NUM_EYES],
                                      const bool correctChromaticAberration, ksTimeWarpBarGraphs *bargraphs,
                                      ksNanoseconds cpuTimes[PROFILE_TIME_MAX], ksNanoseconds gpuTimes[PROFILE_TIME_MAX]) {
    const ksNanoseconds t0 = GetTimeNanoseconds();

    ksMatrix4x4f displayRefreshStartViewMatrix;
    ksMatrix4x4f displayRefreshEndViewMatrix;
    GetHmdViewMatrixForTime(&displayRefreshStartViewMatrix, refreshStartTime);
    GetHmdViewMatrixForTime(&displayRefreshEndViewMatrix, refreshEndTime);

    ksMatrix4x4f timeWarpStartTransform;
    ksMatrix4x4f timeWarpEndTransform;
    CalculateTimeWarpTransform(&timeWarpStartTransform, projectionMatrix, viewMatrix, &displayRefreshStartViewMatrix);
    CalculateTimeWarpTransform(&timeWarpEndTransform, projectionMatrix, viewMatrix, &displayRefreshEndViewMatrix);

    ksMatrix3x4f timeWarpStartTransform3x4;
    ksMatrix3x4f timeWarpEndTransform3x4;
    ksMatrix3x4f_CreateFromMatrix4x4f(&timeWarpStartTransform3x4, &timeWarpStartTransform);
    ksMatrix3x4f_CreateFromMatrix4x4f(&timeWarpEndTransform3x4, &timeWarpEndTransform);

    const ksScreenRect screenRect = ksGpuFramebuffer_GetRect(framebuffer);

    ksGpuCommandBuffer_BeginPrimary(commandBuffer);
    ksGpuCommandBuffer_BeginFramebuffer(commandBuffer, framebuffer, 0, KS_GPU_TEXTURE_USAGE_COLOR_ATTACHMENT);

    ksTimeWarpBarGraphs_UpdateGraphics(commandBuffer, bargraphs);

    ksGpuCommandBuffer_BeginTimer(commandBuffer, &graphics->timeWarpGpuTime);
    ksGpuCommandBuffer_BeginRenderPass(commandBuffer, renderPass, framebuffer, &screenRect);

    ksGpuCommandBuffer_SetViewport(commandBuffer, &screenRect);
    ksGpuCommandBuffer_SetScissor(commandBuffer, &screenRect);

    for (int eye = 0; eye < NUM_EYES; eye++) {
        ksGpuGraphicsCommand command;
        ksGpuGraphicsCommand_Init(&command);
        ksGpuGraphicsCommand_SetPipeline(&command, correctChromaticAberration ? &graphics->timeWarpChromaticPipeline[eye]
                                                                              : &graphics->timeWarpSpatialPipeline[eye]);
        ksGpuGraphicsCommand_SetParmFloatMatrix3x4(&command, GRAPHICS_PROGRAM_UNIFORM_TIMEWARP_START_TRANSFORM,
                                                   &timeWarpStartTransform3x4);
        ksGpuGraphicsCommand_SetParmFloatMatrix3x4(&command, GRAPHICS_PROGRAM_UNIFORM_TIMEWARP_END_TRANSFORM,
                                                   &timeWarpEndTransform3x4);
        ksGpuGraphicsCommand_SetParmInt(&command, GRAPHICS_PROGRAM_UNIFORM_TIMEWARP_ARRAY_LAYER, &eyeArrayLayer[eye]);
        ksGpuGraphicsCommand_SetParmTextureSampled(&command, GRAPHICS_PROGRAM_TEXTURE_TIMEWARP_SOURCE, eyeTexture[eye]);

        ksGpuCommandBuffer_SubmitGraphicsCommand(commandBuffer, &command);
    }

    const ksNanoseconds t1 = GetTimeNanoseconds();

    ksTimeWarpBarGraphs_RenderGraphics(commandBuffer, bargraphs);

    ksGpuCommandBuffer_EndRenderPass(commandBuffer, renderPass);
    ksGpuCommandBuffer_EndTimer(commandBuffer, &graphics->timeWarpGpuTime);

    ksGpuCommandBuffer_EndFramebuffer(commandBuffer, framebuffer, 0, KS_GPU_TEXTURE_USAGE_PRESENTATION);
    ksGpuCommandBuffer_EndPrimary(commandBuffer);

    ksGpuCommandBuffer_SubmitPrimary(commandBuffer);

    const ksNanoseconds t2 = GetTimeNanoseconds();

    cpuTimes[PROFILE_TIME_TIME_WARP] = t1 - t0;
    cpuTimes[PROFILE_TIME_BAR_GRAPHS] = t2 - t1;
    cpuTimes[PROFILE_TIME_BLIT] = 0;

    const ksNanoseconds barGraphGpuTime = ksTimeWarpBarGraphs_GetGpuNanosecondsGraphics(bargraphs);

    gpuTimes[PROFILE_TIME_TIME_WARP] = ksGpuTimer_GetNanoseconds(&graphics->timeWarpGpuTime) - barGraphGpuTime;
    gpuTimes[PROFILE_TIME_BAR_GRAPHS] = barGraphGpuTime;
    gpuTimes[PROFILE_TIME_BLIT] = 0;

#if GL_FINISH_SYNC == 1
    GL(glFinish());
#endif
}

/*
================================================================================================================================

Time warp compute rendering.

ksTimeWarpCompute

static void ksTimeWarpCompute_Create( ksGpuContext * context, ksTimeWarpCompute * compute, const ksHmdInfo * hmdInfo,
                                                                        ksGpuRenderPass * renderPass, ksGpuWindow * window );
static void ksTimeWarpCompute_Destroy( ksGpuContext * context, ksTimeWarpCompute * compute );
static void ksTimeWarpCompute_Render( ksGpuCommandBuffer * commandBuffer, ksTimeWarpCompute * compute,
                                                                        ksGpuFramebuffer * framebuffer,
                                                                        const ksNanoseconds refreshStartTime, const ksNanoseconds
refreshEndTime, const ksMatrix4x4f * projectionMatrix, const ksMatrix4x4f * viewMatrix, ksGpuTexture * const eyeTexture[NUM_EYES],
const int eyeArrayLayer[NUM_EYES], const bool correctChromaticAberration, ksTimeWarpBarGraphs * bargraphs, ksNanoseconds
cpuTimes[PROFILE_TIME_MAX], ksNanoseconds gpuTimes[PROFILE_TIME_MAX] );

================================================================================================================================
*/

#if OPENGL_COMPUTE_ENABLED == 1

typedef struct {
    ksHmdInfo hmdInfo;
    ksGpuTexture distortionImage[NUM_EYES][NUM_COLOR_CHANNELS];
    ksGpuTexture timeWarpImage[NUM_EYES][NUM_COLOR_CHANNELS];
    ksGpuComputeProgram timeWarpTransformProgram;
    ksGpuComputeProgram timeWarpSpatialProgram;
    ksGpuComputeProgram timeWarpChromaticProgram;
    ksGpuComputePipeline timeWarpTransformPipeline;
    ksGpuComputePipeline timeWarpSpatialPipeline;
    ksGpuComputePipeline timeWarpChromaticPipeline;
    ksGpuTimer timeWarpGpuTime;
    ksGpuFramebuffer framebuffer;
} ksTimeWarpCompute;

enum {
    COMPUTE_PROGRAM_TEXTURE_TIMEWARP_TRANSFORM_DST,
    COMPUTE_PROGRAM_TEXTURE_TIMEWARP_TRANSFORM_SRC,
    COMPUTE_PROGRAM_UNIFORM_TIMEWARP_DIMENSIONS,
    COMPUTE_PROGRAM_UNIFORM_TIMEWARP_EYE,
    COMPUTE_PROGRAM_UNIFORM_TIMEWARP_START_TRANSFORM,
    COMPUTE_PROGRAM_UNIFORM_TIMEWARP_END_TRANSFORM
};

static const ksGpuProgramParm timeWarpTransformComputeProgramParms[] = {
    {KS_GPU_PROGRAM_STAGE_FLAG_COMPUTE, KS_GPU_PROGRAM_PARM_TYPE_TEXTURE_STORAGE, KS_GPU_PROGRAM_PARM_ACCESS_WRITE_ONLY,
     COMPUTE_PROGRAM_TEXTURE_TIMEWARP_TRANSFORM_DST, "dst", 0},
    {KS_GPU_PROGRAM_STAGE_FLAG_COMPUTE, KS_GPU_PROGRAM_PARM_TYPE_TEXTURE_STORAGE, KS_GPU_PROGRAM_PARM_ACCESS_READ_ONLY,
     COMPUTE_PROGRAM_TEXTURE_TIMEWARP_TRANSFORM_SRC, "src", 1},
    {KS_GPU_PROGRAM_STAGE_FLAG_COMPUTE, KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_INT_VECTOR2, KS_GPU_PROGRAM_PARM_ACCESS_READ_ONLY,
     COMPUTE_PROGRAM_UNIFORM_TIMEWARP_DIMENSIONS, "dimensions", 0},
    {KS_GPU_PROGRAM_STAGE_FLAG_COMPUTE, KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_INT, KS_GPU_PROGRAM_PARM_ACCESS_READ_ONLY,
     COMPUTE_PROGRAM_UNIFORM_TIMEWARP_EYE, "eye", 1},
    {KS_GPU_PROGRAM_STAGE_FLAG_COMPUTE, KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_MATRIX3X4,
     KS_GPU_PROGRAM_PARM_ACCESS_READ_ONLY, COMPUTE_PROGRAM_UNIFORM_TIMEWARP_START_TRANSFORM, "timeWarpStartTransform", 2},
    {KS_GPU_PROGRAM_STAGE_FLAG_COMPUTE, KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_MATRIX3X4,
     KS_GPU_PROGRAM_PARM_ACCESS_READ_ONLY, COMPUTE_PROGRAM_UNIFORM_TIMEWARP_END_TRANSFORM, "timeWarpEndTransform", 3}};

#define TRANSFORM_LOCAL_SIZE_X 8
#define TRANSFORM_LOCAL_SIZE_Y 8

static const char timeWarpTransformComputeProgramGLSL[] =
    "#version " GLSL_VERSION "\n"
    GLSL_EXTENSIONS
    "\n"
    "layout( local_size_x = " STRINGIFY( TRANSFORM_LOCAL_SIZE_X ) ", local_size_y = " STRINGIFY( TRANSFORM_LOCAL_SIZE_Y ) " ) in;\n"
    "\n"
    "layout( rgba16f, binding = 0 ) uniform writeonly " ES_HIGHP " image2D dst;\n"
    "layout( rgba32f, binding = 1 ) uniform readonly " ES_HIGHP " image2D src;\n"
    "uniform highp mat3x4 timeWarpStartTransform;\n"
     "uniform highp mat3x4 timeWarpEndTransform;\n"
    "uniform ivec2 dimensions;\n"
    "uniform int eye;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    ivec2 mesh = ivec2( gl_GlobalInvocationID.xy );\n"
    "    if ( mesh.x >= dimensions.x || mesh.y >= dimensions.y )\n"
    "    {\n"
    "        return;\n"
    "    }\n"
    "    int eyeTilesWide = int( gl_NumWorkGroups.x * gl_WorkGroupSize.x ) - 1;\n"
    "    int eyeTilesHigh = int( gl_NumWorkGroups.y * gl_WorkGroupSize.y ) - 1;\n"
    "\n"
    "    vec2 coords = imageLoad( src, mesh ).xy;\n"
    "\n"
    "    float displayFraction = float( eye * eyeTilesWide + mesh.x ) / ( float( eyeTilesWide ) * 2.0f );\n"        // landscape left-to-right
    "    vec3 start = vec4( coords, -1.0f, 1.0f ) * timeWarpStartTransform;\n"
    "    vec3 end = vec4( coords, -1.0f, 1.0f ) * timeWarpEndTransform;\n"
    "    vec3 cur = start + displayFraction * ( end - start );\n"
    "    float rcpZ = 1.0f / cur.z;\n"
    "\n"
    "    imageStore( dst, mesh, vec4( cur.xy * rcpZ, 0.0f, 0.0f ) );\n"
    "}\n";

enum {
    COMPUTE_PROGRAM_TEXTURE_TIMEWARP_DEST,
    COMPUTE_PROGRAM_TEXTURE_TIMEWARP_EYE_IMAGE,
    COMPUTE_PROGRAM_TEXTURE_TIMEWARP_WARP_IMAGE_R,
    COMPUTE_PROGRAM_TEXTURE_TIMEWARP_WARP_IMAGE_G,
    COMPUTE_PROGRAM_TEXTURE_TIMEWARP_WARP_IMAGE_B,
    COMPUTE_PROGRAM_UNIFORM_TIMEWARP_IMAGE_SCALE,
    COMPUTE_PROGRAM_UNIFORM_TIMEWARP_IMAGE_BIAS,
    COMPUTE_PROGRAM_UNIFORM_TIMEWARP_IMAGE_LAYER,
    COMPUTE_PROGRAM_UNIFORM_TIMEWARP_EYE_PIXEL_OFFSET
};

static const ksGpuProgramParm timeWarpSpatialComputeProgramParms[] = {
    {KS_GPU_PROGRAM_STAGE_FLAG_COMPUTE, KS_GPU_PROGRAM_PARM_TYPE_TEXTURE_STORAGE, KS_GPU_PROGRAM_PARM_ACCESS_WRITE_ONLY,
     COMPUTE_PROGRAM_TEXTURE_TIMEWARP_DEST, "dest", 0},
    {KS_GPU_PROGRAM_STAGE_FLAG_COMPUTE, KS_GPU_PROGRAM_PARM_TYPE_TEXTURE_SAMPLED, KS_GPU_PROGRAM_PARM_ACCESS_READ_ONLY,
     COMPUTE_PROGRAM_TEXTURE_TIMEWARP_EYE_IMAGE, "eyeImage", 0},
    {KS_GPU_PROGRAM_STAGE_FLAG_COMPUTE, KS_GPU_PROGRAM_PARM_TYPE_TEXTURE_SAMPLED, KS_GPU_PROGRAM_PARM_ACCESS_READ_ONLY,
     COMPUTE_PROGRAM_TEXTURE_TIMEWARP_WARP_IMAGE_G, "warpImageG", 1},
    {KS_GPU_PROGRAM_STAGE_FLAG_COMPUTE, KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_VECTOR2, KS_GPU_PROGRAM_PARM_ACCESS_READ_ONLY,
     COMPUTE_PROGRAM_UNIFORM_TIMEWARP_IMAGE_SCALE, "imageScale", 0},
    {KS_GPU_PROGRAM_STAGE_FLAG_COMPUTE, KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_VECTOR2, KS_GPU_PROGRAM_PARM_ACCESS_READ_ONLY,
     COMPUTE_PROGRAM_UNIFORM_TIMEWARP_IMAGE_BIAS, "imageBias", 1},
    {KS_GPU_PROGRAM_STAGE_FLAG_COMPUTE, KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_INT_VECTOR2, KS_GPU_PROGRAM_PARM_ACCESS_READ_ONLY,
     COMPUTE_PROGRAM_UNIFORM_TIMEWARP_EYE_PIXEL_OFFSET, "eyePixelOffset", 3},
    {KS_GPU_PROGRAM_STAGE_FLAG_COMPUTE, KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_INT, KS_GPU_PROGRAM_PARM_ACCESS_READ_ONLY,
     COMPUTE_PROGRAM_UNIFORM_TIMEWARP_IMAGE_LAYER, "imageLayer", 2}};

#define SPATIAL_LOCAL_SIZE_X 8
#define SPATIAL_LOCAL_SIZE_Y 8

static const char timeWarpSpatialComputeProgramGLSL[] =
    "#version " GLSL_VERSION "\n"
    GLSL_EXTENSIONS
    "\n"
    "layout( local_size_x = " STRINGIFY( SPATIAL_LOCAL_SIZE_X ) ", local_size_y = " STRINGIFY( SPATIAL_LOCAL_SIZE_Y ) " ) in;\n"
    "\n"
    "// imageScale = {    eyeTilesWide / ( eyeTilesWide + 1 ) / eyePixelsWide,\n"
    "//                    eyeTilesHigh / ( eyeTilesHigh + 1 ) / eyePixelsHigh };\n"
    "// imageBias  = {    0.5f / ( eyeTilesWide + 1 ),\n"
    "//                    0.5f / ( eyeTilesHigh + 1 ) };\n"
    "layout( rgba8, binding = 0 ) uniform writeonly " ES_HIGHP " image2D dest;\n"
    "uniform highp sampler2DArray eyeImage;\n"
    "uniform highp sampler2D warpImageG;\n"
    "uniform highp vec2 imageScale;\n"
    "uniform highp vec2 imageBias;\n"
    "uniform ivec2 eyePixelOffset;\n"
    "uniform int imageLayer;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    vec2 tile = ( vec2( gl_GlobalInvocationID.xy ) + vec2( 0.5f ) ) * imageScale + imageBias;\n"
    "\n"
    "    vec2 eyeCoords = texture( warpImageG, tile ).xy;\n"
    "\n"
    "    vec4 rgba = texture( eyeImage, vec3( eyeCoords, imageLayer ) );\n"
    "\n"
    "    imageStore( dest, ivec2( int( gl_GlobalInvocationID.x ) + eyePixelOffset.x, eyePixelOffset.y - 1 - int( gl_GlobalInvocationID.y ) ), rgba );\n"
    "}\n";

static const ksGpuProgramParm timeWarpChromaticComputeProgramParms[] = {
    {KS_GPU_PROGRAM_STAGE_FLAG_COMPUTE, KS_GPU_PROGRAM_PARM_TYPE_TEXTURE_STORAGE, KS_GPU_PROGRAM_PARM_ACCESS_WRITE_ONLY,
     COMPUTE_PROGRAM_TEXTURE_TIMEWARP_DEST, "dest", 0},
    {KS_GPU_PROGRAM_STAGE_FLAG_COMPUTE, KS_GPU_PROGRAM_PARM_TYPE_TEXTURE_SAMPLED, KS_GPU_PROGRAM_PARM_ACCESS_READ_ONLY,
     COMPUTE_PROGRAM_TEXTURE_TIMEWARP_EYE_IMAGE, "eyeImage", 0},
    {KS_GPU_PROGRAM_STAGE_FLAG_COMPUTE, KS_GPU_PROGRAM_PARM_TYPE_TEXTURE_SAMPLED, KS_GPU_PROGRAM_PARM_ACCESS_READ_ONLY,
     COMPUTE_PROGRAM_TEXTURE_TIMEWARP_WARP_IMAGE_R, "warpImageR", 1},
    {KS_GPU_PROGRAM_STAGE_FLAG_COMPUTE, KS_GPU_PROGRAM_PARM_TYPE_TEXTURE_SAMPLED, KS_GPU_PROGRAM_PARM_ACCESS_READ_ONLY,
     COMPUTE_PROGRAM_TEXTURE_TIMEWARP_WARP_IMAGE_G, "warpImageG", 2},
    {KS_GPU_PROGRAM_STAGE_FLAG_COMPUTE, KS_GPU_PROGRAM_PARM_TYPE_TEXTURE_SAMPLED, KS_GPU_PROGRAM_PARM_ACCESS_READ_ONLY,
     COMPUTE_PROGRAM_TEXTURE_TIMEWARP_WARP_IMAGE_B, "warpImageB", 3},
    {KS_GPU_PROGRAM_STAGE_FLAG_COMPUTE, KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_VECTOR2, KS_GPU_PROGRAM_PARM_ACCESS_READ_ONLY,
     COMPUTE_PROGRAM_UNIFORM_TIMEWARP_IMAGE_SCALE, "imageScale", 0},
    {KS_GPU_PROGRAM_STAGE_FLAG_COMPUTE, KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_FLOAT_VECTOR2, KS_GPU_PROGRAM_PARM_ACCESS_READ_ONLY,
     COMPUTE_PROGRAM_UNIFORM_TIMEWARP_IMAGE_BIAS, "imageBias", 1},
    {KS_GPU_PROGRAM_STAGE_FLAG_COMPUTE, KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_INT_VECTOR2, KS_GPU_PROGRAM_PARM_ACCESS_READ_ONLY,
     COMPUTE_PROGRAM_UNIFORM_TIMEWARP_EYE_PIXEL_OFFSET, "eyePixelOffset", 3},
    {KS_GPU_PROGRAM_STAGE_FLAG_COMPUTE, KS_GPU_PROGRAM_PARM_TYPE_PUSH_CONSTANT_INT, KS_GPU_PROGRAM_PARM_ACCESS_READ_ONLY,
     COMPUTE_PROGRAM_UNIFORM_TIMEWARP_IMAGE_LAYER, "imageLayer", 2}};

#define CHROMATIC_LOCAL_SIZE_X 8
#define CHROMATIC_LOCAL_SIZE_Y 8

static const char timeWarpChromaticComputeProgramGLSL[] =
    "#version " GLSL_VERSION "\n"
    GLSL_EXTENSIONS
    "\n"
    "layout( local_size_x = " STRINGIFY( CHROMATIC_LOCAL_SIZE_X ) ", local_size_y = " STRINGIFY( CHROMATIC_LOCAL_SIZE_Y ) " ) in;\n"
    "\n"
    "// imageScale = {    eyeTilesWide / ( eyeTilesWide + 1 ) / eyePixelsWide,\n"
    "//                    eyeTilesHigh / ( eyeTilesHigh + 1 ) / eyePixelsHigh };\n"
    "// imageBias  = {    0.5f / ( eyeTilesWide + 1 ),\n"
    "//                    0.5f / ( eyeTilesHigh + 1 ) };\n"
    "layout( rgba8, binding = 0 ) uniform writeonly " ES_HIGHP " image2D dest;\n"
    "uniform highp sampler2DArray eyeImage;\n"
    "uniform highp sampler2D warpImageR;\n"
    "uniform highp sampler2D warpImageG;\n"
    "uniform highp sampler2D warpImageB;\n"
    "uniform highp vec2 imageScale;\n"
    "uniform highp vec2 imageBias;\n"
    "uniform ivec2 eyePixelOffset;\n"
    "uniform int imageLayer;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    vec2 tile = ( vec2( gl_GlobalInvocationID.xy ) + vec2( 0.5f ) ) * imageScale + imageBias;\n"
    "\n"
    "    vec2 eyeCoordsR = texture( warpImageR, tile ).xy;\n"
    "    vec2 eyeCoordsG = texture( warpImageG, tile ).xy;\n"
    "    vec2 eyeCoordsB = texture( warpImageB, tile ).xy;\n"
    "\n"
    "    vec4 rgba;\n"
    "    rgba.x = texture( eyeImage, vec3( eyeCoordsR, imageLayer ) ).x;\n"
    "    rgba.y = texture( eyeImage, vec3( eyeCoordsG, imageLayer ) ).y;\n"
    "    rgba.z = texture( eyeImage, vec3( eyeCoordsB, imageLayer ) ).z;\n"
    "    rgba.w = 1.0f;\n"
    "\n"
    "    imageStore( dest, ivec2( int( gl_GlobalInvocationID.x ) + eyePixelOffset.x, eyePixelOffset.y - 1 - int( gl_GlobalInvocationID.y ) ), rgba );\n"
    "}\n";

static void ksTimeWarpCompute_Create(ksGpuContext *context, ksTimeWarpCompute *compute, const ksHmdInfo *hmdInfo,
                                     ksGpuRenderPass *renderPass, ksGpuWindow *window) {
    memset(compute, 0, sizeof(ksTimeWarpCompute));

    compute->hmdInfo = *hmdInfo;

    const int numMeshCoords = (hmdInfo->eyeTilesHigh + 1) * (hmdInfo->eyeTilesWide + 1);
    ksMeshCoord *meshCoordsBasePtr = (ksMeshCoord *)malloc(NUM_EYES * NUM_COLOR_CHANNELS * numMeshCoords * sizeof(ksMeshCoord));
    ksMeshCoord *meshCoords[NUM_EYES][NUM_COLOR_CHANNELS] = {
        {meshCoordsBasePtr + 0 * numMeshCoords, meshCoordsBasePtr + 1 * numMeshCoords, meshCoordsBasePtr + 2 * numMeshCoords},
        {meshCoordsBasePtr + 3 * numMeshCoords, meshCoordsBasePtr + 4 * numMeshCoords, meshCoordsBasePtr + 5 * numMeshCoords}};
    BuildDistortionMeshes(meshCoords, hmdInfo);

    float *rgbaFloat = (float *)malloc(numMeshCoords * 4 * sizeof(float));
    for (int eye = 0; eye < NUM_EYES; eye++) {
        for (int channel = 0; channel < NUM_COLOR_CHANNELS; channel++) {
            for (int i = 0; i < numMeshCoords; i++) {
                rgbaFloat[i * 4 + 0] = meshCoords[eye][channel][i].x;
                rgbaFloat[i * 4 + 1] = meshCoords[eye][channel][i].y;
                rgbaFloat[i * 4 + 2] = 0.0f;
                rgbaFloat[i * 4 + 3] = 0.0f;
            }
            const size_t rgbaSize = numMeshCoords * 4 * sizeof(float);
            ksGpuTexture_Create2D(context, &compute->distortionImage[eye][channel], KS_GPU_TEXTURE_FORMAT_R32G32B32A32_SFLOAT,
                                  KS_GPU_SAMPLE_COUNT_1, hmdInfo->eyeTilesWide + 1, hmdInfo->eyeTilesHigh + 1, 1,
                                  KS_GPU_TEXTURE_USAGE_STORAGE, rgbaFloat, rgbaSize);
            ksGpuTexture_Create2D(context, &compute->timeWarpImage[eye][channel], KS_GPU_TEXTURE_FORMAT_R16G16B16A16_SFLOAT,
                                  KS_GPU_SAMPLE_COUNT_1, hmdInfo->eyeTilesWide + 1, hmdInfo->eyeTilesHigh + 1, 1,
                                  KS_GPU_TEXTURE_USAGE_STORAGE | KS_GPU_TEXTURE_USAGE_SAMPLED, NULL, 0);
        }
    }
    free(rgbaFloat);

    free(meshCoordsBasePtr);

    ksGpuComputeProgram_Create(context, &compute->timeWarpTransformProgram, PROGRAM(timeWarpTransformComputeProgram),
                               sizeof(PROGRAM(timeWarpTransformComputeProgram)), timeWarpTransformComputeProgramParms,
                               ARRAY_SIZE(timeWarpTransformComputeProgramParms));
    ksGpuComputeProgram_Create(context, &compute->timeWarpSpatialProgram, PROGRAM(timeWarpSpatialComputeProgram),
                               sizeof(PROGRAM(timeWarpSpatialComputeProgram)), timeWarpSpatialComputeProgramParms,
                               ARRAY_SIZE(timeWarpSpatialComputeProgramParms));
    ksGpuComputeProgram_Create(context, &compute->timeWarpChromaticProgram, PROGRAM(timeWarpChromaticComputeProgram),
                               sizeof(PROGRAM(timeWarpChromaticComputeProgram)), timeWarpChromaticComputeProgramParms,
                               ARRAY_SIZE(timeWarpChromaticComputeProgramParms));

    ksGpuComputePipeline_Create(context, &compute->timeWarpTransformPipeline, &compute->timeWarpTransformProgram);
    ksGpuComputePipeline_Create(context, &compute->timeWarpSpatialPipeline, &compute->timeWarpSpatialProgram);
    ksGpuComputePipeline_Create(context, &compute->timeWarpChromaticPipeline, &compute->timeWarpChromaticProgram);

    ksGpuTimer_Create(context, &compute->timeWarpGpuTime);

    ksGpuFramebuffer_CreateFromTextures(context, &compute->framebuffer, renderPass, window->windowWidth, window->windowHeight, 1);
}

static void ksTimeWarpCompute_Destroy(ksGpuContext *context, ksTimeWarpCompute *compute) {
    ksGpuFramebuffer_Destroy(context, &compute->framebuffer);

    ksGpuTimer_Destroy(context, &compute->timeWarpGpuTime);

    ksGpuComputePipeline_Destroy(context, &compute->timeWarpTransformPipeline);
    ksGpuComputePipeline_Destroy(context, &compute->timeWarpSpatialPipeline);
    ksGpuComputePipeline_Destroy(context, &compute->timeWarpChromaticPipeline);

    ksGpuComputeProgram_Destroy(context, &compute->timeWarpTransformProgram);
    ksGpuComputeProgram_Destroy(context, &compute->timeWarpSpatialProgram);
    ksGpuComputeProgram_Destroy(context, &compute->timeWarpChromaticProgram);

    for (int eye = 0; eye < NUM_EYES; eye++) {
        for (int channel = 0; channel < NUM_COLOR_CHANNELS; channel++) {
            ksGpuTexture_Destroy(context, &compute->distortionImage[eye][channel]);
            ksGpuTexture_Destroy(context, &compute->timeWarpImage[eye][channel]);
        }
    }

    memset(compute, 0, sizeof(ksTimeWarpCompute));
}

static void ksTimeWarpCompute_Render(ksGpuCommandBuffer *commandBuffer, ksTimeWarpCompute *compute, ksGpuFramebuffer *framebuffer,
                                     const ksNanoseconds refreshStartTime, const ksNanoseconds refreshEndTime,
                                     const ksMatrix4x4f *projectionMatrix, const ksMatrix4x4f *viewMatrix,
                                     ksGpuTexture *const eyeTexture[NUM_EYES], const int eyeArrayLayer[NUM_EYES],
                                     const bool correctChromaticAberration, ksTimeWarpBarGraphs *bargraphs,
                                     ksNanoseconds cpuTimes[PROFILE_TIME_MAX], ksNanoseconds gpuTimes[PROFILE_TIME_MAX]) {
    const ksNanoseconds t0 = GetTimeNanoseconds();

    ksMatrix4x4f displayRefreshStartViewMatrix;
    ksMatrix4x4f displayRefreshEndViewMatrix;
    GetHmdViewMatrixForTime(&displayRefreshStartViewMatrix, refreshStartTime);
    GetHmdViewMatrixForTime(&displayRefreshEndViewMatrix, refreshEndTime);

    ksMatrix4x4f timeWarpStartTransform;
    ksMatrix4x4f timeWarpEndTransform;
    CalculateTimeWarpTransform(&timeWarpStartTransform, projectionMatrix, viewMatrix, &displayRefreshStartViewMatrix);
    CalculateTimeWarpTransform(&timeWarpEndTransform, projectionMatrix, viewMatrix, &displayRefreshEndViewMatrix);

    ksMatrix3x4f timeWarpStartTransform3x4;
    ksMatrix3x4f timeWarpEndTransform3x4;
    ksMatrix3x4f_CreateFromMatrix4x4f(&timeWarpStartTransform3x4, &timeWarpStartTransform);
    ksMatrix3x4f_CreateFromMatrix4x4f(&timeWarpEndTransform3x4, &timeWarpEndTransform);

    ksGpuCommandBuffer_BeginPrimary(commandBuffer);
    ksGpuCommandBuffer_BeginFramebuffer(commandBuffer, &compute->framebuffer, 0, KS_GPU_TEXTURE_USAGE_STORAGE);

    ksGpuCommandBuffer_BeginTimer(commandBuffer, &compute->timeWarpGpuTime);

    for (int eye = 0; eye < NUM_EYES; eye++) {
        for (int channel = 0; channel < NUM_COLOR_CHANNELS; channel++) {
            ksGpuCommandBuffer_ChangeTextureUsage(commandBuffer, &compute->timeWarpImage[eye][channel],
                                                  KS_GPU_TEXTURE_USAGE_STORAGE);
            ksGpuCommandBuffer_ChangeTextureUsage(commandBuffer, &compute->distortionImage[eye][channel],
                                                  KS_GPU_TEXTURE_USAGE_STORAGE);
        }
    }

    const ksVector2i dimensions = {compute->hmdInfo.eyeTilesWide + 1, compute->hmdInfo.eyeTilesHigh + 1};
    const int eyeIndex[NUM_EYES] = {0, 1};

    for (int eye = 0; eye < NUM_EYES; eye++) {
        for (int channel = 0; channel < NUM_COLOR_CHANNELS; channel++) {
            ksGpuComputeCommand command;
            ksGpuComputeCommand_Init(&command);
            ksGpuComputeCommand_SetPipeline(&command, &compute->timeWarpTransformPipeline);
            ksGpuComputeCommand_SetParmTextureStorage(&command, COMPUTE_PROGRAM_TEXTURE_TIMEWARP_TRANSFORM_DST,
                                                      &compute->timeWarpImage[eye][channel]);
            ksGpuComputeCommand_SetParmTextureStorage(&command, COMPUTE_PROGRAM_TEXTURE_TIMEWARP_TRANSFORM_SRC,
                                                      &compute->distortionImage[eye][channel]);
            ksGpuComputeCommand_SetParmFloatMatrix3x4(&command, COMPUTE_PROGRAM_UNIFORM_TIMEWARP_START_TRANSFORM,
                                                      &timeWarpStartTransform3x4);
            ksGpuComputeCommand_SetParmFloatMatrix3x4(&command, COMPUTE_PROGRAM_UNIFORM_TIMEWARP_END_TRANSFORM,
                                                      &timeWarpEndTransform3x4);
            ksGpuComputeCommand_SetParmIntVector2(&command, COMPUTE_PROGRAM_UNIFORM_TIMEWARP_DIMENSIONS, &dimensions);
            ksGpuComputeCommand_SetParmInt(&command, COMPUTE_PROGRAM_UNIFORM_TIMEWARP_EYE, &eyeIndex[eye]);
            ksGpuComputeCommand_SetDimensions(&command, (dimensions.x + TRANSFORM_LOCAL_SIZE_X - 1) / TRANSFORM_LOCAL_SIZE_X,
                                              (dimensions.y + TRANSFORM_LOCAL_SIZE_Y - 1) / TRANSFORM_LOCAL_SIZE_Y, 1);

            ksGpuCommandBuffer_SubmitComputeCommand(commandBuffer, &command);
        }
    }

    for (int eye = 0; eye < NUM_EYES; eye++) {
        for (int channel = 0; channel < NUM_COLOR_CHANNELS; channel++) {
            ksGpuCommandBuffer_ChangeTextureUsage(commandBuffer, &compute->timeWarpImage[eye][channel],
                                                  KS_GPU_TEXTURE_USAGE_SAMPLED);
        }
    }
    ksGpuCommandBuffer_ChangeTextureUsage(commandBuffer, ksGpuFramebuffer_GetColorTexture(&compute->framebuffer),
                                          KS_GPU_TEXTURE_USAGE_STORAGE);

    const int screenWidth = ksGpuFramebuffer_GetWidth(&compute->framebuffer);
    const int screenHeight = ksGpuFramebuffer_GetHeight(&compute->framebuffer);
    const int eyePixelsWide = screenWidth / NUM_EYES;
    const int eyePixelsHigh =
        screenHeight * compute->hmdInfo.eyeTilesHigh * compute->hmdInfo.tilePixelsHigh / compute->hmdInfo.displayPixelsHigh;
    const ksVector2f imageScale = {(float)compute->hmdInfo.eyeTilesWide / (compute->hmdInfo.eyeTilesWide + 1) / eyePixelsWide,
                                   (float)compute->hmdInfo.eyeTilesHigh / (compute->hmdInfo.eyeTilesHigh + 1) / eyePixelsHigh};
    const ksVector2f imageBias = {0.5f / (compute->hmdInfo.eyeTilesWide + 1), 0.5f / (compute->hmdInfo.eyeTilesHigh + 1)};
    const ksVector2i eyePixelOffset[NUM_EYES] = {
#if defined(GRAPHICS_API_VULKAN)
        {0 * eyePixelsWide, screenHeight - eyePixelsHigh},
        {1 * eyePixelsWide, screenHeight - eyePixelsHigh}
#else
        {0 * eyePixelsWide, eyePixelsHigh},
        {1 * eyePixelsWide, eyePixelsHigh}
#endif
    };

    for (int eye = 0; eye < NUM_EYES; eye++) {
        assert(screenWidth % (correctChromaticAberration ? CHROMATIC_LOCAL_SIZE_X : SPATIAL_LOCAL_SIZE_X) == 0);
        assert(screenHeight % (correctChromaticAberration ? CHROMATIC_LOCAL_SIZE_Y : SPATIAL_LOCAL_SIZE_Y) == 0);

        ksGpuComputeCommand command;
        ksGpuComputeCommand_Init(&command);
        ksGpuComputeCommand_SetPipeline(
            &command, correctChromaticAberration ? &compute->timeWarpChromaticPipeline : &compute->timeWarpSpatialPipeline);
        ksGpuComputeCommand_SetParmTextureStorage(&command, COMPUTE_PROGRAM_TEXTURE_TIMEWARP_DEST,
                                                  ksGpuFramebuffer_GetColorTexture(&compute->framebuffer));
        ksGpuComputeCommand_SetParmTextureSampled(&command, COMPUTE_PROGRAM_TEXTURE_TIMEWARP_EYE_IMAGE, eyeTexture[eye]);
        ksGpuComputeCommand_SetParmTextureSampled(&command, COMPUTE_PROGRAM_TEXTURE_TIMEWARP_WARP_IMAGE_R,
                                                  &compute->timeWarpImage[eye][0]);
        ksGpuComputeCommand_SetParmTextureSampled(&command, COMPUTE_PROGRAM_TEXTURE_TIMEWARP_WARP_IMAGE_G,
                                                  &compute->timeWarpImage[eye][1]);
        ksGpuComputeCommand_SetParmTextureSampled(&command, COMPUTE_PROGRAM_TEXTURE_TIMEWARP_WARP_IMAGE_B,
                                                  &compute->timeWarpImage[eye][2]);
        ksGpuComputeCommand_SetParmFloatVector2(&command, COMPUTE_PROGRAM_UNIFORM_TIMEWARP_IMAGE_SCALE, &imageScale);
        ksGpuComputeCommand_SetParmFloatVector2(&command, COMPUTE_PROGRAM_UNIFORM_TIMEWARP_IMAGE_BIAS, &imageBias);
        ksGpuComputeCommand_SetParmIntVector2(&command, COMPUTE_PROGRAM_UNIFORM_TIMEWARP_EYE_PIXEL_OFFSET, &eyePixelOffset[eye]);
        ksGpuComputeCommand_SetParmInt(&command, COMPUTE_PROGRAM_UNIFORM_TIMEWARP_IMAGE_LAYER, &eyeArrayLayer[eye]);
        ksGpuComputeCommand_SetDimensions(
            &command, screenWidth / (correctChromaticAberration ? CHROMATIC_LOCAL_SIZE_X : SPATIAL_LOCAL_SIZE_X) / 2,
            screenHeight / (correctChromaticAberration ? CHROMATIC_LOCAL_SIZE_Y : SPATIAL_LOCAL_SIZE_Y), 1);

        ksGpuCommandBuffer_SubmitComputeCommand(commandBuffer, &command);
    }

    const ksNanoseconds t1 = GetTimeNanoseconds();

    ksTimeWarpBarGraphs_UpdateCompute(commandBuffer, bargraphs);
    ksTimeWarpBarGraphs_RenderCompute(commandBuffer, bargraphs, &compute->framebuffer);

    const ksNanoseconds t2 = GetTimeNanoseconds();

    ksGpuCommandBuffer_Blit(commandBuffer, &compute->framebuffer, framebuffer);

    ksGpuCommandBuffer_EndTimer(commandBuffer, &compute->timeWarpGpuTime);

    ksGpuCommandBuffer_EndFramebuffer(commandBuffer, &compute->framebuffer, 0, KS_GPU_TEXTURE_USAGE_PRESENTATION);
    ksGpuCommandBuffer_EndPrimary(commandBuffer);

    ksGpuCommandBuffer_SubmitPrimary(commandBuffer);

    const ksNanoseconds t3 = GetTimeNanoseconds();

    cpuTimes[PROFILE_TIME_TIME_WARP] = t1 - t0;
    cpuTimes[PROFILE_TIME_BAR_GRAPHS] = t2 - t1;
    cpuTimes[PROFILE_TIME_BLIT] = t3 - t2;

    const ksNanoseconds barGraphGpuTime = ksTimeWarpBarGraphs_GetGpuNanosecondsCompute(bargraphs);

    gpuTimes[PROFILE_TIME_TIME_WARP] = ksGpuTimer_GetNanoseconds(&compute->timeWarpGpuTime) - barGraphGpuTime;
    gpuTimes[PROFILE_TIME_BAR_GRAPHS] = barGraphGpuTime;
    gpuTimes[PROFILE_TIME_BLIT] = 0;

#if GL_FINISH_SYNC == 1
    GL(glFinish());
#endif
}

#else

typedef struct {
    int empty;
} ksTimeWarpCompute;

static void ksTimeWarpCompute_Create(ksGpuContext *context, ksTimeWarpCompute *compute, const ksHmdInfo *hmdInfo,
                                     ksGpuRenderPass *renderPass, ksGpuWindow *window) {
    UNUSED_PARM(context);
    UNUSED_PARM(compute);
    UNUSED_PARM(hmdInfo);
    UNUSED_PARM(renderPass);
    UNUSED_PARM(window);
}

static void ksTimeWarpCompute_Destroy(ksGpuContext *context, ksTimeWarpCompute *compute) {
    UNUSED_PARM(context);
    UNUSED_PARM(compute);
}

static void ksTimeWarpCompute_Render(ksGpuCommandBuffer *commandBuffer, ksTimeWarpCompute *compute, ksGpuFramebuffer *framebuffer,
                                     const ksNanoseconds refreshStartTime, const ksNanoseconds refreshEndTime,
                                     const ksMatrix4x4f *projectionMatrix, const ksMatrix4x4f *viewMatrix,
                                     ksGpuTexture *const eyeTexture[NUM_EYES], const int eyeArrayLayer[NUM_EYES],
                                     const bool correctChromaticAberration, ksTimeWarpBarGraphs *bargraphs,
                                     ksNanoseconds cpuTimes[PROFILE_TIME_MAX], ksNanoseconds gpuTimes[PROFILE_TIME_MAX]) {
    UNUSED_PARM(commandBuffer);
    UNUSED_PARM(compute);
    UNUSED_PARM(framebuffer);
    UNUSED_PARM(refreshStartTime);
    UNUSED_PARM(refreshEndTime);
    UNUSED_PARM(viewMatrix);
    UNUSED_PARM(eyeTexture);
    UNUSED_PARM(eyeArrayLayer);
    UNUSED_PARM(correctChromaticAberration);
    UNUSED_PARM(bargraphs);
    UNUSED_PARM(cpuTimes);
    UNUSED_PARM(gpuTimes);
}

#endif

/*
================================================================================================================================

Time warp rendering.

ksTimeWarp
ksTimeWarpImplementation

static void ksTimeWarp_Create( ksTimeWarp * timeWarp, ksGpuWindow * window );
static void ksTimeWarp_Destroy( ksTimeWarp * timeWarp, ksGpuWindow * window );

static void ksTimeWarp_SetBarGraphState( ksTimeWarp * timeWarp, const ksBarGraphState state );
static void ksTimeWarp_CycleBarGraphState( ksTimeWarp * timeWarp );
static void ksTimeWarp_SetImplementation( ksTimeWarp * timeWarp, const ksTimeWarpImplementation implementation );
static void ksTimeWarp_CycleImplementation( ksTimeWarp * timeWarp );
static void ksTimeWarp_SetChromaticAberrationCorrection( ksTimeWarp * timeWarp, const bool set );
static void ksTimeWarp_ToggleChromaticAberrationCorrection( ksTimeWarp * timeWarp );
static void ksTimeWarp_SetMultiView( ksTimeWarp * timeWarp, const bool enabled );

static void ksTimeWarp_SetDisplayResolutionLevel( ksTimeWarp * timeWarp, const int level );
static void ksTimeWarp_SetEyeImageResolutionLevel( ksTimeWarp * timeWarp, const int level );
static void ksTimeWarp_SetEyeImageSamplesLevel( ksTimeWarp * timeWarp, const int level );

static void ksTimeWarp_SetDrawCallLevel( ksTimeWarp * timeWarp, const int level );
static void ksTimeWarp_SetTriangleLevel( ksTimeWarp * timeWarp, const int level );
static void ksTimeWarp_SetFragmentLevel( ksTimeWarp * timeWarp, const int level );

static ksNanoseconds ksTimeWarp_GetPredictedDisplayTime( ksTimeWarp * timeWarp, const int frameIndex );
static void ksTimeWarp_SubmitFrame( ksTimeWarp * timeWarp, const int frameIndex, const ksNanoseconds displayTime,
                                                                        const ksMatrix4x4f * viewMatrix, const Matrix4x4_t *
projectionMatrix, ksGpuTexture * eyeTexture[NUM_EYES], ksGpuFence * eyeCompletionFence[NUM_EYES], int eyeArrayLayer[NUM_EYES],
ksNanoseconds eyeTexturesCpuTime, ksNanoseconds eyeTexturesGpuTime ); static void ksTimeWarp_Render( ksTimeWarp * timeWarp );

================================================================================================================================
*/

#define AVERAGE_FRAME_RATE_FRAMES 20

typedef enum {
    TIMEWARP_IMPLEMENTATION_GRAPHICS,
    TIMEWARP_IMPLEMENTATION_COMPUTE,
    TIMEWARP_IMPLEMENTATION_MAX
} ksTimeWarpImplementation;

typedef struct {
    int index;
    int frameIndex;
    ksNanoseconds displayTime;
    ksMatrix4x4f viewMatrix;
    ksMatrix4x4f projectionMatrix;
    ksGpuTexture *texture[NUM_EYES];
    ksGpuFence *completionFence[NUM_EYES];
    int arrayLayer[NUM_EYES];
    ksNanoseconds cpuTime;
    ksNanoseconds gpuTime;
} ksEyeTextures;

typedef struct {
    long long frameIndex;
    ksNanoseconds vsyncTime;
    ksNanoseconds frameTime;
} ksFrameTiming;

typedef struct {
    ksGpuWindow *window;
    ksGpuTexture defaultTexture;
    ksNanoseconds displayTime;
    ksMatrix4x4f viewMatrix;
    ksMatrix4x4f projectionMatrix;
    ksGpuTexture *eyeTexture[NUM_EYES];
    int eyeArrayLayer[NUM_EYES];

    ksMutex newEyeTexturesMutex;
    ksSignal newEyeTexturesConsumed;
    ksEyeTextures newEyeTextures;
    int eyeTexturesPresentIndex;
    int eyeTexturesConsumedIndex;

    ksFrameTiming frameTiming;
    ksMutex frameTimingMutex;
    ksSignal vsyncSignal;

    float refreshRate;
    ksNanoseconds frameCpuTime[AVERAGE_FRAME_RATE_FRAMES];
    int eyeTexturesFrames[AVERAGE_FRAME_RATE_FRAMES];
    int timeWarpFrames;
    ksNanoseconds cpuTimes[PROFILE_TIME_MAX];
    ksNanoseconds gpuTimes[PROFILE_TIME_MAX];

    ksGpuRenderPass renderPass;
    ksGpuFramebuffer framebuffer;
    ksGpuCommandBuffer commandBuffer;
    bool correctChromaticAberration;
    ksTimeWarpImplementation implementation;
    ksTimeWarpGraphics graphics;
    ksTimeWarpCompute compute;
    ksTimeWarpBarGraphs bargraphs;
} ksTimeWarp;

static void ksTimeWarp_Create(ksTimeWarp *timeWarp, ksGpuWindow *window) {
    timeWarp->window = window;

    ksGpuTexture_CreateDefault(&window->context, &timeWarp->defaultTexture, KS_GPU_TEXTURE_DEFAULT_CIRCLES, 1024, 1024, 0, 2, 1,
                               false, true);
    ksGpuTexture_SetWrapMode(&window->context, &timeWarp->defaultTexture, KS_GPU_TEXTURE_WRAP_MODE_CLAMP_TO_BORDER);

    ksMutex_Create(&timeWarp->newEyeTexturesMutex);
    ksSignal_Create(&timeWarp->newEyeTexturesConsumed, true);
    ksSignal_Raise(&timeWarp->newEyeTexturesConsumed);

    timeWarp->newEyeTextures.index = 0;
    timeWarp->newEyeTextures.displayTime = 0;
    ksMatrix4x4f_CreateIdentity(&timeWarp->newEyeTextures.viewMatrix);
    ksMatrix4x4f_CreateProjectionFov(&timeWarp->newEyeTextures.projectionMatrix, 40.0f, 40.0f, 40.0f, 40.0f, 0.1f, 0.0f);
    for (int eye = 0; eye < NUM_EYES; eye++) {
        timeWarp->newEyeTextures.texture[eye] = &timeWarp->defaultTexture;
        timeWarp->newEyeTextures.completionFence[eye] = NULL;
        timeWarp->newEyeTextures.arrayLayer[eye] = eye;
    }
    timeWarp->newEyeTextures.cpuTime = 0;
    timeWarp->newEyeTextures.gpuTime = 0;

    timeWarp->displayTime = 0;
    timeWarp->viewMatrix = timeWarp->newEyeTextures.viewMatrix;
    timeWarp->projectionMatrix = timeWarp->newEyeTextures.projectionMatrix;
    for (int eye = 0; eye < NUM_EYES; eye++) {
        timeWarp->eyeTexture[eye] = timeWarp->newEyeTextures.texture[eye];
        timeWarp->eyeArrayLayer[eye] = timeWarp->newEyeTextures.arrayLayer[eye];
    }

    timeWarp->eyeTexturesPresentIndex = 1;
    timeWarp->eyeTexturesConsumedIndex = 0;

    timeWarp->frameTiming.frameIndex = 0;
    timeWarp->frameTiming.vsyncTime = 0;
    timeWarp->frameTiming.frameTime = 0;
    ksMutex_Create(&timeWarp->frameTimingMutex);
    ksSignal_Create(&timeWarp->vsyncSignal, false);

    timeWarp->refreshRate = window->windowRefreshRate;
    for (int i = 0; i < AVERAGE_FRAME_RATE_FRAMES; i++) {
        timeWarp->frameCpuTime[i] = 0;
        timeWarp->eyeTexturesFrames[i] = 0;
    }
    timeWarp->timeWarpFrames = 0;

    ksGpuRenderPass_Create(&window->context, &timeWarp->renderPass, window->colorFormat, window->depthFormat, KS_GPU_SAMPLE_COUNT_1,
                           KS_GPU_RENDERPASS_TYPE_INLINE, KS_GPU_RENDERPASS_FLAG_CLEAR_COLOR_BUFFER);
    ksGpuFramebuffer_CreateFromSwapchain(window, &timeWarp->framebuffer, &timeWarp->renderPass);
    ksGpuCommandBuffer_Create(&window->context, &timeWarp->commandBuffer, KS_GPU_COMMAND_BUFFER_TYPE_PRIMARY,
                              ksGpuFramebuffer_GetBufferCount(&timeWarp->framebuffer));

    timeWarp->correctChromaticAberration = false;
    timeWarp->implementation = TIMEWARP_IMPLEMENTATION_GRAPHICS;

    const ksHmdInfo *hmdInfo = GetDefaultHmdInfo(window->windowWidth, window->windowHeight);

    ksTimeWarpGraphics_Create(&window->context, &timeWarp->graphics, hmdInfo, &timeWarp->renderPass);
    ksTimeWarpCompute_Create(&window->context, &timeWarp->compute, hmdInfo, &timeWarp->renderPass, window);
    ksTimeWarpBarGraphs_Create(&window->context, &timeWarp->bargraphs, &timeWarp->renderPass);

    memset(timeWarp->cpuTimes, 0, sizeof(timeWarp->cpuTimes));
    memset(timeWarp->gpuTimes, 0, sizeof(timeWarp->gpuTimes));
}

static void ksTimeWarp_Destroy(ksTimeWarp *timeWarp, ksGpuWindow *window) {
    ksGpuContext_WaitIdle(&window->context);

    ksTimeWarpGraphics_Destroy(&window->context, &timeWarp->graphics);
    ksTimeWarpCompute_Destroy(&window->context, &timeWarp->compute);
    ksTimeWarpBarGraphs_Destroy(&window->context, &timeWarp->bargraphs);

    ksGpuCommandBuffer_Destroy(&window->context, &timeWarp->commandBuffer);
    ksGpuFramebuffer_Destroy(&window->context, &timeWarp->framebuffer);
    ksGpuRenderPass_Destroy(&window->context, &timeWarp->renderPass);

    ksSignal_Destroy(&timeWarp->newEyeTexturesConsumed);
    ksMutex_Destroy(&timeWarp->newEyeTexturesMutex);
    ksMutex_Destroy(&timeWarp->frameTimingMutex);
    ksSignal_Destroy(&timeWarp->vsyncSignal);

    ksGpuTexture_Destroy(&window->context, &timeWarp->defaultTexture);
}

static void ksTimeWarp_SetBarGraphState(ksTimeWarp *timeWarp, const ksBarGraphState state) {
    timeWarp->bargraphs.barGraphState = state;
}

static void ksTimeWarp_CycleBarGraphState(ksTimeWarp *timeWarp) {
    timeWarp->bargraphs.barGraphState = (ksBarGraphState)((timeWarp->bargraphs.barGraphState + 1) % 3);
}

static void ksTimeWarp_SetImplementation(ksTimeWarp *timeWarp, const ksTimeWarpImplementation implementation) {
    timeWarp->implementation = implementation;
    const float delta = (timeWarp->implementation == TIMEWARP_IMPLEMENTATION_GRAPHICS) ? 0.0f : 1.0f;
    ksBarGraph_AddBar(&timeWarp->bargraphs.timeWarpImplementationBarGraph, 0, delta, &ksColorRed, false);
}

static void ksTimeWarp_CycleImplementation(ksTimeWarp *timeWarp) {
    timeWarp->implementation = (ksTimeWarpImplementation)((timeWarp->implementation + 1) % TIMEWARP_IMPLEMENTATION_MAX);
#if OPENGL_COMPUTE_ENABLED == 0
    if (timeWarp->implementation == TIMEWARP_IMPLEMENTATION_COMPUTE) {
        timeWarp->implementation = (ksTimeWarpImplementation)((timeWarp->implementation + 1) % TIMEWARP_IMPLEMENTATION_MAX);
    }
#endif
    const float delta = (timeWarp->implementation == TIMEWARP_IMPLEMENTATION_GRAPHICS) ? 0.0f : 1.0f;
    ksBarGraph_AddBar(&timeWarp->bargraphs.timeWarpImplementationBarGraph, 0, delta, &ksColorRed, false);
}

static void ksTimeWarp_SetChromaticAberrationCorrection(ksTimeWarp *timeWarp, const bool set) {
    timeWarp->correctChromaticAberration = set;
    ksBarGraph_AddBar(&timeWarp->bargraphs.correctChromaticAberrationBarGraph, 0,
                      timeWarp->correctChromaticAberration ? 1.0f : 0.0f, &ksColorRed, false);
}

static void ksTimeWarp_ToggleChromaticAberrationCorrection(ksTimeWarp *timeWarp) {
    timeWarp->correctChromaticAberration = !timeWarp->correctChromaticAberration;
    ksBarGraph_AddBar(&timeWarp->bargraphs.correctChromaticAberrationBarGraph, 0,
                      timeWarp->correctChromaticAberration ? 1.0f : 0.0f, &ksColorRed, false);
}

static void ksTimeWarp_SetMultiView(ksTimeWarp *timeWarp, const bool enabled) {
    ksBarGraph_AddBar(&timeWarp->bargraphs.multiViewBarGraph, 0, enabled ? 1.0f : 0.0f, &ksColorRed, false);
}

static void ksTimeWarp_SetDisplayResolutionLevel(ksTimeWarp *timeWarp, const int level) {
    const ksVector4f *levelColor[4] = {&ksColorBlue, &ksColorGreen, &ksColorYellow, &ksColorRed};
    for (int i = 0; i < 4; i++) {
        ksBarGraph_AddBar(&timeWarp->bargraphs.displayResolutionLevelBarGraph, i, (i <= level) ? 0.25f : 0.0f, levelColor[i],
                          false);
    }
}

static void ksTimeWarp_SetEyeImageResolutionLevel(ksTimeWarp *timeWarp, const int level) {
    const ksVector4f *levelColor[4] = {&ksColorBlue, &ksColorGreen, &ksColorYellow, &ksColorRed};
    for (int i = 0; i < 4; i++) {
        ksBarGraph_AddBar(&timeWarp->bargraphs.eyeImageResolutionLevelBarGraph, i, (i <= level) ? 0.25f : 0.0f, levelColor[i],
                          false);
    }
}

static void ksTimeWarp_SetEyeImageSamplesLevel(ksTimeWarp *timeWarp, const int level) {
    const ksVector4f *levelColor[4] = {&ksColorBlue, &ksColorGreen, &ksColorYellow, &ksColorRed};
    for (int i = 0; i < 4; i++) {
        ksBarGraph_AddBar(&timeWarp->bargraphs.eyeImageSamplesLevelBarGraph, i, (i <= level) ? 0.25f : 0.0f, levelColor[i], false);
    }
}

static void ksTimeWarp_SetDrawCallLevel(ksTimeWarp *timeWarp, const int level) {
    const ksVector4f *levelColor[4] = {&ksColorBlue, &ksColorGreen, &ksColorYellow, &ksColorRed};
    for (int i = 0; i < 4; i++) {
        ksBarGraph_AddBar(&timeWarp->bargraphs.sceneDrawCallLevelBarGraph, i, (i <= level) ? 0.25f : 0.0f, levelColor[i], false);
    }
}

static void ksTimeWarp_SetTriangleLevel(ksTimeWarp *timeWarp, const int level) {
    const ksVector4f *levelColor[4] = {&ksColorBlue, &ksColorGreen, &ksColorYellow, &ksColorRed};
    for (int i = 0; i < 4; i++) {
        ksBarGraph_AddBar(&timeWarp->bargraphs.sceneTriangleLevelBarGraph, i, (i <= level) ? 0.25f : 0.0f, levelColor[i], false);
    }
}

static void ksTimeWarp_SetFragmentLevel(ksTimeWarp *timeWarp, const int level) {
    const ksVector4f *levelColor[4] = {&ksColorBlue, &ksColorGreen, &ksColorYellow, &ksColorRed};
    for (int i = 0; i < 4; i++) {
        ksBarGraph_AddBar(&timeWarp->bargraphs.sceneFragmentLevelBarGraph, i, (i <= level) ? 0.25f : 0.0f, levelColor[i], false);
    }
}

static ksNanoseconds ksTimeWarp_GetPredictedDisplayTime(ksTimeWarp *timeWarp, const int frameIndex) {
    ksMutex_Lock(&timeWarp->frameTimingMutex, true);
    const ksFrameTiming frameTiming = timeWarp->frameTiming;
    ksMutex_Unlock(&timeWarp->frameTimingMutex);

    // The time warp thread is currently released by SwapBuffers shortly after a V-Sync.
    // Where possible, the time warp thread then waits until a short time before the next V-Sync,
    // giving it just enough time to warp the last completed application frame onto the display.
    // The time warp thread then tries to pick up the latest completed application frame and warps
    // the frame onto the display. The application thread is released right after the V-Sync
    // and can start working on a new frame that will be displayed effectively 2 display refresh
    // cycles in the future.

    return frameTiming.vsyncTime + (frameIndex - frameTiming.frameIndex) * frameTiming.frameTime;
}

static void ksTimeWarp_SubmitFrame(ksTimeWarp *timeWarp, const int frameIndex, const ksNanoseconds displayTime,
                                   const ksMatrix4x4f *viewMatrix, const ksMatrix4x4f *projectionMatrix,
                                   ksGpuTexture *eyeTexture[NUM_EYES], ksGpuFence *eyeCompletionFence[NUM_EYES],
                                   int eyeArrayLayer[NUM_EYES], ksNanoseconds eyeTexturesCpuTime,
                                   ksNanoseconds eyeTexturesGpuTime) {
    ksEyeTextures newEyeTextures;
    newEyeTextures.index = timeWarp->eyeTexturesPresentIndex++;
    newEyeTextures.frameIndex = frameIndex;
    newEyeTextures.displayTime = displayTime;
    newEyeTextures.viewMatrix = *viewMatrix;
    newEyeTextures.projectionMatrix = *projectionMatrix;
    for (int eye = 0; eye < NUM_EYES; eye++) {
        newEyeTextures.texture[eye] = eyeTexture[eye];
        newEyeTextures.completionFence[eye] = eyeCompletionFence[eye];
        newEyeTextures.arrayLayer[eye] = eyeArrayLayer[eye];
    }
    newEyeTextures.cpuTime = eyeTexturesCpuTime;
    newEyeTextures.gpuTime = eyeTexturesGpuTime;

    // Wait for the previous eye textures to be consumed before overwriting them.
    ksSignal_Wait(&timeWarp->newEyeTexturesConsumed, SIGNAL_TIMEOUT_INFINITE);

    ksMutex_Lock(&timeWarp->newEyeTexturesMutex, true);
    timeWarp->newEyeTextures = newEyeTextures;
    ksMutex_Unlock(&timeWarp->newEyeTexturesMutex);

    // Wait for at least one V-Sync to pass to avoid piling up frames of latency.
    ksSignal_Wait(&timeWarp->vsyncSignal, SIGNAL_TIMEOUT_INFINITE);

    ksFrameTiming newFrameTiming;
    newFrameTiming.frameIndex = frameIndex;
    newFrameTiming.vsyncTime = ksGpuWindow_GetNextSwapTimeNanoseconds(timeWarp->window);
    newFrameTiming.frameTime = ksGpuWindow_GetFrameTimeNanoseconds(timeWarp->window);

    ksMutex_Lock(&timeWarp->frameTimingMutex, true);
    timeWarp->frameTiming = newFrameTiming;
    ksMutex_Unlock(&timeWarp->frameTimingMutex);
}

static void ksTimeWarp_Render(ksTimeWarp *timeWarp) {
    const ksNanoseconds nextSwapTime = ksGpuWindow_GetNextSwapTimeNanoseconds(timeWarp->window);
    const ksNanoseconds frameTime = ksGpuWindow_GetFrameTimeNanoseconds(timeWarp->window);

    // Wait until close to the next V-Sync but still far enough away to allow the time warp to complete rendering.
    ksGpuWindow_DelayBeforeSwap(timeWarp->window, frameTime / 2);

    timeWarp->eyeTexturesFrames[timeWarp->timeWarpFrames % AVERAGE_FRAME_RATE_FRAMES] = 0;

    // Try to pick up the latest eye textures but never block the time warp thread.
    // It is better to display an old set of eye textures than to miss the next V-Sync
    // in case another thread is suspended while holding on to the mutex.
    if (ksMutex_Lock(&timeWarp->newEyeTexturesMutex, false)) {
        ksEyeTextures newEyeTextures = timeWarp->newEyeTextures;
        ksMutex_Unlock(&timeWarp->newEyeTexturesMutex);

        // If this is a new set of eye textures.
        if (newEyeTextures.index > timeWarp->eyeTexturesConsumedIndex &&
            // Never display the eye textures before they are meant to be displayed.
            newEyeTextures.displayTime < nextSwapTime + frameTime / 2 &&
            // Make sure both eye textures have completed rendering.
            ksGpuFence_IsSignalled(&timeWarp->window->context, newEyeTextures.completionFence[0]) &&
            ksGpuFence_IsSignalled(&timeWarp->window->context, newEyeTextures.completionFence[1])) {
            assert(newEyeTextures.index == timeWarp->eyeTexturesConsumedIndex + 1);
            timeWarp->eyeTexturesConsumedIndex = newEyeTextures.index;
            timeWarp->displayTime = newEyeTextures.displayTime;
            timeWarp->projectionMatrix = newEyeTextures.projectionMatrix;
            timeWarp->viewMatrix = newEyeTextures.viewMatrix;
            for (int eye = 0; eye < NUM_EYES; eye++) {
                timeWarp->eyeTexture[eye] = newEyeTextures.texture[eye];
                timeWarp->eyeArrayLayer[eye] = newEyeTextures.arrayLayer[eye];
            }
            timeWarp->cpuTimes[PROFILE_TIME_APPLICATION] = newEyeTextures.cpuTime;
            timeWarp->gpuTimes[PROFILE_TIME_APPLICATION] = newEyeTextures.gpuTime;
            timeWarp->eyeTexturesFrames[timeWarp->timeWarpFrames % AVERAGE_FRAME_RATE_FRAMES] = 1;
            ksSignal_Clear(&timeWarp->vsyncSignal);
            ksSignal_Raise(&timeWarp->newEyeTexturesConsumed);
        }
    }

    // Calculate the eye texture and time warp frame rates.
    float timeWarpFrameRate = timeWarp->refreshRate;
    float eyeTexturesFrameRate = timeWarp->refreshRate;
    {
        ksNanoseconds lastTime = timeWarp->frameCpuTime[timeWarp->timeWarpFrames % AVERAGE_FRAME_RATE_FRAMES];
        ksNanoseconds time = nextSwapTime;
        timeWarp->frameCpuTime[timeWarp->timeWarpFrames % AVERAGE_FRAME_RATE_FRAMES] = time;
        timeWarp->timeWarpFrames++;
        if (timeWarp->timeWarpFrames > AVERAGE_FRAME_RATE_FRAMES) {
            int timeWarpFrames = AVERAGE_FRAME_RATE_FRAMES;
            int eyeTexturesFrames = 0;
            for (int i = 0; i < AVERAGE_FRAME_RATE_FRAMES; i++) {
                eyeTexturesFrames += timeWarp->eyeTexturesFrames[i];
            }

            timeWarpFrameRate = timeWarpFrames * 1e9f / (time - lastTime);
            eyeTexturesFrameRate = eyeTexturesFrames * 1e9f / (time - lastTime);
        }
    }

    // Update the bar graphs if not paused.
    if (timeWarp->bargraphs.barGraphState == BAR_GRAPH_VISIBLE) {
        const ksVector4f *applicationFrameRateColor =
            (eyeTexturesFrameRate > timeWarp->refreshRate - 0.5f) ? &ksColorPurple : &ksColorRed;
        const ksVector4f *timeWarpFrameRateColor = (timeWarpFrameRate > timeWarp->refreshRate - 0.5f) ? &ksColorGreen : &ksColorRed;

        ksBarGraph_AddBar(&timeWarp->bargraphs.applicationFrameRateGraph, 0, eyeTexturesFrameRate / timeWarp->refreshRate,
                          applicationFrameRateColor, true);
        ksBarGraph_AddBar(&timeWarp->bargraphs.timeWarpFrameRateGraph, 0, timeWarpFrameRate / timeWarp->refreshRate,
                          timeWarpFrameRateColor, true);

        for (int i = 0; i < 2; i++) {
            const ksNanoseconds *times = (i == 0) ? timeWarp->cpuTimes : timeWarp->gpuTimes;
            float barHeights[PROFILE_TIME_MAX];
            float totalBarHeight = 0.0f;
            for (int p = 0; p < PROFILE_TIME_MAX; p++) {
                barHeights[p] = times[p] * timeWarp->refreshRate * 1e-9f;
                totalBarHeight += barHeights[p];
            }

            const float limit = 0.9f;
            if (totalBarHeight > limit) {
                totalBarHeight = 0.0f;
                for (int p = 0; p < PROFILE_TIME_MAX; p++) {
                    barHeights[p] = (totalBarHeight + barHeights[p] > limit) ? (limit - totalBarHeight) : barHeights[p];
                    totalBarHeight += barHeights[p];
                }
                barHeights[PROFILE_TIME_OVERFLOW] = 1.0f - limit;
            }

            ksBarGraph *barGraph = (i == 0) ? &timeWarp->bargraphs.frameCpuTimeBarGraph : &timeWarp->bargraphs.frameGpuTimeBarGraph;
            for (int p = 0; p < PROFILE_TIME_MAX; p++) {
                ksBarGraph_AddBar(barGraph, p, barHeights[p], profileTimeBarColors[p], (p == PROFILE_TIME_MAX - 1));
            }
        }
    }

    ksFrameLog_BeginFrame();

    // assert( timeWarp->displayTime == nextSwapTime );
    const ksNanoseconds refreshStartTime = nextSwapTime;
    const ksNanoseconds refreshEndTime = refreshStartTime /* + display refresh time for an incremental display refresh */;

    if (timeWarp->implementation == TIMEWARP_IMPLEMENTATION_GRAPHICS) {
        ksTimeWarpGraphics_Render(&timeWarp->commandBuffer, &timeWarp->graphics, &timeWarp->framebuffer, &timeWarp->renderPass,
                                  refreshStartTime, refreshEndTime, &timeWarp->projectionMatrix, &timeWarp->viewMatrix,
                                  timeWarp->eyeTexture, timeWarp->eyeArrayLayer, timeWarp->correctChromaticAberration,
                                  &timeWarp->bargraphs, timeWarp->cpuTimes, timeWarp->gpuTimes);
    } else if (timeWarp->implementation == TIMEWARP_IMPLEMENTATION_COMPUTE) {
        ksTimeWarpCompute_Render(&timeWarp->commandBuffer, &timeWarp->compute, &timeWarp->framebuffer, refreshStartTime,
                                 refreshEndTime, &timeWarp->projectionMatrix, &timeWarp->viewMatrix, timeWarp->eyeTexture,
                                 timeWarp->eyeArrayLayer, timeWarp->correctChromaticAberration, &timeWarp->bargraphs,
                                 timeWarp->cpuTimes, timeWarp->gpuTimes);
    }

    ksFrameLog_EndFrame(timeWarp->cpuTimes[PROFILE_TIME_TIME_WARP] + timeWarp->cpuTimes[PROFILE_TIME_BAR_GRAPHS] +
                            timeWarp->cpuTimes[PROFILE_TIME_BLIT],
                        timeWarp->gpuTimes[PROFILE_TIME_TIME_WARP] + timeWarp->gpuTimes[PROFILE_TIME_BAR_GRAPHS] +
                            timeWarp->gpuTimes[PROFILE_TIME_BLIT],
                        KS_GPU_TIMER_FRAMES_DELAYED);

    ksGpuWindow_SwapBuffers(timeWarp->window);

    ksSignal_Raise(&timeWarp->vsyncSignal);
}

#include "scenes/scene_settings.h"
#include "scenes/scene_view_state.h"
#include "scenes/scene_perf.h"
#include "scenes/scene_gltf.h"

/*
================================================================================================================================

Info

================================================================================================================================
*/

static void PrintInfo(const ksGpuWindow *window, const int eyeImageResolutionLevel, const int eyeImageSamplesLevel) {
    const int resolution = (eyeImageResolutionLevel >= 0) ? eyeResolutionTable[eyeImageResolutionLevel] : 0;
    const int samples = (eyeImageSamplesLevel >= 0) ? eyeSampleCountTable[eyeImageSamplesLevel] : 0;
    char resolutionString[32];
    sprintf(resolutionString, "%4d x %4d - %dx MSAA", resolution, resolution, samples);

    Print("--------------------------------\n");
    Print("OS      : %s\n", GetOSVersion());
    Print("CPU     : %s\n", GetCPUVersion());
    Print("GPU     : %s\n", glGetString(GL_RENDERER));
    Print("OpenGL  : %s\n", glGetString(GL_VERSION));
    Print("Display : %4d x %4d - %1.0f Hz (%s)\n", window->windowWidth, window->windowHeight, window->windowRefreshRate,
          window->windowFullscreen ? "fullscreen" : "windowed");
    Print("Eye Img : %s\n", (resolution >= 0) ? resolutionString : "-");
    Print("--------------------------------\n");
}

/*
================================================================================================================================

Dump GLSL

================================================================================================================================
*/

static void WriteTextFile(const char *path, const char *text) {
    FILE *fp = fopen(path, "wb");
    if (fp == NULL) {
        Print("Failed to write %s\n", path);
        return;
    }
    fwrite(text, strlen(text), 1, fp);
    fclose(fp);
    Print("Wrote %s\n", path);
}

typedef struct {
    const char *fileName;
    const char *extension;
    const char *glsl;
} glsl_t;

static void DumpGLSL() {
    glsl_t glsl[] = {
        {"barGraphVertexProgram", "vert", barGraphVertexProgramGLSL},
        {"barGraphFragmentProgram", "frag", barGraphFragmentProgramGLSL},
        {"timeWarpSpatialVertexProgram", "vert", timeWarpSpatialVertexProgramGLSL},
        {"timeWarpSpatialFragmentProgram", "frag", timeWarpSpatialFragmentProgramGLSL},
        {"timeWarpChromaticVertexProgram", "vert", timeWarpChromaticVertexProgramGLSL},
        {"timeWarpChromaticFragmentProgram", "frag", timeWarpChromaticFragmentProgramGLSL},
        {"flatShadedVertexProgram", "vert", flatShadedVertexProgramGLSL},
        {"flatShadedMultiViewVertexProgram", "vert", flatShadedMultiViewVertexProgramGLSL},
        {"flatShadedFragmentProgram", "frag", flatShadedFragmentProgramGLSL},
        {"normalMappedVertexProgram", "vert", normalMappedVertexProgramGLSL},
        {"normalMappedMultiViewVertexProgram", "vert", normalMappedMultiViewVertexProgramGLSL},
        {"normalMapped100LightsFragmentProgram", "frag", normalMapped100LightsFragmentProgramGLSL},
        {"normalMapped1000LightsFragmentProgram", "frag", normalMapped1000LightsFragmentProgramGLSL},
        {"normalMapped2000LightsFragmentProgram", "frag", normalMapped2000LightsFragmentProgramGLSL},

#if OPENGL_COMPUTE_ENABLED == 1
        {"barGraphComputeProgram", "comp", barGraphComputeProgramGLSL},
        {"timeWarpTransformComputeProgram", "comp", timeWarpTransformComputeProgramGLSL},
        {"timeWarpSpatialComputeProgram", "comp", timeWarpSpatialComputeProgramGLSL},
        {"timeWarpChromaticComputeProgram", "comp", timeWarpChromaticComputeProgramGLSL},
#endif
    };

    char path[1024];
    char batchFileBin[4096];
    char batchFileHex[4096];
    size_t batchFileBinLength = 0;
    size_t batchFileHexLength = 0;
    for (size_t i = 0; i < ARRAY_SIZE(glsl); i++) {
        sprintf(path, "glsl/%sGLSL.%s", glsl[i].fileName, glsl[i].extension);
        WriteTextFile(path, glsl[i].glsl);

        batchFileBinLength += sprintf(batchFileBin + batchFileBinLength, "glslangValidator -G -o %sSPIRV.spv %sGLSL.%s\r\n",
                                      glsl[i].fileName, glsl[i].fileName, glsl[i].extension);
        batchFileHexLength += sprintf(batchFileHex + batchFileHexLength, "glslangValidator -G -x -o %sSPIRV.h %sGLSL.%s\r\n",
                                      glsl[i].fileName, glsl[i].fileName, glsl[i].extension);
    }

    WriteTextFile("glsl/spirv_bin.bat", batchFileBin);
    WriteTextFile("glsl/spirv_hex.bat", batchFileHex);
}

/*
================================================================================================================================

Startup settings.

ksStartupSettings

static int ksStartupSettings_StringToLevel( const char * string, const int maxLevels );
static int ksStartupSettings_StringToRenderMode( const char * string );
static int ksStartupSettings_StringToTimeWarpImplementation( const char * string );

================================================================================================================================
*/

typedef enum { RENDER_MODE_ASYNC_TIME_WARP, RENDER_MODE_TIME_WARP, RENDER_MODE_SCENE, RENDER_MODE_MAX } ksRenderMode;

typedef struct {
    const char *glTF;
    bool fullscreen;
    bool simulationPaused;
    bool headRotationDisabled;
    int displayResolutionLevel;
    int eyeImageResolutionLevel;
    int eyeImageSamplesLevel;
    int drawCallLevel;
    int triangleLevel;
    int fragmentLevel;
    bool useMultiView;
    bool correctChromaticAberration;
    bool hideGraphs;
    ksTimeWarpImplementation timeWarpImplementation;
    ksRenderMode renderMode;
    ksNanoseconds startupTimeNanoseconds;
    ksNanoseconds noVSyncNanoseconds;
    ksNanoseconds noLogNanoseconds;
} ksStartupSettings;

static int ksStartupSettings_StringToLevel(const char *string, const int maxLevels) {
    const int level = atoi(string);
    return (level >= 0) ? ((level < maxLevels) ? level : maxLevels - 1) : 0;
}

static int ksStartupSettings_StringToRenderMode(const char *string) {
    return ((strcmp(string, "atw") == 0) ? RENDER_MODE_ASYNC_TIME_WARP
                                         : ((strcmp(string, "tw") == 0) ? RENDER_MODE_TIME_WARP : RENDER_MODE_SCENE));
}

static int ksStartupSettings_StringToTimeWarpImplementation(const char *string) {
    return ((strcmp(string, "graphics") == 0)
                ? TIMEWARP_IMPLEMENTATION_GRAPHICS
                : ((strcmp(string, "compute") == 0) ? TIMEWARP_IMPLEMENTATION_COMPUTE : TIMEWARP_IMPLEMENTATION_GRAPHICS));
}

/*
================================================================================================================================

Asynchronous time warp.

================================================================================================================================
*/

enum { QUEUE_INDEX_TIMEWARP = 0, QUEUE_INDEX_SCENE = 1 };

// Two should be enough but use three to make absolutely sure there are no stalls due to buffer locking.
#define NUM_EYE_BUFFERS 3

#if defined(OS_ANDROID)
#define WINDOW_RESOLUTION(x, fullscreen) (x)  // always fullscreen
#else
#define WINDOW_RESOLUTION(x, fullscreen) ((fullscreen) ? (x) : ROUNDUP(x / 2, 8))
#endif

typedef struct {
    ksSignal initialized;
    ksGpuContext *shareContext;
    ksTimeWarp *timeWarp;
    ksSceneSettings *sceneSettings;
    ksGpuWindowInput *input;

    volatile bool terminate;
    volatile bool openFrameLog;
} ksSceneThreadData;

void SceneThread_Render(ksSceneThreadData *threadData) {
    ksThread_SetAffinity(THREAD_AFFINITY_BIG_CORES);

    ksGpuContext context;
    ksGpuContext_CreateShared(&context, threadData->shareContext, QUEUE_INDEX_SCENE);
    ksGpuContext_SetCurrent(&context);

    const int resolution = eyeResolutionTable[threadData->sceneSettings->eyeImageResolutionLevel];

    const ksGpuSampleCount sampleCount = eyeSampleCountTable[threadData->sceneSettings->eyeImageSamplesLevel];

    ksGpuRenderPass renderPass;
    ksGpuRenderPass_Create(&context, &renderPass, KS_GPU_SURFACE_COLOR_FORMAT_R8G8B8A8, KS_GPU_SURFACE_DEPTH_FORMAT_D24,
                           sampleCount, KS_GPU_RENDERPASS_TYPE_INLINE,
                           KS_GPU_RENDERPASS_FLAG_CLEAR_COLOR_BUFFER | KS_GPU_RENDERPASS_FLAG_CLEAR_DEPTH_BUFFER);

    ksGpuFramebuffer framebuffer;
    ksGpuFramebuffer_CreateFromTextureArrays(&context, &framebuffer, &renderPass, resolution, resolution, NUM_EYES, NUM_EYE_BUFFERS,
                                             threadData->sceneSettings->useMultiView);

    const int numPasses = threadData->sceneSettings->useMultiView ? 1 : NUM_EYES;

    ksGpuCommandBuffer eyeCommandBuffer[NUM_EYES];
    ksGpuTimer eyeTimer[NUM_EYES];

    for (int eye = 0; eye < numPasses; eye++) {
        ksGpuCommandBuffer_Create(&context, &eyeCommandBuffer[eye], KS_GPU_COMMAND_BUFFER_TYPE_PRIMARY, NUM_EYE_BUFFERS);
        ksGpuTimer_Create(&context, &eyeTimer[eye]);
    }

    const ksBodyInfo *bodyInfo = GetDefaultBodyInfo();

    ksViewState viewState;
    ksViewState_Init(&viewState, bodyInfo->interpupillaryDistance);

    ksPerfScene perfScene;
    ksGltfScene gltfScene;

    if (threadData->sceneSettings->glTF == NULL) {
        ksPerfScene_Create(&context, &perfScene, threadData->sceneSettings, &renderPass);
    } else {
        ksGltfScene_CreateFromFile(&context, &gltfScene, threadData->sceneSettings, &renderPass);
    }

    ksSignal_Raise(&threadData->initialized);

    for (int frameIndex = 0; !threadData->terminate; frameIndex++) {
        if (threadData->openFrameLog) {
            threadData->openFrameLog = false;
            ksFrameLog_Open(OUTPUT_PATH "framelog_scene.txt", 10);
        }

        const ksNanoseconds nextDisplayTime = ksTimeWarp_GetPredictedDisplayTime(threadData->timeWarp, frameIndex);

        if (threadData->sceneSettings->glTF == NULL) {
            ksPerfScene_Simulate(&perfScene, &viewState, nextDisplayTime);
        } else {
            ksGltfScene_Simulate(&gltfScene, &viewState, threadData->input, nextDisplayTime);
        }

        ksFrameLog_BeginFrame();

        const ksNanoseconds t0 = GetTimeNanoseconds();

        ksGpuTexture *eyeTexture[NUM_EYES] = {0};
        ksGpuFence *eyeCompletionFence[NUM_EYES] = {0};
        int eyeArrayLayer[NUM_EYES] = {0, 1};

        for (int eye = 0; eye < numPasses; eye++) {
            const ksScreenRect screenRect = ksGpuFramebuffer_GetRect(&framebuffer);

            ksGpuCommandBuffer_BeginPrimary(&eyeCommandBuffer[eye]);
            ksGpuCommandBuffer_BeginFramebuffer(&eyeCommandBuffer[eye], &framebuffer, eye, KS_GPU_TEXTURE_USAGE_COLOR_ATTACHMENT);

            if (threadData->sceneSettings->glTF == NULL) {
                ksPerfScene_UpdateBuffers(&eyeCommandBuffer[eye], &perfScene, &viewState, (numPasses == 1) ? 2 : eye);
            } else {
                ksGltfScene_UpdateBuffers(&eyeCommandBuffer[eye], &gltfScene, &viewState, (numPasses == 1) ? 2 : eye);
            }

            ksGpuCommandBuffer_BeginTimer(&eyeCommandBuffer[eye], &eyeTimer[eye]);
            ksGpuCommandBuffer_BeginRenderPass(&eyeCommandBuffer[eye], &renderPass, &framebuffer, &screenRect);

            ksGpuCommandBuffer_SetViewport(&eyeCommandBuffer[eye], &screenRect);
            ksGpuCommandBuffer_SetScissor(&eyeCommandBuffer[eye], &screenRect);

            if (threadData->sceneSettings->glTF == NULL) {
                ksPerfScene_Render(&eyeCommandBuffer[eye], &perfScene, &viewState);
            } else {
                ksGltfScene_Render(&eyeCommandBuffer[eye], &gltfScene, &viewState);
            }

            ksGpuCommandBuffer_EndRenderPass(&eyeCommandBuffer[eye], &renderPass);
            ksGpuCommandBuffer_EndTimer(&eyeCommandBuffer[eye], &eyeTimer[eye]);

            ksGpuCommandBuffer_EndFramebuffer(&eyeCommandBuffer[eye], &framebuffer, eye, KS_GPU_TEXTURE_USAGE_SAMPLED);
            ksGpuCommandBuffer_EndPrimary(&eyeCommandBuffer[eye]);

            eyeTexture[eye] = ksGpuFramebuffer_GetColorTexture(&framebuffer);
            eyeCompletionFence[eye] = ksGpuCommandBuffer_SubmitPrimary(&eyeCommandBuffer[eye]);
        }

        if (threadData->sceneSettings->useMultiView) {
            eyeTexture[1] = eyeTexture[0];
            eyeCompletionFence[1] = eyeCompletionFence[0];
        }

        const ksNanoseconds t1 = GetTimeNanoseconds();

        const ksNanoseconds eyeTexturesCpuTime = t1 - t0;
        const ksNanoseconds eyeTexturesGpuTime = ksGpuTimer_GetNanoseconds(&eyeTimer[0]) + ksGpuTimer_GetNanoseconds(&eyeTimer[1]);

        ksFrameLog_EndFrame(eyeTexturesCpuTime, eyeTexturesGpuTime, KS_GPU_TIMER_FRAMES_DELAYED);

        ksMatrix4x4f projectionMatrix;
        ksMatrix4x4f_CreateProjectionFov(&projectionMatrix, 40.0f, 40.0f, 40.0f, 40.0f, DEFAULT_NEAR_Z, INFINITE_FAR_Z);

        ksTimeWarp_SubmitFrame(threadData->timeWarp, frameIndex, nextDisplayTime, &viewState.displayViewMatrix, &projectionMatrix,
                               eyeTexture, eyeCompletionFence, eyeArrayLayer, eyeTexturesCpuTime, eyeTexturesGpuTime);
    }

    if (threadData->sceneSettings->glTF == NULL) {
        ksPerfScene_Destroy(&context, &perfScene);
    } else {
        ksGltfScene_Destroy(&context, &gltfScene);
    }

    for (int eye = 0; eye < numPasses; eye++) {
        ksGpuTimer_Destroy(&context, &eyeTimer[eye]);
        ksGpuCommandBuffer_Destroy(&context, &eyeCommandBuffer[eye]);
    }

    ksGpuFramebuffer_Destroy(&context, &framebuffer);
    ksGpuRenderPass_Destroy(&context, &renderPass);
    ksGpuContext_Destroy(&context);
}

void SceneThread_Create(ksThread *sceneThread, ksSceneThreadData *sceneThreadData, ksGpuWindow *window, ksTimeWarp *timeWarp,
                        ksSceneSettings *sceneSettings) {
    ksSignal_Create(&sceneThreadData->initialized, true);
    sceneThreadData->shareContext = &window->context;
    sceneThreadData->timeWarp = timeWarp;
    sceneThreadData->sceneSettings = sceneSettings;
    sceneThreadData->input = &window->input;
    sceneThreadData->terminate = false;
    sceneThreadData->openFrameLog = false;

    // On MacOS context creation fails if the share context is current on another thread.
    ksGpuContext_UnsetCurrent(&window->context);

    ksThread_Create(sceneThread, "atw:scene", (ksThreadFunction)SceneThread_Render, sceneThreadData);
    ksThread_Signal(sceneThread);
    ksSignal_Wait(&sceneThreadData->initialized, SIGNAL_TIMEOUT_INFINITE);

    ksGpuContext_SetCurrent(&window->context);
}

void SceneThread_Destroy(ksThread *sceneThread, ksSceneThreadData *sceneThreadData) {
    sceneThreadData->terminate = true;
    // The following assumes the time warp thread is blocked when this function is called.
    ksSignal_Raise(&sceneThreadData->timeWarp->newEyeTexturesConsumed);
    ksSignal_Raise(&sceneThreadData->timeWarp->vsyncSignal);
    ksSignal_Destroy(&sceneThreadData->initialized);
    ksThread_Destroy(sceneThread);
}

bool RenderAsyncTimeWarp(ksStartupSettings *startupSettings) {
    ksThread_SetAffinity(THREAD_AFFINITY_BIG_CORES);
    ksThread_SetRealTimePriority(1);

    ksDriverInstance instance;
    ksDriverInstance_Create(&instance);

    const ksGpuQueueInfo queueInfo = {2,
                                      KS_GPU_QUEUE_PROPERTY_GRAPHICS | KS_GPU_QUEUE_PROPERTY_COMPUTE,
                                      {KS_GPU_QUEUE_PRIORITY_HIGH, KS_GPU_QUEUE_PRIORITY_MEDIUM}};

    ksGpuWindow window;
    ksGpuWindow_Create(
        &window, &instance, &queueInfo, QUEUE_INDEX_TIMEWARP, KS_GPU_SURFACE_COLOR_FORMAT_R8G8B8A8,
        KS_GPU_SURFACE_DEPTH_FORMAT_NONE, KS_GPU_SAMPLE_COUNT_1,
        WINDOW_RESOLUTION(displayResolutionTable[startupSettings->displayResolutionLevel * 2 + 0], startupSettings->fullscreen),
        WINDOW_RESOLUTION(displayResolutionTable[startupSettings->displayResolutionLevel * 2 + 1], startupSettings->fullscreen),
        startupSettings->fullscreen);

    int swapInterval = (startupSettings->noVSyncNanoseconds <= 0);
    ksGpuWindow_SwapInterval(&window, swapInterval);

    ksTimeWarp timeWarp;
    ksTimeWarp_Create(&timeWarp, &window);
    ksTimeWarp_SetBarGraphState(&timeWarp, startupSettings->hideGraphs ? BAR_GRAPH_HIDDEN : BAR_GRAPH_VISIBLE);
    ksTimeWarp_SetImplementation(&timeWarp, startupSettings->timeWarpImplementation);
    ksTimeWarp_SetChromaticAberrationCorrection(&timeWarp, startupSettings->correctChromaticAberration);
    ksTimeWarp_SetMultiView(&timeWarp, startupSettings->useMultiView);
    ksTimeWarp_SetDisplayResolutionLevel(&timeWarp, startupSettings->displayResolutionLevel);
    ksTimeWarp_SetEyeImageResolutionLevel(&timeWarp, startupSettings->eyeImageResolutionLevel);
    ksTimeWarp_SetEyeImageSamplesLevel(&timeWarp, startupSettings->eyeImageSamplesLevel);
    ksTimeWarp_SetDrawCallLevel(&timeWarp, startupSettings->drawCallLevel);
    ksTimeWarp_SetTriangleLevel(&timeWarp, startupSettings->triangleLevel);
    ksTimeWarp_SetFragmentLevel(&timeWarp, startupSettings->fragmentLevel);

    ksSceneSettings sceneSettings;
    ksSceneSettings_Init(&window.context, &sceneSettings);
    ksSceneSettings_SetGltf(&sceneSettings, startupSettings->glTF);
    ksSceneSettings_SetSimulationPaused(&sceneSettings, startupSettings->simulationPaused);
    ksSceneSettings_SetMultiView(&sceneSettings, startupSettings->useMultiView);
    ksSceneSettings_SetDisplayResolutionLevel(&sceneSettings, startupSettings->displayResolutionLevel);
    ksSceneSettings_SetEyeImageResolutionLevel(&sceneSettings, startupSettings->eyeImageResolutionLevel);
    ksSceneSettings_SetEyeImageSamplesLevel(&sceneSettings, startupSettings->eyeImageSamplesLevel);
    ksSceneSettings_SetDrawCallLevel(&sceneSettings, startupSettings->drawCallLevel);
    ksSceneSettings_SetTriangleLevel(&sceneSettings, startupSettings->triangleLevel);
    ksSceneSettings_SetFragmentLevel(&sceneSettings, startupSettings->fragmentLevel);

    ksThread sceneThread;
    ksSceneThreadData sceneThreadData;
    SceneThread_Create(&sceneThread, &sceneThreadData, &window, &timeWarp, &sceneSettings);

    hmd_headRotationDisabled = startupSettings->headRotationDisabled;

    ksNanoseconds startupTimeNanoseconds = startupSettings->startupTimeNanoseconds;
    ksNanoseconds noVSyncNanoseconds = startupSettings->noVSyncNanoseconds;
    ksNanoseconds noLogNanoseconds = startupSettings->noLogNanoseconds;

    ksThread_SetName("atw:timewarp");

    bool exit = false;
    while (!exit) {
        const ksNanoseconds time = GetTimeNanoseconds();

        const ksGpuWindowEvent handleEvent = ksGpuWindow_ProcessEvents(&window);
        if (handleEvent == KS_GPU_WINDOW_EVENT_ACTIVATED) {
            PrintInfo(&window, sceneSettings.eyeImageResolutionLevel, startupSettings->eyeImageSamplesLevel);
        } else if (handleEvent == KS_GPU_WINDOW_EVENT_EXIT) {
            exit = true;
            break;
        }

        if (ksGpuWindowInput_ConsumeKeyboardKey(&window.input, KEY_ESCAPE)) {
            ksGpuWindow_Exit(&window);
        }
        if (ksGpuWindowInput_ConsumeKeyboardKey(&window.input, KEY_Z)) {
            startupSettings->renderMode = (ksRenderMode)((startupSettings->renderMode + 1) % RENDER_MODE_MAX);
            break;
        }
        if (ksGpuWindowInput_ConsumeKeyboardKey(&window.input, KEY_F)) {
            startupSettings->fullscreen = !startupSettings->fullscreen;
            break;
        }
        if (ksGpuWindowInput_ConsumeKeyboardKey(&window.input, KEY_V) ||
            (noVSyncNanoseconds > 0 && time - startupTimeNanoseconds > noVSyncNanoseconds)) {
            swapInterval = !swapInterval;
            ksGpuWindow_SwapInterval(&window, swapInterval);
            noVSyncNanoseconds = 0;
        }
        if (ksGpuWindowInput_ConsumeKeyboardKey(&window.input, KEY_L) ||
            (noLogNanoseconds > 0 && time - startupTimeNanoseconds > noLogNanoseconds)) {
            ksFrameLog_Open(OUTPUT_PATH "framelog_timewarp.txt", 10);
            sceneThreadData.openFrameLog = true;
            noLogNanoseconds = 0;
        }
        if (ksGpuWindowInput_ConsumeKeyboardKey(&window.input, KEY_H)) {
            hmd_headRotationDisabled = !hmd_headRotationDisabled;
        }
        if (ksGpuWindowInput_ConsumeKeyboardKey(&window.input, KEY_P)) {
            ksSceneSettings_ToggleSimulationPaused(&sceneSettings);
        }
        if (ksGpuWindowInput_ConsumeKeyboardKey(&window.input, KEY_G)) {
            ksTimeWarp_CycleBarGraphState(&timeWarp);
        }
        if (ksGpuWindowInput_ConsumeKeyboardKey(&window.input, KEY_R)) {
            ksSceneSettings_CycleDisplayResolutionLevel(&sceneSettings);
            startupSettings->displayResolutionLevel = sceneSettings.displayResolutionLevel;
            break;
        }
        if (ksGpuWindowInput_ConsumeKeyboardKey(&window.input, KEY_B)) {
            ksSceneSettings_CycleEyeImageResolutionLevel(&sceneSettings);
            startupSettings->eyeImageResolutionLevel = sceneSettings.eyeImageResolutionLevel;
            break;
        }
        if (ksGpuWindowInput_ConsumeKeyboardKey(&window.input, KEY_S)) {
            ksSceneSettings_CycleEyeImageSamplesLevel(&sceneSettings);
            startupSettings->eyeImageSamplesLevel = sceneSettings.eyeImageSamplesLevel;
            break;
        }
        if (ksGpuWindowInput_ConsumeKeyboardKey(&window.input, KEY_Q)) {
            ksSceneSettings_CycleDrawCallLevel(&sceneSettings);
            ksTimeWarp_SetDrawCallLevel(&timeWarp, ksSceneSettings_GetDrawCallLevel(&sceneSettings));
        }
        if (ksGpuWindowInput_ConsumeKeyboardKey(&window.input, KEY_W)) {
            ksSceneSettings_CycleTriangleLevel(&sceneSettings);
            ksTimeWarp_SetTriangleLevel(&timeWarp, ksSceneSettings_GetTriangleLevel(&sceneSettings));
        }
        if (ksGpuWindowInput_ConsumeKeyboardKey(&window.input, KEY_E)) {
            ksSceneSettings_CycleFragmentLevel(&sceneSettings);
            ksTimeWarp_SetFragmentLevel(&timeWarp, ksSceneSettings_GetFragmentLevel(&sceneSettings));
        }
        if (ksGpuWindowInput_ConsumeKeyboardKey(&window.input, KEY_I)) {
            ksTimeWarp_CycleImplementation(&timeWarp);
        }
        if (ksGpuWindowInput_ConsumeKeyboardKey(&window.input, KEY_C)) {
            ksTimeWarp_ToggleChromaticAberrationCorrection(&timeWarp);
        }
        if (ksGpuWindowInput_ConsumeKeyboardKey(&window.input, KEY_M)) {
            if (glExtensions.multi_view) {
                ksSceneSettings_ToggleMultiView(&sceneSettings);
                break;
            }
        }
        if (ksGpuWindowInput_ConsumeKeyboardKey(&window.input, KEY_D)) {
            DumpGLSL();
        }

        if (window.windowActive) {
            ksTimeWarp_Render(&timeWarp);
        }
    }

    ksGpuContext_WaitIdle(&window.context);
    SceneThread_Destroy(&sceneThread, &sceneThreadData);
    ksTimeWarp_Destroy(&timeWarp, &window);
    ksGpuWindow_Destroy(&window);
    ksDriverInstance_Destroy(&instance);

    return exit;
}

/*
================================================================================================================================

Time warp rendering test.

================================================================================================================================
*/

bool RenderTimeWarp(ksStartupSettings *startupSettings) {
    ksThread_SetAffinity(THREAD_AFFINITY_BIG_CORES);

    ksDriverInstance instance;
    ksDriverInstance_Create(&instance);

    const ksGpuQueueInfo queueInfo = {
        1, KS_GPU_QUEUE_PROPERTY_GRAPHICS | KS_GPU_QUEUE_PROPERTY_COMPUTE, {KS_GPU_QUEUE_PRIORITY_MEDIUM}};

    ksGpuWindow window;
    ksGpuWindow_Create(
        &window, &instance, &queueInfo, 0, KS_GPU_SURFACE_COLOR_FORMAT_R8G8B8A8, KS_GPU_SURFACE_DEPTH_FORMAT_NONE,
        KS_GPU_SAMPLE_COUNT_1,
        WINDOW_RESOLUTION(displayResolutionTable[startupSettings->displayResolutionLevel * 2 + 0], startupSettings->fullscreen),
        WINDOW_RESOLUTION(displayResolutionTable[startupSettings->displayResolutionLevel * 2 + 1], startupSettings->fullscreen),
        startupSettings->fullscreen);

    int swapInterval = (startupSettings->noVSyncNanoseconds <= 0);
    ksGpuWindow_SwapInterval(&window, swapInterval);

    ksTimeWarp timeWarp;
    ksTimeWarp_Create(&timeWarp, &window);
    ksTimeWarp_SetBarGraphState(&timeWarp, startupSettings->hideGraphs ? BAR_GRAPH_HIDDEN : BAR_GRAPH_VISIBLE);
    ksTimeWarp_SetImplementation(&timeWarp, startupSettings->timeWarpImplementation);
    ksTimeWarp_SetChromaticAberrationCorrection(&timeWarp, startupSettings->correctChromaticAberration);
    ksTimeWarp_SetDisplayResolutionLevel(&timeWarp, startupSettings->displayResolutionLevel);

    hmd_headRotationDisabled = startupSettings->headRotationDisabled;

    ksNanoseconds startupTimeNanoseconds = startupSettings->startupTimeNanoseconds;
    ksNanoseconds noVSyncNanoseconds = startupSettings->noVSyncNanoseconds;
    ksNanoseconds noLogNanoseconds = startupSettings->noLogNanoseconds;

    ksThread_SetName("atw:timewarp");

    bool exit = false;
    while (!exit) {
        const ksNanoseconds time = GetTimeNanoseconds();

        const ksGpuWindowEvent handleEvent = ksGpuWindow_ProcessEvents(&window);
        if (handleEvent == KS_GPU_WINDOW_EVENT_ACTIVATED) {
            PrintInfo(&window, 0, 0);
        } else if (handleEvent == KS_GPU_WINDOW_EVENT_EXIT) {
            exit = true;
        }

        if (ksGpuWindowInput_ConsumeKeyboardKey(&window.input, KEY_ESCAPE)) {
            ksGpuWindow_Exit(&window);
        }
        if (ksGpuWindowInput_ConsumeKeyboardKey(&window.input, KEY_Z)) {
            startupSettings->renderMode = (ksRenderMode)((startupSettings->renderMode + 1) % RENDER_MODE_MAX);
            break;
        }
        if (ksGpuWindowInput_ConsumeKeyboardKey(&window.input, KEY_F)) {
            startupSettings->fullscreen = !startupSettings->fullscreen;
            break;
        }
        if (ksGpuWindowInput_ConsumeKeyboardKey(&window.input, KEY_V) ||
            (noVSyncNanoseconds > 0 && time - startupTimeNanoseconds > noVSyncNanoseconds)) {
            swapInterval = !swapInterval;
            ksGpuWindow_SwapInterval(&window, swapInterval);
            noVSyncNanoseconds = 0;
        }
        if (ksGpuWindowInput_ConsumeKeyboardKey(&window.input, KEY_L) ||
            (noLogNanoseconds > 0 && time - startupTimeNanoseconds > noLogNanoseconds)) {
            ksFrameLog_Open(OUTPUT_PATH "framelog_timewarp.txt", 10);
            noLogNanoseconds = 0;
        }
        if (ksGpuWindowInput_ConsumeKeyboardKey(&window.input, KEY_H)) {
            hmd_headRotationDisabled = !hmd_headRotationDisabled;
        }
        if (ksGpuWindowInput_ConsumeKeyboardKey(&window.input, KEY_G)) {
            ksTimeWarp_CycleBarGraphState(&timeWarp);
        }
        if (ksGpuWindowInput_ConsumeKeyboardKey(&window.input, KEY_I)) {
            ksTimeWarp_CycleImplementation(&timeWarp);
        }
        if (ksGpuWindowInput_ConsumeKeyboardKey(&window.input, KEY_C)) {
            ksTimeWarp_ToggleChromaticAberrationCorrection(&timeWarp);
        }
        if (ksGpuWindowInput_ConsumeKeyboardKey(&window.input, KEY_D)) {
            DumpGLSL();
        }

        if (window.windowActive) {
            ksTimeWarp_Render(&timeWarp);
        }
    }

    ksGpuContext_WaitIdle(&window.context);
    ksTimeWarp_Destroy(&timeWarp, &window);
    ksGpuWindow_Destroy(&window);
    ksDriverInstance_Destroy(&instance);

    return exit;
}

/*
================================================================================================================================

Scene rendering test.

================================================================================================================================
*/

bool RenderScene(ksStartupSettings *startupSettings) {
    ksThread_SetAffinity(THREAD_AFFINITY_BIG_CORES);

    ksDriverInstance instance;
    ksDriverInstance_Create(&instance);

    const ksGpuSampleCount sampleCountTable[] = {KS_GPU_SAMPLE_COUNT_1, KS_GPU_SAMPLE_COUNT_2, KS_GPU_SAMPLE_COUNT_4,
                                                 KS_GPU_SAMPLE_COUNT_8};
    const ksGpuSampleCount sampleCount = sampleCountTable[startupSettings->eyeImageSamplesLevel];

    const ksGpuQueueInfo queueInfo = {1, KS_GPU_QUEUE_PROPERTY_GRAPHICS, {KS_GPU_QUEUE_PRIORITY_MEDIUM}};

    ksGpuWindow window;
    ksGpuWindow_Create(
        &window, &instance, &queueInfo, 0, KS_GPU_SURFACE_COLOR_FORMAT_R8G8B8A8, KS_GPU_SURFACE_DEPTH_FORMAT_D24, sampleCount,
        WINDOW_RESOLUTION(displayResolutionTable[startupSettings->displayResolutionLevel * 2 + 0], startupSettings->fullscreen),
        WINDOW_RESOLUTION(displayResolutionTable[startupSettings->displayResolutionLevel * 2 + 1], startupSettings->fullscreen),
        startupSettings->fullscreen);

    int swapInterval = (startupSettings->noVSyncNanoseconds <= 0);
    ksGpuWindow_SwapInterval(&window, swapInterval);

    ksGpuRenderPass renderPass;
    ksGpuRenderPass_Create(&window.context, &renderPass, window.colorFormat, window.depthFormat, sampleCount,
                           KS_GPU_RENDERPASS_TYPE_INLINE,
                           KS_GPU_RENDERPASS_FLAG_CLEAR_COLOR_BUFFER | KS_GPU_RENDERPASS_FLAG_CLEAR_DEPTH_BUFFER);

    ksGpuFramebuffer framebuffer;
    ksGpuFramebuffer_CreateFromSwapchain(&window, &framebuffer, &renderPass);

    ksGpuCommandBuffer commandBuffer;
    ksGpuCommandBuffer_Create(&window.context, &commandBuffer, KS_GPU_COMMAND_BUFFER_TYPE_PRIMARY,
                              ksGpuFramebuffer_GetBufferCount(&framebuffer));

    ksGpuTimer timer;
    ksGpuTimer_Create(&window.context, &timer);

    ksBarGraph frameCpuTimeBarGraph;
    ksBarGraph_CreateVirtualRect(&window.context, &frameCpuTimeBarGraph, &renderPass, &frameCpuTimeBarGraphRect, 64, 1,
                                 &ksColorDarkGrey);

    ksBarGraph frameGpuTimeBarGraph;
    ksBarGraph_CreateVirtualRect(&window.context, &frameGpuTimeBarGraph, &renderPass, &frameGpuTimeBarGraphRect, 64, 1,
                                 &ksColorDarkGrey);

    ksSceneSettings sceneSettings;
    ksSceneSettings_Init(&window.context, &sceneSettings);
    ksSceneSettings_SetGltf(&sceneSettings, startupSettings->glTF);
    ksSceneSettings_SetSimulationPaused(&sceneSettings, startupSettings->simulationPaused);
    ksSceneSettings_SetDisplayResolutionLevel(&sceneSettings, startupSettings->displayResolutionLevel);
    ksSceneSettings_SetEyeImageResolutionLevel(&sceneSettings, startupSettings->eyeImageResolutionLevel);
    ksSceneSettings_SetEyeImageSamplesLevel(&sceneSettings, startupSettings->eyeImageSamplesLevel);
    ksSceneSettings_SetDrawCallLevel(&sceneSettings, startupSettings->drawCallLevel);
    ksSceneSettings_SetTriangleLevel(&sceneSettings, startupSettings->triangleLevel);
    ksSceneSettings_SetFragmentLevel(&sceneSettings, startupSettings->fragmentLevel);

    ksViewState viewState;
    ksViewState_Init(&viewState, 0.0f);

    ksPerfScene perfScene;
    ksGltfScene gltfScene;

    if (startupSettings->glTF == NULL) {
        ksPerfScene_Create(&window.context, &perfScene, &sceneSettings, &renderPass);
    } else {
        ksGltfScene_CreateFromFile(&window.context, &gltfScene, &sceneSettings, &renderPass);
    }

    hmd_headRotationDisabled = startupSettings->headRotationDisabled;

    ksNanoseconds startupTimeNanoseconds = startupSettings->startupTimeNanoseconds;
    ksNanoseconds noVSyncNanoseconds = startupSettings->noVSyncNanoseconds;
    ksNanoseconds noLogNanoseconds = startupSettings->noLogNanoseconds;

    ksThread_SetName("atw:scene");

    bool exit = false;
    while (!exit) {
        const ksNanoseconds time = GetTimeNanoseconds();

        const ksGpuWindowEvent handleEvent = ksGpuWindow_ProcessEvents(&window);
        if (handleEvent == KS_GPU_WINDOW_EVENT_ACTIVATED) {
            PrintInfo(&window, -1, -1);
        } else if (handleEvent == KS_GPU_WINDOW_EVENT_EXIT) {
            exit = true;
            break;
        }

        if (ksGpuWindowInput_ConsumeKeyboardKey(&window.input, KEY_ESCAPE)) {
            ksGpuWindow_Exit(&window);
        }
        if (ksGpuWindowInput_ConsumeKeyboardKey(&window.input, KEY_Z)) {
            startupSettings->renderMode = (ksRenderMode)((startupSettings->renderMode + 1) % RENDER_MODE_MAX);
            break;
        }
        if (ksGpuWindowInput_ConsumeKeyboardKey(&window.input, KEY_F)) {
            startupSettings->fullscreen = !startupSettings->fullscreen;
            break;
        }
        if (ksGpuWindowInput_ConsumeKeyboardKey(&window.input, KEY_V) ||
            (noVSyncNanoseconds > 0 && time - startupTimeNanoseconds > noVSyncNanoseconds)) {
            swapInterval = !swapInterval;
            ksGpuWindow_SwapInterval(&window, swapInterval);
            noVSyncNanoseconds = 0;
        }
        if (ksGpuWindowInput_ConsumeKeyboardKey(&window.input, KEY_L) ||
            (noLogNanoseconds > 0 && time - startupTimeNanoseconds > noLogNanoseconds)) {
            ksFrameLog_Open(OUTPUT_PATH "framelog_scene.txt", 10);
            noLogNanoseconds = 0;
        }
        if (ksGpuWindowInput_ConsumeKeyboardKey(&window.input, KEY_H)) {
            hmd_headRotationDisabled = !hmd_headRotationDisabled;
        }
        if (ksGpuWindowInput_ConsumeKeyboardKey(&window.input, KEY_P)) {
            ksSceneSettings_ToggleSimulationPaused(&sceneSettings);
        }
        if (ksGpuWindowInput_ConsumeKeyboardKey(&window.input, KEY_R)) {
            ksSceneSettings_CycleDisplayResolutionLevel(&sceneSettings);
            startupSettings->displayResolutionLevel = sceneSettings.displayResolutionLevel;
            break;
        }
        if (ksGpuWindowInput_ConsumeKeyboardKey(&window.input, KEY_B)) {
            ksSceneSettings_CycleEyeImageResolutionLevel(&sceneSettings);
            startupSettings->eyeImageResolutionLevel = sceneSettings.eyeImageResolutionLevel;
            break;
        }
        if (ksGpuWindowInput_ConsumeKeyboardKey(&window.input, KEY_S)) {
            ksSceneSettings_CycleEyeImageSamplesLevel(&sceneSettings);
            startupSettings->eyeImageSamplesLevel = sceneSettings.eyeImageSamplesLevel;
            break;
        }
        if (ksGpuWindowInput_ConsumeKeyboardKey(&window.input, KEY_Q)) {
            ksSceneSettings_CycleDrawCallLevel(&sceneSettings);
        }
        if (ksGpuWindowInput_ConsumeKeyboardKey(&window.input, KEY_W)) {
            ksSceneSettings_CycleTriangleLevel(&sceneSettings);
        }
        if (ksGpuWindowInput_ConsumeKeyboardKey(&window.input, KEY_E)) {
            ksSceneSettings_CycleFragmentLevel(&sceneSettings);
        }
        if (ksGpuWindowInput_ConsumeKeyboardKey(&window.input, KEY_D)) {
            DumpGLSL();
        }

        if (window.windowActive) {
            const ksNanoseconds nextSwapTime = ksGpuWindow_GetNextSwapTimeNanoseconds(&window);

            if (startupSettings->glTF == NULL) {
                ksPerfScene_Simulate(&perfScene, &viewState, nextSwapTime);
            } else {
                ksGltfScene_Simulate(&gltfScene, &viewState, &window.input, nextSwapTime);
            }

            ksFrameLog_BeginFrame();

            const ksNanoseconds t0 = GetTimeNanoseconds();

            const ksScreenRect screenRect = ksGpuFramebuffer_GetRect(&framebuffer);

            ksGpuCommandBuffer_BeginPrimary(&commandBuffer);
            ksGpuCommandBuffer_BeginFramebuffer(&commandBuffer, &framebuffer, 0, KS_GPU_TEXTURE_USAGE_COLOR_ATTACHMENT);

            if (startupSettings->glTF == NULL) {
                ksPerfScene_UpdateBuffers(&commandBuffer, &perfScene, &viewState, 0);
            } else {
                ksGltfScene_UpdateBuffers(&commandBuffer, &gltfScene, &viewState, 0);
            }

            ksBarGraph_UpdateGraphics(&commandBuffer, &frameCpuTimeBarGraph);
            ksBarGraph_UpdateGraphics(&commandBuffer, &frameGpuTimeBarGraph);

            ksGpuCommandBuffer_BeginTimer(&commandBuffer, &timer);
            ksGpuCommandBuffer_BeginRenderPass(&commandBuffer, &renderPass, &framebuffer, &screenRect);

            ksGpuCommandBuffer_SetViewport(&commandBuffer, &screenRect);
            ksGpuCommandBuffer_SetScissor(&commandBuffer, &screenRect);

            if (startupSettings->glTF == NULL) {
                ksPerfScene_Render(&commandBuffer, &perfScene, &viewState);
            } else {
                ksGltfScene_Render(&commandBuffer, &gltfScene, &viewState);
            }

            ksBarGraph_RenderGraphics(&commandBuffer, &frameCpuTimeBarGraph);
            ksBarGraph_RenderGraphics(&commandBuffer, &frameGpuTimeBarGraph);

            ksGpuCommandBuffer_EndRenderPass(&commandBuffer, &renderPass);
            ksGpuCommandBuffer_EndTimer(&commandBuffer, &timer);

            ksGpuCommandBuffer_EndFramebuffer(&commandBuffer, &framebuffer, 0, KS_GPU_TEXTURE_USAGE_PRESENTATION);
            ksGpuCommandBuffer_EndPrimary(&commandBuffer);

            ksGpuCommandBuffer_SubmitPrimary(&commandBuffer);

            const ksNanoseconds t1 = GetTimeNanoseconds();

            const ksNanoseconds sceneCpuTime = t1 - t0;
            const ksNanoseconds sceneGpuTime = ksGpuTimer_GetNanoseconds(&timer);

            ksFrameLog_EndFrame(sceneCpuTime, sceneGpuTime, KS_GPU_TIMER_FRAMES_DELAYED);

            ksBarGraph_AddBar(&frameCpuTimeBarGraph, 0, sceneCpuTime * window.windowRefreshRate * 1e-9f, &ksColorGreen, true);
            ksBarGraph_AddBar(&frameGpuTimeBarGraph, 0, sceneGpuTime * window.windowRefreshRate * 1e-9f, &ksColorGreen, true);

            ksGpuWindow_SwapBuffers(&window);
        }
    }

    if (startupSettings->glTF == NULL) {
        ksPerfScene_Destroy(&window.context, &perfScene);
    } else {
        ksGltfScene_Destroy(&window.context, &gltfScene);
    }

    ksBarGraph_Destroy(&window.context, &frameGpuTimeBarGraph);
    ksBarGraph_Destroy(&window.context, &frameCpuTimeBarGraph);
    ksGpuTimer_Destroy(&window.context, &timer);
    ksGpuCommandBuffer_Destroy(&window.context, &commandBuffer);
    ksGpuFramebuffer_Destroy(&window.context, &framebuffer);
    ksGpuRenderPass_Destroy(&window.context, &renderPass);
    ksGpuWindow_Destroy(&window);
    ksDriverInstance_Destroy(&instance);

    return exit;
}

/*
================================================================================================================================

Startup

================================================================================================================================
*/

static int StartApplication(int argc, char *argv[]) {
    ksStartupSettings startupSettings;
    memset(&startupSettings, 0, sizeof(startupSettings));
    startupSettings.startupTimeNanoseconds = GetTimeNanoseconds();

    for (int i = 1; i < argc; i++) {
        const char *arg = argv[i];
        if (arg[0] == '-') {
            arg++;
        }

        if (strcmp(arg, "a") == 0 && i + 0 < argc) {
            startupSettings.glTF = argv[++i];
        } else if (strcmp(arg, "f") == 0 && i + 0 < argc) {
            startupSettings.fullscreen = true;
        } else if (strcmp(arg, "v") == 0 && i + 1 < argc) {
            startupSettings.noVSyncNanoseconds = (ksNanoseconds)(atof(argv[++i]) * 1000 * 1000 * 1000);
        } else if (strcmp(arg, "h") == 0 && i + 0 < argc) {
            startupSettings.headRotationDisabled = true;
        } else if (strcmp(arg, "p") == 0 && i + 0 < argc) {
            startupSettings.simulationPaused = true;
        } else if (strcmp(arg, "r") == 0 && i + 1 < argc) {
            startupSettings.displayResolutionLevel = ksStartupSettings_StringToLevel(argv[++i], MAX_DISPLAY_RESOLUTION_LEVELS);
        } else if (strcmp(arg, "b") == 0 && i + 1 < argc) {
            startupSettings.eyeImageResolutionLevel = ksStartupSettings_StringToLevel(argv[++i], MAX_EYE_IMAGE_RESOLUTION_LEVELS);
        } else if (strcmp(arg, "s") == 0 && i + 1 < argc) {
            startupSettings.eyeImageSamplesLevel = ksStartupSettings_StringToLevel(argv[++i], MAX_EYE_IMAGE_SAMPLES_LEVELS);
        } else if (strcmp(arg, "q") == 0 && i + 1 < argc) {
            startupSettings.drawCallLevel = ksStartupSettings_StringToLevel(argv[++i], MAX_SCENE_DRAWCALL_LEVELS);
        } else if (strcmp(arg, "w") == 0 && i + 1 < argc) {
            startupSettings.triangleLevel = ksStartupSettings_StringToLevel(argv[++i], MAX_SCENE_TRIANGLE_LEVELS);
        } else if (strcmp(arg, "e") == 0 && i + 1 < argc) {
            startupSettings.fragmentLevel = ksStartupSettings_StringToLevel(argv[++i], MAX_SCENE_FRAGMENT_LEVELS);
        } else if (strcmp(arg, "m") == 0 && i + 0 < argc) {
            startupSettings.useMultiView = (atoi(argv[++i]) != 0);
        } else if (strcmp(arg, "c") == 0 && i + 1 < argc) {
            startupSettings.correctChromaticAberration = (atoi(argv[++i]) != 0);
        } else if (strcmp(arg, "i") == 0 && i + 1 < argc) {
            startupSettings.timeWarpImplementation =
                (ksTimeWarpImplementation)ksStartupSettings_StringToTimeWarpImplementation(argv[++i]);
        } else if (strcmp(arg, "z") == 0 && i + 1 < argc) {
            startupSettings.renderMode = ksStartupSettings_StringToRenderMode(argv[++i]);
        } else if (strcmp(arg, "g") == 0 && i + 0 < argc) {
            startupSettings.hideGraphs = true;
        } else if (strcmp(arg, "l") == 0 && i + 1 < argc) {
            startupSettings.noLogNanoseconds = (ksNanoseconds)(atof(argv[++i]) * 1000 * 1000 * 1000);
        } else if (strcmp(arg, "d") == 0 && i + 0 < argc) {
            DumpGLSL();
            exit(0);
        } else {
            Print(
                "Unknown option: %s\n"
                "atw_opengl [options]\n"
                "options:\n"
                "   -a <file>   load glTF scene\n"
                "   -f          start fullscreen\n"
                "   -v <s>      start with V-Sync disabled for this many seconds\n"
                "   -h          start with head rotation disabled\n"
                "   -p          start with the simulation paused\n"
                "   -r <0-3>    set display resolution level\n"
                "   -b <0-3>    set eye image resolution level\n"
                "   -s <0-3>    set multi-sampling level\n"
                "   -q <0-3>    set per eye draw calls level\n"
                "   -w <0-3>    set per eye triangles per draw call level\n"
                "   -e <0-3>    set per eye fragment program complexity level\n"
                "   -m <0-1>    enable/disable multi-view\n"
                "   -c <0-1>    enable/disable correction for chromatic aberration\n"
                "   -i <name>   set time warp implementation: graphics, compute\n"
                "   -z <name>   set the render mode: atw, tw, scene\n"
                "   -g          hide graphs\n"
                "   -l <s>      log 10 frames of OpenGL commands after this many seconds\n"
                "   -d          dump GLSL to files for conversion to SPIR-V\n",
                arg);
            return 1;
        }
    }

    // startupSettings.glTF = "models.json";
    // startupSettings.headRotationDisabled = true;
    // startupSettings.simulationPaused = true;
    // startupSettings.eyeImageSamplesLevel = 0;
    // startupSettings.useMultiView = true;
    // startupSettings.correctChromaticAberration = true;
    // startupSettings.renderMode = RENDER_MODE_SCENE;
    // startupSettings.timeWarpImplementation = TIMEWARP_IMPLEMENTATION_COMPUTE;

    Print("    fullscreen = %d\n", startupSettings.fullscreen);
    Print("    noVSyncNanoseconds = %lld\n", startupSettings.noVSyncNanoseconds);
    Print("    headRotationDisabled = %d\n", startupSettings.headRotationDisabled);
    Print("    simulationPaused = %d\n", startupSettings.simulationPaused);
    Print("    displayResolutionLevel = %d\n", startupSettings.displayResolutionLevel);
    Print("    eyeImageResolutionLevel = %d\n", startupSettings.eyeImageResolutionLevel);
    Print("    eyeImageSamplesLevel = %d\n", startupSettings.eyeImageSamplesLevel);
    Print("    drawCallLevel = %d\n", startupSettings.drawCallLevel);
    Print("    triangleLevel = %d\n", startupSettings.triangleLevel);
    Print("    fragmentLevel = %d\n", startupSettings.fragmentLevel);
    Print("    useMultiView = %d\n", startupSettings.useMultiView);
    Print("    correctChromaticAberration = %d\n", startupSettings.correctChromaticAberration);
    Print("    timeWarpImplementation = %d\n", startupSettings.timeWarpImplementation);
    Print("    renderMode = %d\n", startupSettings.renderMode);
    Print("    hideGraphs = %d\n", startupSettings.hideGraphs);
    Print("    noLogNanoseconds = %lld\n", startupSettings.noLogNanoseconds);

    for (bool exit = false; !exit;) {
        if (startupSettings.renderMode == RENDER_MODE_ASYNC_TIME_WARP) {
            exit = RenderAsyncTimeWarp(&startupSettings);
        } else if (startupSettings.renderMode == RENDER_MODE_TIME_WARP) {
            exit = RenderTimeWarp(&startupSettings);
        } else if (startupSettings.renderMode == RENDER_MODE_SCENE) {
            exit = RenderScene(&startupSettings);
        }
    }

    return 0;
}

#if defined(OS_WINDOWS)

int APIENTRY WinMain(HINSTANCE hCurrentInst, HINSTANCE hPreviousInst, LPSTR lpszCmdLine, int nCmdShow) {
    UNUSED_PARM(hCurrentInst);
    UNUSED_PARM(hPreviousInst);
    UNUSED_PARM(nCmdShow);

    int argc = 0;
    char *argv[32];

    char filename[_MAX_PATH];
    GetModuleFileNameA(NULL, filename, _MAX_PATH);
    argv[argc++] = filename;

    while (argc < 32) {
        while (lpszCmdLine[0] != '\0' && lpszCmdLine[0] == ' ') {
            lpszCmdLine++;
        }
        if (lpszCmdLine[0] == '\0') {
            break;
        }

        argv[argc++] = lpszCmdLine;

        while (lpszCmdLine[0] != '\0' && lpszCmdLine[0] != ' ') {
            lpszCmdLine++;
        }
        if (lpszCmdLine[0] == '\0') {
            break;
        }

        *lpszCmdLine++ = '\0';
    }

    return StartApplication(argc, argv);
}

#elif defined(OS_LINUX)

int main(int argc, char *argv[]) { return StartApplication(argc, argv); }

#elif defined(OS_APPLE_MACOS)

static const char *FormatString(const char *format, ...) {
    static int index = 0;
    static char buffer[2][4096];
    index ^= 1;
    va_list args;
    va_start(args, format);
    vsnprintf(buffer[index], sizeof(buffer[index]), format, args);
    va_end(args);
    return buffer[index];
}

void SystemCommandVerbose(const char *command) {
    int result = system(command);
    printf("%s : %s\n", command, (result == 0) ? "\e[0;32msuccessful\e[0m" : "\e[0;31mfailed\e[0m");
}

void WriteTextFileVerbose(const char *fileName, const char *text) {
    FILE *file = fopen(fileName, "wb");
    int elem = 0;
    if (file != NULL) {
        elem = fwrite(text, strlen(text), 1, file);
        fclose(file);
    }
    printf("write %s %s\n", fileName, (elem == 1) ? "\e[0;32msuccessful\e[0m" : "\e[0;31mfailed\e[0m");
}

void CreateBundle(const char *exePath) {
    const char *bundleIdentifier = "ATW";
    const char *bundleName = "ATW";
    const char *bundleSignature = "atwx";

    const char *infoPlistText =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<plist version=\"1.0\">\n"
        "<dict>\n"
        "    <key>BuildMachineOSBuild</key>\n"
        "    <string>13F34</string>\n"
        "    <key>CFBundleDevelopmentRegion</key>\n"
        "    <string>en</string>\n"
        "    <key>CFBundleExecutable</key>\n"
        "    <string>%s</string>\n"  // %s for executable name
        "    <key>CFBundleIdentifier</key>\n"
        "    <string>%s</string>\n"  // %s for bundleIdentifier
        "    <key>CFBundleInfoDictionaryVersion</key>\n"
        "    <string>6.0</string>\n"
        "    <key>CFBundleName</key>\n"
        "    <string>%s</string>\n"  // %s for bundleName
        "    <key>CFBundlePackageType</key>\n"
        "    <string>APPL</string>\n"
        "    <key>CFBundleShortVersionString</key>\n"
        "    <string>1.0</string>\n"
        "    <key>CFBundleSignature</key>\n"
        "    <string>atwx</string>\n"  // %s for bundleSignature
        "    <key>CFBundleVersion</key>\n"
        "    <string>1</string>\n"
        "    <key>DTCompiler</key>\n"
        "    <string>com.apple.compilers.llvm.clang.1_0</string>\n"
        "    <key>DTPlatformBuild</key>\n"
        "    <string>6A2008a</string>\n"
        "    <key>DTPlatformVersion</key>\n"
        "    <string>GM</string>\n"
        "    <key>DTSDKBuild</key>\n"
        "    <string>14A382</string>\n"
        "    <key>DTSDKName</key>\n"
        "    <string>macosx10.10</string>\n"
        "    <key>DTXcode</key>\n"
        "    <string>0611</string>\n"
        "    <key>DTXcodeBuild</key>\n"
        "    <string>6A2008a</string>\n"
        "    <key>LSMinimumSystemVersion</key>\n"
        "    <string>10.9</string>\n"
        "    <key>NSMainNibFile</key>\n"
        "    <string>MainMenu</string>\n"
        "    <key>NSPrincipalClass</key>\n"
        "    <string>NSApplication</string>\n"
        "</dict>\n"
        "</plist>\n";

    const char *exeName = exePath + strlen(exePath) - 1;
    for (; exeName > exePath && exeName[-1] != '/'; exeName--) {
    }

    SystemCommandVerbose(FormatString("rm -r %s.app", exePath));
    SystemCommandVerbose(FormatString("mkdir %s.app", exePath));
    SystemCommandVerbose(FormatString("mkdir %s.app/Contents", exePath));
    SystemCommandVerbose(FormatString("mkdir %s.app/Contents/MacOS", exePath));
    SystemCommandVerbose(FormatString("cp %s %s.app/Contents/MacOS", exePath, exePath));
    WriteTextFileVerbose(FormatString("%s.app/Contents/Info.plist", exePath),
                         FormatString(infoPlistText, exeName, bundleIdentifier, bundleName, bundleSignature));
}

void LaunchBundle(int argc, char *argv[]) {
    // Print command to open the bundled application.
    char command[2048];
    int length = snprintf(command, sizeof(command), "open %s.app", argv[0]);

    // Append all the original command-line arguments.
    const char *argsParm = " --args";
    const int argsParmLength = strlen(argsParm);
    if (argc > 1 && argsParmLength + 1 < sizeof(command) - length) {
        strcpy(command + length, argsParm);
        length += argsParmLength;
        for (int i = 1; i < argc; i++) {
            const int argLength = strlen(argv[i]);
            if (argLength + 2 > sizeof(command) - length) {
                break;
            }
            strcpy(command + length + 0, " ");
            strcpy(command + length + 1, argv[i]);
            length += 1 + argLength;
        }
    }

    // Launch the bundled application with the original command-line arguments.
    SystemCommandVerbose(command);
}

void SetBundleCWD(const char *bundledExecutablePath) {
    // Inside a bundle, an executable lives three folders and
    // four forward slashes deep: /name.app/Contents/MacOS/name
    char cwd[1024];
    strcpy(cwd, bundledExecutablePath);
    for (int i = strlen(cwd) - 1, slashes = 0; i >= 0 && slashes < 4; i--) {
        slashes += (cwd[i] == '/');
        cwd[i] = '\0';
    }
    int result = chdir(cwd);
    Print("chdir( \"%s\" ) %s", cwd, (result == 0) ? "successful" : "failed");
}

int main(int argc, char *argv[]) {
    /*
            When an application executable is not launched from a bundle, macOS
            considers the application to be a console application with only text output
            and console keyboard input. As a result, an application will not receive
            keyboard events unless the application is launched from a bundle.
            Programmatically created graphics windows are also unable to properly
            acquire focus unless the application is launched from a bundle.

            If the executable was not launched from a bundle then automatically create
            a bundle right here and then launch the bundled application.
    */
    if (strstr(argv[0], "/Contents/MacOS/") == NULL) {
        CreateBundle(argv[0]);
        LaunchBundle(argc, argv);
        return 0;
    }

    SetBundleCWD(argv[0]);

    autoReleasePool = [[NSAutoreleasePool alloc] init];

    [NSApplication sharedApplication];
    [NSApp finishLaunching];
    [NSApp activateIgnoringOtherApps:YES];

    return StartApplication(argc, argv);
}

#elif defined(OS_APPLE_IOS)

static int argc_deferred;
static char **argv_deferred;

@interface MyAppDelegate : NSObject <UIApplicationDelegate> {
}
@end

@implementation MyAppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    CGRect screenRect = UIScreen.mainScreen.bounds;
    myUIView = [[MyUIView alloc] initWithFrame:screenRect];
    UIViewController *myUIVC = [[[MyUIViewController alloc] init] autorelease];
    myUIVC.view = myUIView;

    myUIWindow = [[UIWindow alloc] initWithFrame:screenRect];
    [myUIWindow addSubview:myUIView];
    myUIWindow.rootViewController = myUIVC;
    [myUIWindow makeKeyAndVisible];

    // Delay to allow startup runloop to complete.
    [self performSelector:@selector(startApplication:) withObject:nil afterDelay:0.25f];

    return YES;
}

- (void)startApplication:(id)argObj {
    autoReleasePool = [[NSAutoreleasePool alloc] init];
    StartApplication(argc_deferred, argv_deferred);
}

@end

int main(int argc, char *argv[]) {
    argc_deferred = argc;
    argv_deferred = argv;

    return UIApplicationMain(argc, argv, nil, @"MyAppDelegate");
}

#elif defined(OS_ANDROID)

#define MAX_ARGS 32
#define MAX_ARGS_BUFFER 1024

typedef struct {
    char buffer[MAX_ARGS_BUFFER];
    char *argv[MAX_ARGS];
    int argc;
} ksAndroidParm;

// adb shell am start -n com.vulkansamples.atw_opengl/android.app.NativeActivity -a "android.intent.action.MAIN" --es "args" "\"-r
// tw\""
void GetIntentParms(ksAndroidParm *parms) {
    parms->buffer[0] = '\0';
    parms->argv[0] = "atw_vulkan";
    parms->argc = 1;

    Java_t java;
    java.vm = global_app->activity->vm;
    (*java.vm)->AttachCurrentThread(java.vm, &java.env, NULL);
    java.activity = global_app->activity->clazz;

    jclass activityClass = (*java.env)->GetObjectClass(java.env, java.activity);
    jmethodID getIntenMethodId = (*java.env)->GetMethodID(java.env, activityClass, "getIntent", "()Landroid/content/Intent;");
    jobject intent = (*java.env)->CallObjectMethod(java.env, java.activity, getIntenMethodId);
    (*java.env)->DeleteLocalRef(java.env, activityClass);

    jclass intentClass = (*java.env)->GetObjectClass(java.env, intent);
    jmethodID getStringExtraMethodId =
        (*java.env)->GetMethodID(java.env, intentClass, "getStringExtra", "(Ljava/lang/String;)Ljava/lang/String;");

    jstring argsJstring = (*java.env)->NewStringUTF(java.env, "args");
    jstring extraJstring = (*java.env)->CallObjectMethod(java.env, intent, getStringExtraMethodId, argsJstring);
    if (extraJstring != NULL) {
        const char *args = (*java.env)->GetStringUTFChars(java.env, extraJstring, 0);
        strncpy(parms->buffer, args, sizeof(parms->buffer) - 1);
        parms->buffer[sizeof(parms->buffer) - 1] = '\0';
        (*java.env)->ReleaseStringUTFChars(java.env, extraJstring, args);

        Print("    args = %s\n", args);

        char *ptr = parms->buffer;
        while (parms->argc < MAX_ARGS) {
            while (ptr[0] != '\0' && ptr[0] == ' ') {
                ptr++;
            }
            if (ptr[0] == '\0') {
                break;
            }

            parms->argv[parms->argc++] = ptr;

            while (ptr[0] != '\0' && ptr[0] != ' ') {
                ptr++;
            }
            if (ptr[0] == '\0') {
                break;
            }

            *ptr++ = '\0';
        }
    }

    (*java.env)->DeleteLocalRef(java.env, argsJstring);
    (*java.env)->DeleteLocalRef(java.env, intentClass);
    (*java.vm)->DetachCurrentThread(java.vm);
}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events.
 */
void android_main(struct android_app *app) {
    Print("----------------------------------------------------------------");
    Print("onCreate()");
    Print("    android_main()");

    // Make sure the native app glue is not stripped.
    app_dummy();

    global_app = app;

    ksAndroidParm parms;
    GetIntentParms(&parms);

    StartApplication(parms.argc, parms.argv);
}

#endif
