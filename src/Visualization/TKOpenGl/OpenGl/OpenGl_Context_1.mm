// Created on: 2012-11-12
// Created by: Kirill GAVRILOV
// Copyright (c) 2012-2014 OPEN CASCADE SAS
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

// macOS 桌面默认走 NSOpenGL/CGL；启用 GLES2(HAVE_GLES2) 时上下文由
// EGL/ANGLE 提供，IsCurrent/MakeCurrent/SwapBuffers/Init 的实现位于
// OpenGl_Context.cxx，本文件必须整体排除（否则 EGL 句柄会被当作
// NSOpenGLContext* 发消息，导致 EXC_BAD_ACCESS）。
#if defined(__APPLE__) && !defined(HAVE_XLIB) && !defined(HAVE_GLES2)

#ifndef GL_GLEXT_LEGACY
#define GL_GLEXT_LEGACY // To prevent inclusion of system glext.h on Mac OS X 10.6.8
#endif

// macOS 10.4 deprecated OpenGL framework - suppress useless warnings
#define GL_SILENCE_DEPRECATION

#import <TargetConditionals.h>

// Suppress warnings from macOS system headers
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#pragma clang diagnostic ignored "-Wmissing-method-return-type"
#pragma clang diagnostic ignored "-Wavailability"
#if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
  #import <UIKit/UIKit.h>
#else
  #import <Cocoa/Cocoa.h>
#endif
#pragma clang diagnostic pop

#include <OpenGl_GlCore11.hxx>
#include <OpenGl_Context.hxx>
#include <OpenGl_FrameBuffer.hxx>
#include <OpenGl_ShaderManager.hxx>

#include <Standard_ProgramError.hxx>

//=================================================================================================

bool OpenGl_Context::IsCurrent() const
{
#if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
  return myGContext != NULL
      && [EAGLContext     currentContext] == myGContext;
#else
  return myGContext != NULL
      && [NSOpenGLContext currentContext] == myGContext;
#endif
}

//=================================================================================================

bool OpenGl_Context::MakeCurrent()
{
  if (myGContext == NULL)
  {
    Standard_ProgramError_Raise_if (myIsInitialized, "OpenGl_Context::Init() should be called before!");
    return false;
  }

#if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
  if ([EAGLContext setCurrentContext: myGContext] != YES)
  {
    return false;
  }
#else
  [myGContext makeCurrentContext];
#endif

  // The shader manager is shared between contexts and is detached from its
  // context (SetContext(NULL)) when a context is destroyed while the shared
  // resource set is still referenced by another one. It must therefore be
  // re-attached on every successful MakeCurrent, as done by the EGL, WGL and
  // GLX implementations in OpenGl_Context.cxx. Without this line a later
  // Redraw() reaches OpenGl_ShaderManager with a null context and crashes on
  // the first state query (OpenGl_Context::HasSRGB()).
  myShaderManager->SetContext(this);
  return true;
}

//=================================================================================================

void OpenGl_Context::SwapBuffers()
{
  if (myGContext == NULL)
  {
    return;
  }

#if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
  if (myDefaultFbo.IsNull()
  || !myDefaultFbo->IsValid()
  ||  myDefaultFbo->ColorRenderBuffer() == 0)
  {
    return;
  }

  myFuncs->glBindRenderbuffer (GL_RENDERBUFFER, myDefaultFbo->ColorRenderBuffer());
  [myGContext presentRenderbuffer: GL_RENDERBUFFER];
  //::glBindRenderbuffer (GL_RENDERBUFFER, 0);
#else
  core11fwd->glFinish();
  [myGContext flushBuffer];
#endif
}

//=================================================================================================

bool OpenGl_Context::Init (const bool theIsCoreProfile)
{
  if (myIsInitialized)
  {
    return true;
  }

#if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
  myGContext = [EAGLContext     currentContext];
#else
  myGContext = [NSOpenGLContext currentContext];
#endif
  if (myGContext == NULL)
  {
    return false;
  }

  init (theIsCoreProfile);
  myIsInitialized = true;
  return true;
}

#endif // __APPLE__
