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

#ifndef Wayland_Window_HeaderFile
#define Wayland_Window_HeaderFile

#include <Aspect_VKey.hxx>
#include <Aspect_Window.hxx>

#include <set>

class Wayland_DisplayConnection;
class Aspect_WindowInputListener;

struct wl_egl_window;
struct wl_keyboard;
struct wl_keyboard_listener;
struct wl_output;
struct wl_pointer;
struct wl_pointer_listener;
struct wl_region;
struct wl_surface;
struct wl_surface_listener;

struct xdg_surface;
struct xdg_toplevel;
struct xdg_toplevel_listener;

//! This class implements basic facilities for creating Wayland window suitable for 3D Viewer initialization.
//!
//! As Wayland is a low-level API, it is preferred using GUI framework for managing windows;
//! use Aspect_NeutralWindow class to wrap existing window
//! with Aspect_NeutralWindow::SetNativeHandle() set to wl_egl_window pointer.
class Wayland_Window : public Aspect_Window
{
  DEFINE_STANDARD_RTTIEXT(Wayland_Window, Aspect_Window)
public:

  //! Convert Wayland virtual key (linux/input-event-codes.h) into Aspect_VKey.
  Standard_EXPORT static Aspect_VKey VirtualKeyFromNative(unsigned int theKey);

public:

  //! Empty constructor.
  Standard_EXPORT Wayland_Window();

  //! Destroys the window and all resources attached to it.
  Standard_EXPORT ~Wayland_Window();

  //! Creates a Wayland window defined by size.
  Standard_EXPORT void Create(const Handle(Wayland_DisplayConnection)& theDisplay,
                              const Standard_CString theAppId,
                              const Standard_CString theTitle,
                              const Graphic3d_Vec2i& theSize);

  //! Destroys the window and all resources attached to it.
  Standard_EXPORT void Close();

  //! Returns connection to Display.
  Standard_EXPORT virtual const Handle(Aspect_DisplayConnection)& DisplayConnection() const Standard_OVERRIDE;

  //! Opens the window.
  Standard_EXPORT virtual void Map() const Standard_OVERRIDE;

  //! Closes the window.
  Standard_EXPORT virtual void Unmap() const Standard_OVERRIDE;

  //! Applies the resizing to the window.
  Standard_EXPORT virtual Aspect_TypeOfResize DoResize() Standard_OVERRIDE;

  //! Apply the mapping change to the window.
  virtual Standard_Boolean DoMapping() const Standard_OVERRIDE { return true; } // IsMapped()

  //! Returns True if the window is opened.
  Standard_EXPORT virtual Standard_Boolean IsMapped() const Standard_OVERRIDE;

  //! Returns The Window RATIO equal to the physical WIDTH/HEIGHT dimensions.
  virtual Standard_Real Ratio() const Standard_OVERRIDE
  {
    return Standard_Real(myLogicalSize.x()) / Standard_Real(myLogicalSize.y());
  }

  //! Returns The Window POSITION in PIXEL
  virtual void Position(Standard_Integer& theX1,
                        Standard_Integer& theY1,
                        Standard_Integer& theX2,
                        Standard_Integer& theY2) const Standard_OVERRIDE
  {
    theX1 = 0;
    theX2 = myBufferSize.x();
    theY1 = 0;
    theY2 = myBufferSize.y();
  }

  //! Returns The Window SIZE in PIXEL.
  virtual void Size(Standard_Integer& theWidth,
                    Standard_Integer& theHeight) const Standard_OVERRIDE
  {
    theWidth  = myBufferSize.x();
    theHeight = myBufferSize.y();
  }

  //! @return native window handle - pointer to wl_egl_window
  virtual Aspect_Drawable NativeHandle() const Standard_OVERRIDE
  {
    return (Aspect_Drawable)myWlEglWindow.get();
  }

  //! @return parent of native window handle.
  virtual Aspect_Drawable NativeParentHandle() const Standard_OVERRIDE { return 0; }

  //! @return native Window FB config
  virtual Aspect_FBConfig NativeFBConfig() const Standard_OVERRIDE { return 0; }

  //! Sets window title.
  Standard_EXPORT virtual void SetTitle(const TCollection_AsciiString& theTitle) Standard_OVERRIDE;

  //! Invalidate entire window content through generation of Expose event.
  Standard_EXPORT virtual void InvalidateContent(const Handle(Aspect_DisplayConnection)& theDisp) Standard_OVERRIDE;

  //! Return device pixel ratio (logical to backing store scale factor).
  virtual Standard_Real DevicePixelRatio() const Standard_OVERRIDE { return myDevicePixelRatio; }

  //! Return flag to ignore DPI within DevicePixelRatio(); FALSE by default.
  bool ToIgnoreDpi() const { return myIsDpiUnaware; }

  //! Set flag to ignore DPI within DevicePixelRatio().
  void SetIgnoreDpi(bool theToIgnore) { myIsDpiUnaware = theToIgnore; }

private:

  //! Fill in surface listener callbacks.
  static wl_surface_listener getSurfaceListener();

  //! Fill in XDG top-level listener callbacks.
  static xdg_toplevel_listener getXdgTopLevelListener();

  //! Fill in pointer listener callbacks.
  static wl_pointer_listener getPointerListener();

  //! Fill in keyboard listener callbacks.
  static wl_keyboard_listener getKeyboardListener();

  //! Perform resizing of buffer.
  void performBufferResize(int32_t theWidth, int32_t theHeight);

protected:

  Handle(Wayland_DisplayConnection) myDisplay;

  TCollection_AsciiString myAppId;
  TCollection_AsciiString myTitle;
  Graphic3d_Vec2i  myLogicalSize;
  Graphic3d_Vec2i  myBufferSize;
  double           myDevicePixelRatio = 1.0;
  Aspect_VKeyFlags myKeyModifiers = Aspect_VKeyFlags_NONE;
  bool myIsDpiUnaware = false;
  bool myIsPointerIn = false;
  bool myIsKeyboardIn = false;

  std::shared_ptr<wl_surface>    myWlSurface;
  std::shared_ptr<xdg_surface>   myWlXdgSurf;
  std::shared_ptr<xdg_toplevel>  myWlXdgTop;
  std::shared_ptr<wl_egl_window> myWlEglWindow;

  std::shared_ptr<wl_pointer>  myWlPointer;
  std::shared_ptr<wl_keyboard> myWlKeyboard;

  std::set<wl_output*> myWlOutputs;
  wl_output* myWlActiveOutput = nullptr;
};

#endif // Wayland_Window_HeaderFile
