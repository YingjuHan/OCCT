// Copyright (c) 2026 OPEN CASCADE SAS
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

#ifndef OpenGl_HasEGL_HeaderFile
#define OpenGl_HasEGL_HeaderFile

//! Single point of definition of HAVE_EGL macro.
//!
//! HAVE_EGL is NOT defined by the build system - there is no add_definitions()
//! for it anywhere in adm/cmake. Historically every translation unit dealing
//! with EGL had to reproduce the same platform deduction on its own, which is
//! error prone: a single forgotten copy silently switches the whole file to the
//! platform-native GL backend (CGL on macOS, GLX on Linux, WGL on Windows)
//! without any compilation or link error - the EGL code path simply becomes
//! dead code.
//!
//! Include this header instead of duplicating the deduction.
#if !defined(HAVE_EGL)
  #if defined(__ANDROID__) || defined(__QNX__) || defined(__EMSCRIPTEN__) || defined(HAVE_GLES2)     \
    || defined(OCCT_UWP)
    // Note: Apple is intentionally NOT excluded here.
    // When OCCT is built with USE_GLES2 on macOS / iOS, OpenGL ES is provided
    // through EGL (ANGLE on desktop macOS, EAGL on iOS), so the EGL code path
    // must stay reachable - excluding __APPLE__ used to make it dead code.
    #define HAVE_EGL // EAGL is used instead of EGL
  #elif !defined(_WIN32) && !defined(__APPLE__) && !defined(HAVE_XLIB)
    #define HAVE_EGL
  #endif
#endif

#endif // OpenGl_HasEGL_HeaderFile
