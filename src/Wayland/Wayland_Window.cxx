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

#include <Wayland_Window.hxx>

#include <Aspect_ScrollDelta.hxx>
#include <Aspect_WindowDefinitionError.hxx>
#include <Aspect_WindowInputListener.hxx>
#include <Message.hxx>

#if defined(HAVE_WAYLAND)
  #include <linux/input-event-codes.h>
  #include <wayland-client.h>
  #include <wayland-egl.h> // should be included before EGL headers
  // generated from xdg-shell.xml
  #include "xdg-shell-client-protocol.pxx"
#endif

#include <Wayland_DisplayConnection.hxx>

IMPLEMENT_STANDARD_RTTIEXT(Wayland_Window, Aspect_Window)

#if defined(HAVE_WAYLAND)
// =======================================================================
// function : getSurfaceListener
// purpose  :
// =======================================================================
wl_surface_listener Wayland_Window::getSurfaceListener()
{
  wl_surface_listener aList = {};
  aList.enter = [](void* theData, wl_surface* theSurface, wl_output* theOutput)
  {
    Wayland_Window* aThis = (Wayland_Window*)theData;
    if (aThis->myWlSurface.get() != theSurface)
      return;

    if (aThis->myWlOutputs.empty())
      aThis->myWlActiveOutput = theOutput;

    aThis->myWlOutputs.insert(theOutput);
    aThis->performBufferResize(aThis->myLogicalSize.x(), aThis->myLogicalSize.y());
  };

  aList.leave = [](void* theData, wl_surface* theSurface, wl_output* theOutput)
  {
    Wayland_Window* aThis = (Wayland_Window*)theData;
    if (aThis->myWlSurface.get() != theSurface)
      return;

    aThis->myWlOutputs.erase(theOutput);
    if (aThis->myWlActiveOutput == theOutput)
      aThis->myWlActiveOutput = !aThis->myWlOutputs.empty() ? *aThis->myWlOutputs.begin() : nullptr;

    aThis->performBufferResize(aThis->myLogicalSize.x(), aThis->myLogicalSize.y());
  };

  // 6+ (WL_SURFACE_PREFERRED_BUFFER_SCALE_SINCE_VERSION+)
  // this is more straightforward interface to get scale than tracking wl_output,
  // but it was introduced recently
  aList.preferred_buffer_scale = [](void* theData, wl_surface* theSurface, int32_t theFactor)
  {
    Wayland_Window* aThis = (Wayland_Window*)theData;
    if (aThis->myWlSurface.get() != theSurface)
      return;

    const double aNewScale = theFactor;
    if (aThis->myDevicePixelRatio == aNewScale)
      return;
  };

  aList.preferred_buffer_transform = [](void* theData, wl_surface* theSurface, uint32_t theTransform)
  {
    Wayland_Window* aThis = (Wayland_Window*)theData;
    if (aThis->myWlSurface.get() != theSurface)
      return;

    (void)theTransform;
  };

  return aList;
}

// =======================================================================
// function : performBufferResize
// purpose  :
// =======================================================================
void Wayland_Window::performBufferResize(int32_t theWidth, int32_t theHeight)
{
  const bool isScallable = !myIsDpiUnaware && !IsVirtual()
                        && wl_surface_get_version(myWlSurface.get()) >= WL_SURFACE_SET_BUFFER_SCALE_SINCE_VERSION;
  double aNewScale = isScallable ? 1.0 : myDevicePixelRatio;
  if (isScallable)
  {
    for (const wl_output* anOutIter : myWlOutputs)
      aNewScale = Max(aNewScale, (double)myDisplay->GetOutputScale(anOutIter));
  }
  const Graphic3d_Vec2i aNewSize(theWidth, theHeight);
  const Graphic3d_Vec2i aNewBufferSize((int)Round(aNewSize.x() * aNewScale),
                                       (int)Round(aNewSize.y() * aNewScale));
  const bool isScaleChanged = myDevicePixelRatio != aNewScale;
  if (myLogicalSize == aNewSize && myBufferSize == aNewBufferSize && !isScaleChanged)
    return;

  myDevicePixelRatio = aNewScale;
  myLogicalSize = aNewSize;
  myBufferSize = aNewBufferSize;
  if (isScallable)
    wl_surface_set_buffer_scale(myWlSurface.get(), (int)Round(aNewScale));

  wl_egl_window_resize(myWlEglWindow.get(), aNewBufferSize.x(), aNewBufferSize.y(), 0, 0);
  wl_surface_commit(myWlSurface.get());
  if (myListener != nullptr)
  {
    if (isScaleChanged)
      myListener->ProcessDpiChange();
    else
      myListener->ProcessConfigure(true);
  }
}

// =======================================================================
// function : getXdgTopLevelListener
// purpose  :
// =======================================================================
xdg_toplevel_listener Wayland_Window::getXdgTopLevelListener()
{
  xdg_toplevel_listener aList = {};
  aList.configure = [](void* theData, xdg_toplevel* theXdgTop,
                       int32_t theWidth, int32_t theHeight, wl_array* theStates)
  {
    Wayland_Window* aThis = (Wayland_Window*)theData;
    if (aThis->myWlXdgTop.get() != theXdgTop)
      return;

    Aspect_WindowInputListener* aViewList = aThis->GetListener();
    for (size_t aStateIter = 0; aStateIter < theStates->size; ++aStateIter)
    {
      const uint32_t aState = ((uint32_t*)theStates->data)[aStateIter];
      switch (aState)
      {
        case XDG_TOPLEVEL_STATE_MAXIMIZED:
          break;
        case XDG_TOPLEVEL_STATE_FULLSCREEN:
          break;
        case XDG_TOPLEVEL_STATE_RESIZING:
          break;
        case XDG_TOPLEVEL_STATE_ACTIVATED:
          if (aViewList != nullptr)
          {
            aViewList->ProcessFocus(true);
          }
          break;
      }
    }

    if (theWidth != 0 && theHeight != 0)
      aThis->performBufferResize(theWidth, theHeight);
  };

  aList.close = [](void* theData, xdg_toplevel* theXdgTop)
  {
    Wayland_Window* aThis = (Wayland_Window*)theData;
    if (aThis->myWlXdgTop.get() != theXdgTop)
      return;

    Aspect_WindowInputListener* aViewList = aThis->GetListener();
    if (aViewList != nullptr)
      aViewList->ProcessClose();
  };

  aList.configure_bounds = [](void* theData, xdg_toplevel* theXdgTop,
                              int32_t theWidth, int32_t theHeight)
  {
    (void)theWidth, (void)theHeight; // just ignore event
    Wayland_Window* aThis = (Wayland_Window*)theData;
    if (aThis->myWlXdgTop.get() != theXdgTop)
      return;
  };

  aList.wm_capabilities = [](void* theData, xdg_toplevel* theXdgTop, wl_array* theCaps)
  {
    (void)theCaps; // just ignore event
    Wayland_Window* aThis = (Wayland_Window*)theData;
    if (aThis->myWlXdgTop.get() != theXdgTop)
      return;
  };

  return aList;
}

// =======================================================================
// function : getPointerListener
// purpose  :
// =======================================================================
wl_pointer_listener Wayland_Window::getPointerListener()
{
  wl_pointer_listener aList = {};

  aList.enter = [](void* theData, wl_pointer* thePointer, uint32_t theSerial,
                   wl_surface* theSurface, wl_fixed_t theSurfX, wl_fixed_t theSurfY)
  {
    Wayland_Window* aThis = (Wayland_Window*)theData;
    aThis->myIsPointerIn = aThis->myWlSurface.get() == theSurface;

    Aspect_WindowInputListener* aViewList = aThis->GetListener();
    if (aViewList == nullptr || !aThis->myIsPointerIn)
      return;

    aThis->myDisplay->SetWlMainCursor(thePointer, theSerial, aThis->myDevicePixelRatio);

    const Graphic3d_Vec2d aNewPos2d = aThis->ConvertPointToBacking(Graphic3d_Vec2d(wl_fixed_to_double(theSurfX), wl_fixed_to_double(theSurfY)));
    const Graphic3d_Vec2i aNewPos2i = Graphic3d_Vec2i(aNewPos2d + Graphic3d_Vec2d(0.5));
    const bool isEmulated = false;
    if (aViewList->UpdateMousePosition(aNewPos2i, aViewList->PressedMouseButtons(), aViewList->LastMouseFlags(), isEmulated))
      aViewList->ProcessInput();
  };

  aList.leave = [](void* theData, wl_pointer* , uint32_t , wl_surface* theSurface)
  {
    Wayland_Window* aThis = (Wayland_Window*)theData;
    if (aThis->myWlSurface.get() == theSurface)
      aThis->myIsPointerIn = false;
  };

  aList.motion = [](void* theData, wl_pointer* , uint32_t , wl_fixed_t theSurfX, wl_fixed_t theSurfY)
  {
    Wayland_Window* aThis = (Wayland_Window*)theData;
    Aspect_WindowInputListener* aViewList = aThis->GetListener();
    if (aViewList == nullptr || !aThis->myIsPointerIn)
      return;

    const Graphic3d_Vec2d aNewPos2d = aThis->ConvertPointToBacking(Graphic3d_Vec2d(wl_fixed_to_double(theSurfX), wl_fixed_to_double(theSurfY)));
    const Graphic3d_Vec2i aNewPos2i = Graphic3d_Vec2i(aNewPos2d + Graphic3d_Vec2d(0.5));
    const bool isEmulated = false;
    if (aViewList->UpdateMousePosition(aNewPos2i, aViewList->PressedMouseButtons(), aThis->myKeyModifiers, isEmulated))
      aViewList->ProcessInput();
  };

  aList.button = [](void* theData, wl_pointer* , uint32_t , uint32_t , uint32_t theButton, uint32_t theState)
  {
    Wayland_Window* aThis = (Wayland_Window*)theData;
    Aspect_WindowInputListener* aViewList = aThis->GetListener();
    if (aViewList == nullptr || !aThis->myIsPointerIn)
      return;

    Aspect_VKeyMouse aButton = Aspect_VKeyMouse_NONE;
    if (theButton == BTN_LEFT)   { aButton = Aspect_VKeyMouse_LeftButton; }
    if (theButton == BTN_MIDDLE) { aButton = Aspect_VKeyMouse_MiddleButton; }
    if (theButton == BTN_RIGHT)  { aButton = Aspect_VKeyMouse_RightButton; }
    if (aButton == Aspect_VKeyMouse_NONE)
      return;

    const bool isEmulated = false;
    if (theState == WL_POINTER_BUTTON_STATE_PRESSED)
      aViewList->PressMouseButton(aViewList->LastMousePosition(), aButton, aThis->myKeyModifiers, isEmulated);
    else
      aViewList->ReleaseMouseButton(aViewList->LastMousePosition(), aButton, aThis->myKeyModifiers, isEmulated);

    aViewList->ProcessInput();
  };

  aList.axis = [](void* theData, wl_pointer* , uint32_t , uint32_t theAxis, wl_fixed_t theValue)
  {
    Wayland_Window* aThis = (Wayland_Window*)theData;
    Aspect_WindowInputListener* aViewList = aThis->GetListener();
    if (aViewList == nullptr || !aThis->myIsPointerIn || theAxis != WL_POINTER_AXIS_VERTICAL_SCROLL)
      return;
    else if (wl_seat_get_version(aThis->myDisplay->GetWlSeat()) >= WL_POINTER_AXIS_VALUE120_SINCE_VERSION)
      return; // duplicated by axis_value120

    const double aDeltaF = wl_fixed_to_double(theValue) / -15.0;
    if (aViewList->UpdateMouseScroll(Aspect_ScrollDelta(aViewList->LastMousePosition(), aDeltaF, aThis->myKeyModifiers)))
      aViewList->ProcessInput();
  };

  // just ignore these events
  aList.frame = [](void* , wl_pointer* ) {};
  aList.axis_source = [](void* , wl_pointer* , uint32_t ) {};
  aList.axis_stop = [](void* , wl_pointer* , uint32_t , uint32_t ) {};
  aList.axis_discrete = [](void* , wl_pointer* , uint32_t , int32_t ) {};

  // 8+ (WL_POINTER_AXIS_VALUE120_SINCE_VERSION+)
  aList.axis_value120 = [](void* theData, wl_pointer* , uint32_t theAxis, int32_t theValue120)
  {
    Wayland_Window* aThis = (Wayland_Window*)theData;
    Aspect_WindowInputListener* aViewList = aThis->GetListener();
    if (aViewList == nullptr || !aThis->myIsPointerIn || theAxis != WL_POINTER_AXIS_VERTICAL_SCROLL)
      return;

    const double aDeltaF = double(theValue120) / -120.0;
    if (aViewList->UpdateMouseScroll(Aspect_ScrollDelta(aViewList->LastMousePosition(), aDeltaF, aViewList->LastMouseFlags())))
      aViewList->ProcessInput();
  };

  aList.axis_relative_direction = [](void* , wl_pointer* , uint32_t , uint32_t ) {};
  return aList;
}

// =======================================================================
// function : getKeyboardListener
// purpose  :
// =======================================================================
wl_keyboard_listener Wayland_Window::getKeyboardListener()
{
  wl_keyboard_listener aList = {};
  aList.keymap = [](void* theData, wl_keyboard* theKeyboard,
                    uint32_t theFormat, int32_t theFd, uint32_t theSize)
  {
    (void)theData, (void)theKeyboard, (void)theFormat, (void)theFd, (void)theSize;
  };

  aList.enter = [](void* theData, wl_keyboard* theKeyboard,
                   uint32_t theSerial, wl_surface* theSurface, wl_array* theKeys)
  {
    (void)theKeyboard, (void)theSerial, (void)theKeys;
    Wayland_Window* aThis = (Wayland_Window*)theData;
    aThis->myIsKeyboardIn = aThis->myWlSurface.get() == theSurface;
  };

  aList.leave = [](void* theData, wl_keyboard* theKeyboard, uint32_t theSerial, wl_surface* theSurface)
  {
    (void)theKeyboard, (void)theSerial;
    Wayland_Window* aThis = (Wayland_Window*)theData;
    Aspect_WindowInputListener* aViewList = aThis->GetListener();
    if (aThis->myWlSurface.get() == theSurface)
    {
      aThis->myIsKeyboardIn = false;
      if (aViewList != nullptr)
      {
        aThis->myKeyModifiers = Aspect_VKeyFlags_NONE;
        aViewList->ProcessFocus(false);
      }
    }
  };

  aList.key = [](void* theData, wl_keyboard* theKeyboard, uint32_t theSerial,
                 uint32_t theTime, uint32_t theKey, uint32_t theState)
  {
    (void)theKeyboard, (void)theSerial;
    Wayland_Window* aThis = (Wayland_Window*)theData;
    Aspect_WindowInputListener* aViewList = aThis->GetListener();
    if (aViewList == nullptr || !aThis->myIsKeyboardIn)
      return;

    (void)theTime;
    const double aTimeStamp = aViewList->EventTime();
    Aspect_VKey aVKey = Wayland_Window::VirtualKeyFromNative(theKey);
    if (aVKey == Aspect_VKey_UNKNOWN)
      return;

    if (theState == WL_KEYBOARD_KEY_STATE_PRESSED)
      aViewList->KeyDown(aVKey, aTimeStamp);
    else
      aViewList->KeyUp(aVKey, aTimeStamp);

    Aspect_VKeyFlags aFlags = Aspect_VKeyFlags_NONE;
    if (aViewList->Keys().IsKeyDown(Aspect_VKey_Shift))
        aFlags |= Aspect_VKeyFlags_SHIFT;

    if (aViewList->Keys().IsKeyDown(Aspect_VKey_Control))
        aFlags |= Aspect_VKeyFlags_CTRL;

    if (aViewList->Keys().IsKeyDown(Aspect_VKey_Meta))
        aFlags |= Aspect_VKeyFlags_META;

    if (aViewList->Keys().IsKeyDown(Aspect_VKey_Alt))
        aFlags |= Aspect_VKeyFlags_ALT;

    aThis->myKeyModifiers = aFlags;

    aViewList->ProcessInput();
  };

  aList.modifiers = [](void* theData, wl_keyboard* theKeyboard, uint32_t theSerial,
                       uint32_t theModsDepressed, uint32_t theModsLatched, uint32_t theModsLocked, uint32_t theGroup)
  {
    // unhandled
    (void)theData, (void)theKeyboard, (void)theSerial;
    (void)theModsDepressed, (void)theModsLatched, (void)theModsLocked, (void)theGroup;
  };

  aList.repeat_info = [](void* theData, wl_keyboard* theKeyboard, int32_t theRate, int32_t theDelay)
  {
    (void)theData, (void)theKeyboard, (void)theRate, (void)theDelay;
  };

  return aList;
}
#endif

// =======================================================================
// function : Wayland_Window
// purpose  :
// =======================================================================
Wayland_Window::Wayland_Window()
{
  //
}

// =======================================================================
// function : ~Wayland_Window
// purpose  :
// =======================================================================
Wayland_Window::~Wayland_Window()
{
  Close();
}

// =======================================================================
// function : Close
// purpose  :
// =======================================================================
void Wayland_Window::Close()
{
#if defined(HAVE_WAYLAND)
  // destruct in reversed order, as some object become invalid after deletion of parent
  myWlPointer.reset();
  myWlKeyboard.reset();
  myWlEglWindow.reset();
  myWlXdgTop.reset();
  myWlXdgSurf.reset();
  myWlSurface.reset();
  myDisplay.Nullify();
#endif
}

// =======================================================================
// function : Create
// purpose  :
// =======================================================================
void Wayland_Window::Create(const Handle(Wayland_DisplayConnection)& theDisplay,
                            const Standard_CString theAppId,
                            const Standard_CString theTitle,
                            const Graphic3d_Vec2i& theSize)
{
  Close();
  myDisplay = theDisplay;
  myAppId = theAppId;
  myTitle = theTitle;
  myLogicalSize = theSize;
  myBufferSize = theSize;

  if (theSize.x() <= 0 || theSize.y() <= 0)
    throw Aspect_WindowDefinitionError("Wayland_Window, Coordinate(s) out of range");
  else if (theDisplay.IsNull())
    throw Aspect_WindowDefinitionError("Wayland_Window, Display connection is undefined");

#if defined(HAVE_WAYLAND)
  myWlSurface.reset(wl_compositor_create_surface(myDisplay->GetWlCompositor()),
                    [](wl_surface* theSurf) { theSurf != nullptr ? wl_surface_destroy(theSurf) : (void)theSurf; });
  if (myWlSurface.get() == nullptr)
    throw Aspect_WindowDefinitionError("Wayland_Window, cannot create compositor surface");

  // just a best guess, as it is tricky to get initial scale synchronously...
  myWlActiveOutput = myDisplay->GetFirstOutput();

  static const wl_surface_listener aSurfListener = getSurfaceListener();
  wl_surface_add_listener(myWlSurface.get(), &aSurfListener, this);
  wl_surface_commit(myWlSurface.get());

  const bool isScallable = !myIsDpiUnaware && !IsVirtual()
                         && wl_surface_get_version(myWlSurface.get()) >= WL_SURFACE_SET_BUFFER_SCALE_SINCE_VERSION;
  if (isScallable)
  {
    myDevicePixelRatio = myWlActiveOutput != nullptr
                       ? myDisplay->GetOutputScale(myWlActiveOutput)
                       : myDevicePixelRatio;
    myBufferSize = Graphic3d_Vec2i((int)Round(myLogicalSize.x() * myDevicePixelRatio),
                                   (int)Round(myLogicalSize.y() * myDevicePixelRatio));
    wl_surface_set_buffer_scale(myWlSurface.get(), (int)Round(myDevicePixelRatio));
  }

  std::shared_ptr<wl_region> aRegion(wl_compositor_create_region(myDisplay->GetWlCompositor()),
                                     [](wl_region* theReg) { theReg != nullptr ? wl_region_destroy(theReg) : (void)theReg; });
  wl_region_add(aRegion.get(), 0, 0, myLogicalSize.x(), myLogicalSize.y());
  wl_surface_set_opaque_region(myWlSurface.get(), aRegion.get());

  myWlEglWindow.reset(wl_egl_window_create(myWlSurface.get(), myBufferSize.x(), myBufferSize.y()),
                      [](wl_egl_window* theWin) { theWin != nullptr ? wl_egl_window_destroy(theWin) : (void)theWin; });
  if (myWlEglWindow.get() == nullptr)
    throw Aspect_WindowDefinitionError("Wayland_Window, cannot create EGL window");

  if (myDisplay->GetWlPointer() != nullptr)
    myWlPointer.reset(wl_seat_get_pointer(myDisplay->GetWlSeat()),
                      [](wl_pointer* theObj) { theObj ? wl_pointer_destroy(theObj) : (void)theObj; });

  if (myDisplay->GetWlKeyboard() != nullptr)
    myWlKeyboard.reset(wl_seat_get_keyboard(myDisplay->GetWlSeat()),
                       [](wl_keyboard* theKey) { theKey ? wl_keyboard_destroy(theKey) : (void)theKey; });

  static const wl_pointer_listener aPointerListener = getPointerListener();
  if (myDisplay->GetWlPointer() != nullptr)
    wl_pointer_add_listener(myWlPointer.get(), &aPointerListener, this);

  static const wl_keyboard_listener aKeyboardListener = getKeyboardListener();
  if (myDisplay->GetWlKeyboard() != nullptr)
    wl_keyboard_add_listener(myWlKeyboard.get(), &aKeyboardListener, this);
#else
  (void )theTitle;
  throw Aspect_WindowDefinitionError("Wayland_Window, Unable to create window - not implemented");
#endif
}

// =======================================================================
// function : IsMapped
// purpose  :
// =======================================================================
Standard_Boolean Wayland_Window::IsMapped() const
{
  if (!myWlEglWindow)
    return false;
  else if (IsVirtual())
    return true;

#if defined(HAVE_WAYLAND)
  return (bool)myWlXdgTop;
#else
  return Standard_False;
#endif
}

// =======================================================================
// function : Map
// purpose  :
// =======================================================================
void Wayland_Window::Map() const
{
  if (IsVirtual() || !myWlEglWindow)
    return;

#if defined(HAVE_WAYLAND)
  if (myWlXdgTop)
    return;

  Wayland_Window* aThis = const_cast<Wayland_Window*>(this);
  aThis->myWlXdgSurf.reset(xdg_wm_base_get_xdg_surface(myDisplay->GetXDGWMBase(), myWlSurface.get()),
                           [](xdg_surface* theXdgSurf) { theXdgSurf != nullptr ? xdg_surface_destroy(theXdgSurf) : (void)theXdgSurf; });

  static const xdg_surface_listener anXdgSurfList =
  {
    .configure = [](void* , struct xdg_surface* theXdgSurf, uint32_t theSerial)
    {
      xdg_surface_ack_configure(theXdgSurf, theSerial); // just confirm existence to the compositor
    },
  };
  xdg_surface_add_listener(aThis->myWlXdgSurf.get(), &anXdgSurfList, aThis);

  aThis->myWlXdgTop.reset(xdg_surface_get_toplevel(myWlXdgSurf.get()),
                          [](xdg_toplevel* theTop) { theTop != nullptr ? xdg_toplevel_destroy(theTop) : (void)theTop; });
  xdg_toplevel_set_app_id(aThis->myWlXdgTop.get(), myAppId.ToCString());
  xdg_toplevel_set_title(aThis->myWlXdgTop.get(), myTitle.ToCString());

  static const xdg_toplevel_listener anXdgTopList = getXdgTopLevelListener();
  xdg_toplevel_add_listener(aThis->myWlXdgTop.get(), &anXdgTopList, aThis);

  wl_surface_commit(myWlSurface.get());
#endif
}

// =======================================================================
// function : Unmap
// purpose  :
// =======================================================================
void Wayland_Window::Unmap() const
{
  if (IsVirtual() || !myWlEglWindow)
    return;

#if defined(HAVE_WAYLAND)
  if (!myWlXdgTop)
    return;

  Wayland_Window* aThis = const_cast<Wayland_Window*>(this);
  aThis->myWlXdgTop.reset();
  aThis->myWlXdgSurf.reset();
  wl_surface_attach(aThis->myWlSurface.get(), nullptr, 0, 0);
  wl_surface_commit(aThis->myWlSurface.get());
#endif
}

// =======================================================================
// function : DoResize
// purpose  :
// =======================================================================
Aspect_TypeOfResize Wayland_Window::DoResize()
{
  if (IsVirtual() || !myWlEglWindow)
    return Aspect_TOR_UNKNOWN;

  return Aspect_TOR_UNKNOWN;
}

// =======================================================================
// function : SetTitle
// purpose  :
// =======================================================================
void Wayland_Window::SetTitle (const TCollection_AsciiString& theTitle)
{
  myTitle = theTitle;
#if defined(HAVE_WAYLAND)
  if (myWlXdgTop)
    xdg_toplevel_set_title(myWlXdgTop.get(), theTitle.ToCString());
#endif
}

// =======================================================================
// function : DisplayConnection
// purpose  :
// =======================================================================
const Handle(Aspect_DisplayConnection)& Wayland_Window::DisplayConnection() const
{
  return myDisplay;
}

// =======================================================================
// function : InvalidateContent
// purpose  :
// =======================================================================
void Wayland_Window::InvalidateContent (const Handle(Aspect_DisplayConnection)& theDisp)
{
  if (!myWlEglWindow)
    return;

  (void )theDisp;
}

// =======================================================================
// function : VirtualKeyFromNative
// purpose  :
// =======================================================================
Aspect_VKey Wayland_Window::VirtualKeyFromNative (unsigned int theKey)
{
#if defined(HAVE_WAYLAND)
  switch(theKey)
  {
    // letters
    case KEY_A: return Aspect_VKey_A;
    case KEY_B: return Aspect_VKey_B;
    case KEY_C: return Aspect_VKey_C;
    case KEY_D: return Aspect_VKey_D;
    case KEY_E: return Aspect_VKey_E;
    case KEY_F: return Aspect_VKey_F;
    case KEY_G: return Aspect_VKey_G;
    case KEY_H: return Aspect_VKey_H;
    case KEY_I: return Aspect_VKey_I;
    case KEY_J: return Aspect_VKey_J;
    case KEY_K: return Aspect_VKey_K;
    case KEY_L: return Aspect_VKey_L;
    case KEY_M: return Aspect_VKey_M;
    case KEY_N: return Aspect_VKey_N;
    case KEY_O: return Aspect_VKey_O;
    case KEY_P: return Aspect_VKey_P;
    case KEY_Q: return Aspect_VKey_Q;
    case KEY_R: return Aspect_VKey_R;
    case KEY_S: return Aspect_VKey_S;
    case KEY_T: return Aspect_VKey_T;
    case KEY_U: return Aspect_VKey_U;
    case KEY_V: return Aspect_VKey_V;
    case KEY_W: return Aspect_VKey_W;
    case KEY_X: return Aspect_VKey_X;
    case KEY_Y: return Aspect_VKey_Y;
    case KEY_Z: return Aspect_VKey_Z;
    // numbers
    case KEY_0: return Aspect_VKey_0;
    case KEY_1: return Aspect_VKey_1;
    case KEY_2: return Aspect_VKey_2;
    case KEY_3: return Aspect_VKey_3;
    case KEY_4: return Aspect_VKey_4;
    case KEY_5: return Aspect_VKey_5;
    case KEY_6: return Aspect_VKey_6;
    case KEY_7: return Aspect_VKey_7;
    case KEY_8: return Aspect_VKey_8;
    case KEY_9: return Aspect_VKey_9;
    // Functional keys
    case KEY_F1: return Aspect_VKey_F1;
    case KEY_F2: return Aspect_VKey_F2;
    case KEY_F3: return Aspect_VKey_F3;
    case KEY_F4: return Aspect_VKey_F4;
    case KEY_F5: return Aspect_VKey_F5;
    case KEY_F6: return Aspect_VKey_F6;
    case KEY_F7: return Aspect_VKey_F7;
    case KEY_F8: return Aspect_VKey_F8;
    case KEY_F9: return Aspect_VKey_F9;
    case KEY_F10: return Aspect_VKey_F10;
    case KEY_F11: return Aspect_VKey_F11;
    case KEY_F12: return Aspect_VKey_F12;
    // common key
    case KEY_UP: return Aspect_VKey_Up;
    case KEY_DOWN: return Aspect_VKey_Down;
    case KEY_LEFT: return Aspect_VKey_Left;
    case KEY_RIGHT: return Aspect_VKey_Right;
    //case ?: return Aspect_VKey_Plus;         //!< '+'
    case KEY_MINUS: return Aspect_VKey_Minus;
    case KEY_EQUAL: return Aspect_VKey_Equal;
    case KEY_PAGEUP: return Aspect_VKey_PageUp;
    case KEY_PAGEDOWN: return Aspect_VKey_PageDown;
    case KEY_HOME: return Aspect_VKey_Home;
    case KEY_END: return Aspect_VKey_End;
    case KEY_ESC: return Aspect_VKey_Escape;
    //case ?: return Aspect_VKey_Back,
    case KEY_ENTER: return Aspect_VKey_Enter;
    case KEY_BACKSPACE: return Aspect_VKey_Backspace;
    case KEY_SPACE: return Aspect_VKey_Space;
    case KEY_DELETE: return Aspect_VKey_Delete;
    //case KEY_INSERT: return Aspect_VKey_Insert;
    case KEY_GRAVE: return Aspect_VKey_Tilde;
    case KEY_TAB: return Aspect_VKey_Tab;
    case KEY_COMMA: return Aspect_VKey_Comma;
    case KEY_DOT: return Aspect_VKey_Period;
    case KEY_SEMICOLON: return Aspect_VKey_Semicolon;
    case KEY_SLASH: return Aspect_VKey_Slash;
    case KEY_LEFTBRACE: return Aspect_VKey_BracketLeft;
    case KEY_BACKSLASH: return Aspect_VKey_Backslash;
    case KEY_RIGHTBRACE: return Aspect_VKey_BracketRight;
    case KEY_APOSTROPHE: return Aspect_VKey_Apostrophe;
    case KEY_NUMLOCK: return Aspect_VKey_Numlock;
    case KEY_SCROLLLOCK: return Aspect_VKey_Scroll;
    // keypad
    case KEY_KP0: return Aspect_VKey_Numpad0;
    case KEY_KP1: return Aspect_VKey_Numpad1;
    case KEY_KP2: return Aspect_VKey_Numpad2;
    case KEY_KP3: return Aspect_VKey_Numpad3;
    case KEY_KP4: return Aspect_VKey_Numpad4;
    case KEY_KP5: return Aspect_VKey_Numpad5;
    case KEY_KP6: return Aspect_VKey_Numpad6;
    case KEY_KP7: return Aspect_VKey_Numpad7;
    case KEY_KP8: return Aspect_VKey_Numpad8;
    case KEY_KP9: return Aspect_VKey_Numpad9;
    case KEY_KPASTERISK: return Aspect_VKey_NumpadMultiply;
    case KEY_KPPLUS: return Aspect_VKey_NumpadAdd;
    case KEY_KPMINUS: return Aspect_VKey_NumpadSubtract;
    case KEY_KPSLASH: return Aspect_VKey_NumpadDivide;
    //
    case KEY_KPDOT: return Aspect_VKey_Period;
    case KEY_KPEQUAL: return Aspect_VKey_Equal;
    case KEY_KPENTER: return Aspect_VKey_Enter;
    // modifiers
    case KEY_LEFTSHIFT:
    case KEY_RIGHTSHIFT:
      return Aspect_VKey_Shift;
    case KEY_LEFTCTRL:
    case KEY_RIGHTCTRL:
      return Aspect_VKey_Control;
    case KEY_LEFTALT:
    case KEY_RIGHTALT:
      return Aspect_VKey_Alt;
    case KEY_MENU:
    case KEY_COMPOSE:
      return Aspect_VKey_Menu;
    case KEY_LEFTMETA:
    case KEY_RIGHTMETA:
      return Aspect_VKey_Meta;
  }
  return Aspect_VKey_UNKNOWN;
#else
  (void)theKey;
  return Aspect_VKey_UNKNOWN;
#endif
}

// compile sources generated from xdg-shell.xml
#if defined(HAVE_WAYLAND)
#if defined(__clang__)
  #pragma clang diagnostic ignored "-Wattributes"
#elif defined(__GNUC__)
  #pragma GCC diagnostic ignored "-Wattributes"
#endif
#include "xdg-shell-client-protocol-impl.pxx"
#endif
