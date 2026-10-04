// Copyright (c) 2021 OPEN CASCADE SAS
//
// This file is part of Open CASCADE Technology software library.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 2.1 as published
// by the Free Software Foundation, with special exception defined in the file
// OCCT_LGPL_EXCEPTION.txt. Consult the file LICENSE_LGPL_21.txt included in OCCT
// distribution for complete text of the license and disclaimer of any warranty.
//
// Alternatively, this file may be used under the terms of Open CASCADE
// commercial license or contractual agreement.

#ifndef OpenGl_GlNative_HeaderFile
#define OpenGl_GlNative_HeaderFile

// required for correct APIENTRY definition
#if defined(_WIN32) && !defined(APIENTRY) && !defined(__CYGWIN__) && !defined(__SCITECH_SNAP__)
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
#endif

#ifndef APIENTRY
  #define APIENTRY
#endif
#ifndef APIENTRYP
  #define APIENTRYP APIENTRY*
#endif
#ifndef GLAPI
  #define GLAPI extern
#endif

#ifndef GL_APICALL
  #define GL_APICALL GLAPI
#endif

// exclude modern definitions and system-provided glext.h, should be defined before gl.h inclusion
#ifndef GL_GLEXT_LEGACY
  #define GL_GLEXT_LEGACY
#endif
#ifndef GLX_GLXEXT_LEGACY
  #define GLX_GLXEXT_LEGACY
#endif

// TargetConditionals.h must be included BEFORE any test of TARGET_OS_* macros.
// Testing an undefined macro silently yields 0, which would wrongly select the
// desktop macOS branch when targeting iOS / visionOS.
// Note that TARGET_OS_MAC must not be used to mean "desktop macOS" - it is 1 on
// iOS too (TARGET_OS_IPHONE is a subset of TARGET_OS_MAC); use TARGET_OS_OSX.
// TargetConditionals.h is a plain C header (with include guard) - use #include.
#if defined(__APPLE__)
  #include <TargetConditionals.h>
#endif

// include main OpenGL header provided with system
// Apple 桌面平台(macOS)在启用 GLES2 时走 GLES 分支：此时 OpenGL ES 由
// EGL/ANGLE 提供，系统不自带 OpenGLES.framework，必须用 GLES3 头，
// 否则会误引入桌面 <OpenGL/gl.h> 而与 ANGLE 提供的 GLES2 运行时冲突。
#if defined(__APPLE__) && !defined(HAVE_GLES2) && !defined(OCCT_UWP)                        \
  && !(defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE)
  // macOS 10.4 deprecated OpenGL framework - suppress useless warnings
  #define GL_SILENCE_DEPRECATION
  #include <OpenGL/gl.h>
  #define __X_GL_H // prevent chaotic gl.h inclusions to avoid compile errors
#elif defined(HAVE_GLES2) || defined(OCCT_UWP) || defined(__ANDROID__) || defined(__QNX__)         \
  || defined(__EMSCRIPTEN__)
  #if defined(_WIN32)
    // Angle OpenGL ES headers do not define function prototypes even for core functions,
    // however OCCT is expected to be linked against libGLESv2
    #define GL_GLEXT_PROTOTYPES
  #endif
  #if defined(__APPLE__)
    #define GL_SILENCE_DEPRECATION
    #if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
      #include <OpenGLES/ES3/gl.h>
    #else
      // macOS 桌面 + GLES2：GLES 头由 EGL/ANGLE 提供（如 vcpkg 的
      // libGLESv2_angle 附带 GLES3/gl3.h），不是系统 framework。
      #include <GLES3/gl3.h>
    #endif
    #define __X_GL_H // prevent chaotic gl.h inclusions to avoid compile errors
  #else
    #include <GLES3/gl3.h>
  #endif
#else
  #include <GL/gl.h>
#endif

#endif // OpenGl_GlNative_HeaderFile
