/*
================================================================================================

Description :   Convenient wrapper for the OpenGL API.
Author      :   J.M.P. van Waveren
Date        :   12/21/2014
Language    :   C99
Format      :   Real tabs with the tab size equal to 4 spaces.
Copyright   :   Copyright (c) 2016 Oculus VR, LLC. All Rights reserved.


LICENSE
=======

Copyright (c) 2016 Oculus VR, LLC.

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
*/
#if !defined(KSGRAPHICSWRAPPER_COMMON_H)
#define KSGRAPHICSWRAPPER_COMMON_H

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
#if defined(SUPPORT_X)
//#define OS_LINUX_XLIB
#define OS_LINUX_XCB
#endif
#else
#error "unknown platform"
#endif

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
================================================================================================================================

String Hash.

ksStringHash

================================================================================================================================
*/

typedef uint32_t ksStringHash;

static inline void ksStringHash_Init(ksStringHash *hash) { *hash = 5381; }

static inline void ksStringHash_Update(ksStringHash *hash, const char *string) {
    ksStringHash value = *hash;
    for (int i = 0; string[i] != '\0'; i++) {
        value = ((value << 5) - value) + string[i];
    }
    *hash = value;
}

/*
================================================================================================================================

Rectangles.

ksScreenRect
ksClipRect

ksScreenRect is specified in pixels with 0,0 at the left-bottom.
ksClipRect is specified in clip space in the range [-1,1], with -1,-1 at the left-bottom.

static ksClipRect ksScreenRect_ToClipRect( const ksScreenRect * screenRect, const int resolutionX, const int resolutionY );
static ksScreenRect ksClipRect_ToScreenRect( const ksClipRect * clipRect, const int resolutionX, const int resolutionY );

================================================================================================================================
*/

typedef struct {
    int x;
    int y;
    int width;
    int height;
} ksScreenRect;

typedef struct {
    float x;
    float y;
    float width;
    float height;
} ksClipRect;

static inline ksClipRect ksScreenRect_ToClipRect(const ksScreenRect *screenRect, const int resolutionX, const int resolutionY) {
    ksClipRect clipRect;
    clipRect.x = 2.0f * screenRect->x / resolutionX - 1.0f;
    clipRect.y = 2.0f * screenRect->y / resolutionY - 1.0f;
    clipRect.width = 2.0f * screenRect->width / resolutionX;
    clipRect.height = 2.0f * screenRect->height / resolutionY;
    return clipRect;
}

static inline ksScreenRect ksClipRect_ToScreenRect(const ksClipRect *clipRect, const int resolutionX, const int resolutionY) {
    ksScreenRect screenRect;
    screenRect.x = (int)((clipRect->x * 0.5f + 0.5f) * resolutionX + 0.5f);
    screenRect.y = (int)((clipRect->y * 0.5f + 0.5f) * resolutionY + 0.5f);
    screenRect.width = (int)(clipRect->width * 0.5f * resolutionX + 0.5f);
    screenRect.height = (int)(clipRect->height * 0.5f * resolutionY + 0.5f);
    return screenRect;
}

#ifdef __cplusplus
}
#endif

#endif  // !KSGRAPHICSWRAPPER_COMMON_H
