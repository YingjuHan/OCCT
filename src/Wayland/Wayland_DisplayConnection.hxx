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

#ifndef Wayland_DisplayConnection_HeaderFile
#define Wayland_DisplayConnection_HeaderFile

#include <Aspect_DisplayConnection.hxx>

#include <vector>

struct wl_compositor;
struct wl_cursor_image;
struct wl_cursor_theme;
struct wl_display;
struct wl_keyboard;
struct wl_registry;
struct wl_output;
struct wl_pointer;
struct wl_seat;
struct wl_shm;
struct wl_surface;
struct xdg_wm_base;

//! This class creates and provides connection with Wayland server.
//! WARRNING: Do not close display connection manually!
class Wayland_DisplayConnection : public Aspect_DisplayConnection
{
  DEFINE_STANDARD_RTTIEXT(Wayland_DisplayConnection, Aspect_DisplayConnection)
public:

  //! Default constructor. Creates connection with default server.
  Standard_EXPORT Wayland_DisplayConnection();

  //! Constructor. Creates connection with specified display.
  Standard_EXPORT Wayland_DisplayConnection(const TCollection_AsciiString& theDisplayName);

  //! Constructor wrapping existing wl_display instance.
  //! WARNING! it is a responsibility of application to keep this pointer
  //! valid while Aspect_DisplayConnection is alive and to close wl_display when it is no more needed.
  Standard_EXPORT Wayland_DisplayConnection(Aspect_XDisplay* theDisplay);

  //! Destructor. Close opened connection.
  Standard_EXPORT virtual ~Wayland_DisplayConnection();

  //! @return pointer to Display structure that serves as the connection to the Wayland server.
  virtual Aspect_XDisplay* GetDisplayAspect() Standard_OVERRIDE { return (Aspect_XDisplay* )myWlDisplay.get(); }

  //! @return TRUE if X Display has been allocated by this class
  Standard_Boolean IsOwnDisplay() const { return myIsOwnDisplay; }

  //! Open connection with display or takes theDisplay parameter when it is not NULL.
  //! WARNING! When external Display is specified, it is a responsibility of application
  //! to keep this pointer valid while Aspect_DisplayConnection is alive
  //! and to close Display when it is no more needed.
  //! @param theDisplay external pointer to allocated wl_display, or NULL if new connection should be created
  Standard_EXPORT void Init(Aspect_XDisplay* theDisplay,
                            const TCollection_AsciiString& theDisplayName = TCollection_AsciiString());

  //! Close connection.
  Standard_EXPORT void Close();

  //! Return file description for this connection.
  Standard_EXPORT virtual int FileDescriptor() const Standard_OVERRIDE;

  //! Flushes output buffer (wl_display_dispatch()).
  Standard_EXPORT int Flush() Standard_OVERRIDE;

  //! wl_display_dispatch_pending() wrapper.
  Standard_EXPORT int DispatchPending();

public:

  //! Return native display.
  wl_display* GetWlDisplay() { return myWlDisplay.get(); }

  //! Return native compositor.
  wl_compositor* GetWlCompositor() { return myWlCompositor; }

  //! Return shared memory manager.
  wl_shm* GetWlSharedMemory() { return myWlShm; }

  //! Return native seat.
  wl_seat* GetWlSeat() { return myWlSeat; }

  //! Return native pointer.
  wl_pointer* GetWlPointer() { return myWlPointer.get(); }

  //! Return native keyboard.
  wl_keyboard* GetWlKeyboard() { return myWlKeyboard.get(); }

  //! Return native XDG WM base.
  xdg_wm_base* GetXDGWMBase() { return myWlXdgWmBase; }

  //! Return cached scale of specified output.
  int32_t GetOutputScale(const wl_output* theOutput) const
  {
    for (const WlOutputInfo& anOutput : myWlOutputs)
    {
      if (anOutput.Output == theOutput)
        return anOutput.Scale;
    }
    return 1;
  }

  //! Return first output.
  wl_output* GetFirstOutput() const { return !myWlOutputs.empty() ? myWlOutputs[0].Output : nullptr; }

  //! Load and sets main cursor surface.
  Standard_EXPORT void SetWlMainCursor(wl_pointer* thePointer, uint32_t theSerial, double theScale);

private:

  std::shared_ptr<wl_display> myWlDisplay;

  std::shared_ptr<wl_pointer>  myWlPointer;
  std::shared_ptr<wl_keyboard> myWlKeyboard;

  std::shared_ptr<wl_cursor_theme> myWlCursorTheme;
  std::shared_ptr<wl_surface> myWlCursorMainSurf;
  double myWlCursorScale = 1.0;

  wl_compositor* myWlCompositor = nullptr;
  wl_shm*        myWlShm        = nullptr;
  wl_seat*       myWlSeat       = nullptr;
  xdg_wm_base*   myWlXdgWmBase  = nullptr;

  wl_cursor_image* myWlCursorMainImg = nullptr;

  struct WlOutputInfo
  {
    wl_output* Output = nullptr;
    int32_t    Scale  = 1;
  };
  std::vector<WlOutputInfo> myWlOutputs;

  bool myIsOwnDisplay = false;

private:

  //! To protect the connection from closing copying allowed only through the handles.
  Wayland_DisplayConnection            (const Wayland_DisplayConnection& ) Standard_DELETE;
  Wayland_DisplayConnection& operator= (const Wayland_DisplayConnection& ) Standard_DELETE;

};

#endif // Wayland_DisplayConnection_HeaderFile
