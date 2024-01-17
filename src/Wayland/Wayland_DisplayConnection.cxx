// Copyright (c) 2024-2026 Kirill Gavrilov
//
// This file is part of Open CASCADE Technology software library.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 2.1 as published
// by the Free Software Foundation, with special exception defined in the file
// OCCT_LGPL_EXCEPTION.txt. Consult the file LICENSE_LGPL_21.txt included in OCCT
// distribution for complete text of the license and disclaimer of any warranty.
//
// Alternatively, this file may be used under the terms of
// commercial license or contractual agreement.

#include <Wayland_DisplayConnection.hxx>

#include <Aspect_DisplayConnectionDefinitionError.hxx>
#include <Message.hxx>

#if defined(HAVE_WAYLAND)
  #include <wayland-client.h>
  #include <wayland-cursor.h>
  // generated from xdg-shell.xml
  #include "xdg-shell-client-protocol.pxx"
#endif

IMPLEMENT_STANDARD_RTTIEXT(Wayland_DisplayConnection,Aspect_DisplayConnection)

// =======================================================================
// function : Wayland_DisplayConnection
// purpose  :
// =======================================================================
Wayland_DisplayConnection::Wayland_DisplayConnection()
{
  Init(nullptr);
}

// =======================================================================
// function : ~Wayland_DisplayConnection
// purpose  :
// =======================================================================
Wayland_DisplayConnection::~Wayland_DisplayConnection()
{
  Close();
}

// =======================================================================
// function : Close
// purpose  :
// =======================================================================
void Wayland_DisplayConnection::Close()
{
#if defined(HAVE_WAYLAND)
  myWlCursorMainSurf.reset();
  myWlCursorTheme.reset();

  myWlPointer.reset();
  myWlKeyboard.reset();
  myWlDisplay.reset();

  myWlCompositor = nullptr;
  myWlShm        = nullptr;
  myWlSeat       = nullptr;
  myWlXdgWmBase  = nullptr;
  myWlCursorMainImg = nullptr;
  myIsOwnDisplay = false;
#endif
}

// =======================================================================
// function : Wayland_DisplayConnection
// purpose  :
// =======================================================================
Wayland_DisplayConnection::Wayland_DisplayConnection(const TCollection_AsciiString& theDisplayName)
{
  Init(nullptr, theDisplayName);
}

// =======================================================================
// function : Wayland_DisplayConnection
// purpose  :
// =======================================================================
Wayland_DisplayConnection::Wayland_DisplayConnection(Aspect_XDisplay* theDisplay)
{
  Init(theDisplay);
}

// =======================================================================
// function : FileDescriptor
// purpose  :
// =======================================================================
int Wayland_DisplayConnection::FileDescriptor() const
{
#if defined(HAVE_WAYLAND)
  return myWlDisplay.get() != nullptr ? wl_display_get_fd(myWlDisplay.get()) : 0;
#else
  return 0;
#endif
}

// =======================================================================
// function : Init
// purpose  :
// =======================================================================
void Wayland_DisplayConnection::Init(Aspect_XDisplay* theDisplay,
                                     const TCollection_AsciiString& theDisplayName)
{
  Close();
#if defined(HAVE_WAYLAND)
  if (theDisplay != nullptr)
    myWlDisplay.reset((wl_display*)theDisplay, [](wl_display* theDisp) { (void)theDisp; });
  else
    myWlDisplay.reset(wl_display_connect(!theDisplayName.IsEmpty() ? theDisplayName.ToCString() : nullptr),
                      [](wl_display* theDisp) { theDisp ? wl_display_disconnect(theDisp) : (void)theDisp; });

  if (myWlDisplay.get() == nullptr)
  {
    TCollection_AsciiString aMessage =
        TCollection_AsciiString("Cannot connect to the Wayland server \"") + theDisplayName + "\"";
    throw Aspect_DisplayConnectionDefinitionError(aMessage.ToCString());
  }
  myIsOwnDisplay = (theDisplay == nullptr);

  static const wl_registry_listener aWlListener =
  {
    .global = [](void* theData,
                 wl_registry* theRegistry,
                 uint32_t theId,
                 const char* theInterface,
                 uint32_t theVersion) {
      Wayland_DisplayConnection* aThis = (Wayland_DisplayConnection*)theData;
      //Message::SendTrace() << "Wayland interface: " << theInterface << " v." << theVersion;
      if (::strcmp(theInterface, "wl_compositor") == 0)
      {
        // it is unclear from documentation, but it seems that version of compositor interface
        // defines version of created wl_surface from it
        static constexpr int aTopCompVer = WL_SURFACE_PREFERRED_BUFFER_SCALE_SINCE_VERSION;
        aThis->myWlCompositor = (wl_compositor* )wl_registry_bind(theRegistry, theId, &wl_compositor_interface,
                                                                  Min(theVersion, aTopCompVer));
        return;
      }
      if (::strcmp(theInterface, "wl_shm") == 0)
      {
        aThis->myWlShm = (wl_shm* )wl_registry_bind(theRegistry, theId, &wl_shm_interface, 1);
        return;
      }
      if (::strcmp(theInterface, "wl_output") == 0)
      {
        // we need at least WL_OUTPUT_SCALE_SINCE_VERSION
        WlOutputInfo anOutput;
        anOutput.Output = (wl_output* )wl_registry_bind(theRegistry, theId, &wl_output_interface,
                                                        Min(theVersion, WL_OUTPUT_NAME_SINCE_VERSION));
        aThis->myWlOutputs.push_back(anOutput);

        static const wl_output_listener anOutListener =
        {
          .geometry = [](void* , wl_output* , int32_t , int32_t , int32_t , int32_t , int32_t ,
                         const char* , const char* , int32_t ) {},
          .mode = [](void* , wl_output* , uint32_t , int32_t , int32_t , int32_t ) {},
          .done = [](void* , wl_output* ) {},
          .scale = [](void* theData, wl_output* theOutput, int32_t theFactor)
          {
            Wayland_DisplayConnection* aThis = (Wayland_DisplayConnection*)theData;
            for (WlOutputInfo& anOutput : aThis->myWlOutputs)
            {
              if (anOutput.Output == theOutput)
              {
                anOutput.Scale = theFactor;
                break;
              }
            }
          },
          .name = [](void* , wl_output* , const char* ) {},
          .description = [](void* , wl_output* , const char* ) {}
        };
        wl_output_add_listener(anOutput.Output, &anOutListener, aThis);
        return;
      }
      if (::strcmp(theInterface, "wl_seat") == 0)
      {
        aThis->myWlSeat = (wl_seat* )wl_registry_bind(theRegistry, theId, &wl_seat_interface,
                                                      Min(theVersion, WL_POINTER_AXIS_VALUE120_SINCE_VERSION));

        static const wl_seat_listener aSeatListener =
        {
          .capabilities = [](void* theData, wl_seat* theSeat, uint32_t theCaps)
          {
            Wayland_DisplayConnection* aThis = (Wayland_DisplayConnection*)theData;
            if ((theCaps & WL_SEAT_CAPABILITY_POINTER) != 0 && !aThis->myWlPointer)
              aThis->myWlPointer.reset(wl_seat_get_pointer(theSeat),
                                       [](wl_pointer* theObj) { theObj ? wl_pointer_destroy(theObj) : (void)theObj; });
            else if ((theCaps & WL_SEAT_CAPABILITY_POINTER) == 0 && aThis->myWlPointer)
              aThis->myWlPointer.reset();

            if ((theCaps & WL_SEAT_CAPABILITY_KEYBOARD) != 0 && !aThis->myWlKeyboard)
              aThis->myWlKeyboard.reset(wl_seat_get_keyboard(theSeat),
                                        [](wl_keyboard* theKey) { theKey ? wl_keyboard_destroy(theKey) : (void)theKey; });
            else if ((theCaps & WL_SEAT_CAPABILITY_KEYBOARD) == 0 && aThis->myWlKeyboard)
              aThis->myWlKeyboard.reset();
          },
          .name = [](void* , wl_seat* , const char* ) {}
        };
        wl_seat_add_listener(aThis->myWlSeat, &aSeatListener, aThis);
        return;
      }
      if (::strcmp(theInterface, xdg_wm_base_interface.name) == 0)
      {
        static const xdg_wm_base_listener anXdgBaseList =
        {
          .ping = [](void* theData, xdg_wm_base* theXdgWmBase, uint32_t theSerial)
          {
            (void )theData;
            xdg_wm_base_pong(theXdgWmBase, theSerial);
          }
        };

        aThis->myWlXdgWmBase = (xdg_wm_base* )wl_registry_bind(theRegistry, theId, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(aThis->myWlXdgWmBase, &anXdgBaseList, aThis);
        return;
      }
    },
    .global_remove = [](void* theData, wl_registry* theRegistry, uint32_t theId)
    {
      (void)theData; (void)theRegistry; (void)theId;
    }
  };

  wl_registry* aWlRegistry = wl_display_get_registry(myWlDisplay.get());
  wl_registry_add_listener(aWlRegistry, &aWlListener, this);
  // call attached listener
  wl_display_dispatch(myWlDisplay.get());
  wl_display_roundtrip(myWlDisplay.get());
  if (myWlCompositor == nullptr)
    throw Aspect_DisplayConnectionDefinitionError("Wayland server has no compositor");

  if (myWlXdgWmBase == nullptr)
    throw Aspect_DisplayConnectionDefinitionError("Wayland server has no XDG shell");

#else
  (void)theDisplayName;
  myWlDisplay.reset((wl_display*)theDisplay, [](wl_display* theDisp) { (void)theDisp; });
  myIsOwnDisplay = (theDisplay == nullptr);
#endif
}

// =======================================================================
// function : Flush
// purpose  :
// =======================================================================
int Wayland_DisplayConnection::Flush()
{
  if (myWlDisplay.get() == nullptr)
    return -1;

#if defined(HAVE_WAYLAND)
  return wl_display_dispatch(myWlDisplay.get());
#else
  return -1;
#endif
}

// =======================================================================
// function : DispatchPending
// purpose  :
// =======================================================================
int Wayland_DisplayConnection::DispatchPending()
{
  if (myWlDisplay.get() == nullptr)
    return -1;

#if defined(HAVE_WAYLAND)
  return wl_display_dispatch_pending (myWlDisplay.get());
#else
  return -1;
#endif
}

// =======================================================================
// function : SetWlMainCursor
// purpose  :
// =======================================================================
void Wayland_DisplayConnection::SetWlMainCursor(wl_pointer* thePointer, uint32_t theSerial, double theScale)
{
#if defined(HAVE_WAYLAND)
  if (myWlCursorScale != Round(theScale))
  {
    myWlCursorMainSurf.reset();
    myWlCursorTheme.reset();
  }

  myWlCursorScale = Round(theScale);
  if (!myWlCursorTheme)
    myWlCursorTheme.reset(wl_cursor_theme_load(nullptr, Max(24, (int)Round(24 * myWlCursorScale)), myWlShm),
                          [](wl_cursor_theme* theObj) { theObj ? wl_cursor_theme_destroy(theObj) : (void)theObj; });

  if (!myWlCursorTheme)
    return;

  if (!myWlCursorMainSurf)
  {
    wl_cursor* aCursor = wl_cursor_theme_get_cursor(myWlCursorTheme.get(), "left_ptr");
    if (aCursor == nullptr)
      return;

    myWlCursorMainImg = aCursor->images[0];
    wl_buffer* aBuffer = wl_cursor_image_get_buffer(myWlCursorMainImg);

    myWlCursorMainSurf.reset(wl_compositor_create_surface(myWlCompositor),
                             [](wl_surface* theSurf) { theSurf != nullptr ? wl_surface_destroy(theSurf) : (void)theSurf; });
    if (!myWlCursorMainSurf)
      return;

    if (wl_surface_get_version(myWlCursorMainSurf.get()) >= WL_SURFACE_SET_BUFFER_SCALE_SINCE_VERSION)
      wl_surface_set_buffer_scale(myWlCursorMainSurf.get(), (int)Round(myWlCursorScale));

    wl_surface_attach(myWlCursorMainSurf.get(), aBuffer, 0, 0);
    wl_surface_commit(myWlCursorMainSurf.get());
  }

  wl_pointer_set_cursor(thePointer, theSerial, myWlCursorMainSurf.get(),
                        myWlCursorMainImg->hotspot_x, myWlCursorMainImg->hotspot_y);
#else
  (void)thePointer, (void)theSerial, (void)theScale;
  (void)myWlCursorMainImg, (void)myWlCursorScale;
#endif
}
