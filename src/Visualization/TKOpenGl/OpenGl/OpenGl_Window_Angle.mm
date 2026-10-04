// Copyright (c) 2026 OPEN CASCADE SAS
//
// This file is part of Open CASCADE Technology software library.
//
// This library is free; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 2.1 as published
// by the Free Software Foundation, with special exception defined in the file
// OCCT_LGPL_EXCEPTION.txt. Consult the file LICENSE_LGPL_21.txt included in OCCT
// distribution for more information about the GNU Lesser General Public License
// version 2.1
//
// Alternatively, this file may be used under the terms of GNU General
// Public License version 2 (GPL) as published by the Free Software Foundation.

#if defined(HAVE_EGL) && defined(__APPLE__) && !defined(HAVE_XLIB)

#include <TargetConditionals.h>
#include <Cocoa/Cocoa.h>
#include <QuartzCore/QuartzCore.h>

#include <EGL/egl.h>

//! ANGLE platform type used to create the EGLDisplay on macOS.
//! ANGLE does not declare this token in eglext.h, it is resolved at runtime
//! through eglGetProcAddress(). On ANGLE 2.1.x / macOS arm64 only
//! EGL_PLATFORM_ANGLE_TYPE_OPENGLES yields a valid display; ANGLE_TYPE_ANGLE,
//! METAL and SURFACELESS_MESA all return EGL_NO_DISPLAY.
#ifndef EGL_PLATFORM_ANGLE_TYPE_OPENGLES
  #define EGL_PLATFORM_ANGLE_TYPE_OPENGLES 0x3202
#endif

//! Return CAMetalLayer hosting the drawable of the given NSView.
//! Creates the layer on demand and keeps its size in sync with the view,
//! accounting for the Retina backing scale factor.
static CAMetalLayer* getOrCreateMetalLayer (NSView* theView)
{
  if (theView == nil)
  {
    return nil;
  }
  if (![theView wantsLayer])
  {
    [theView setWantsLayer:YES];
  }
  CALayer* aLayer = [theView layer];
  if (![aLayer isKindOfClass:[CAMetalLayer class]])
  {
    aLayer = [CAMetalLayer layer];
    [theView setLayer:aLayer];
  }
  CAMetalLayer* aMetalLayer = (CAMetalLayer*)aLayer;

  // keep drawable in sync with the view size in pixels (Retina-aware)
  double aScale = 1.0;
  if (theView.window != nil && theView.window.backingScaleFactor > 0.0)
  {
    aScale = theView.window.backingScaleFactor;
  }
  aMetalLayer.contentsScale = aScale;
  const NSSize aSize = theView.frame.size;
  const CGSize aSizePx = CGSizeMake (aSize.width * aScale, aSize.height * aScale);
  if (aMetalLayer.drawableSize.width != aSizePx.width
   || aMetalLayer.drawableSize.height != aSizePx.height)
  {
    aMetalLayer.drawableSize = aSizePx;
  }
  return aMetalLayer;
}

//! Create an EGL window surface bound to the CAMetalLayer of the native view.
//! Returns EGL_NO_SURFACE on failure.
extern "C" void* OpenGl_Window_CreateAngleSurface (void* theNativeView,
                                                  void* theEglDisplay,
                                                  void* theEglConfig)
{
  NSView* aView = (NSView*)theNativeView;
  CAMetalLayer* aLayer = getOrCreateMetalLayer (aView);
  if (aLayer == nil || theEglDisplay == nullptr || theEglConfig == nullptr)
  {
    return (void*)EGL_NO_SURFACE;
  }

  typedef EGLSurface (*TOpenGl_Proc_eglCreatePlatformWindowSurface) (EGLDisplay, EGLConfig,
                                                                     void*, const EGLint*);
  TOpenGl_Proc_eglCreatePlatformWindowSurface aProc =
    (TOpenGl_Proc_eglCreatePlatformWindowSurface)eglGetProcAddress ("eglCreatePlatformWindowSurfaceEXT");
  if (aProc == nullptr)
  {
    aProc = (TOpenGl_Proc_eglCreatePlatformWindowSurface)eglGetProcAddress ("eglCreatePlatformWindowSurface");
  }
  if (aProc == nullptr)
  {
    return (void*)EGL_NO_SURFACE;
  }

  // explicitly request the OpenGLES ANGLE platform, consistent with InitContext()
  EGLint aSurfAttribs[] =
  {
    EGL_PLATFORM_ANGLE_TYPE_OPENGLES, EGL_PLATFORM_ANGLE_TYPE_OPENGLES,
    EGL_NONE
  };
  return (void*)aProc ((EGLDisplay)theEglDisplay, (EGLConfig)theEglConfig, (void*)aLayer, aSurfAttribs);
}

//! Resize the CAMetalLayer of the native view to match its current frame.
//! Called on window resize so that the EGL surface follows the window.
extern "C" void OpenGl_Window_SyncAngleLayer (void* theNativeView)
{
  getOrCreateMetalLayer ((NSView*)theNativeView);
}

#endif // HAVE_EGL && __APPLE__ && !HAVE_XLIB
