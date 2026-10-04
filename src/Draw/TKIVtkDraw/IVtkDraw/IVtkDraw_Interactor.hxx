// Created on: 2012-05-28
//
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

#ifndef _IVtkDraw_Interactor_HeaderFile
#define _IVtkDraw_Interactor_HeaderFile

#include <Standard.hxx>
#include <Standard_Macro.hxx>
#include <Standard_Transient.hxx>

#ifdef _WIN32
  #include <windows.h>
#elif defined(__APPLE__)
  #include <vtkCallbackCommand.h>
  #include <vtkCocoaRenderWindowInteractor.h>
#else
  #include <X11/X.h>
  #include <X11/Xlib.h>
  #include <X11/Xutil.h>
  #include <tcl.h>
#endif

// prevent disabling some MSVC warning messages by VTK headers
#include <Standard_WarningsDisable.hxx>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <Standard_WarningsRestore.hxx>

#include <IVtkTools_ShapePicker.hxx>
#include <IVtkDraw_HighlightAndSelectionPipeline.hxx>
#include <Aspect_Window.hxx>

//! Base Vtk interactor class.
//! On Apple platforms the Cocoa flavor is used, so that native events coming
//! from the vtkCocoaGLView are reused instead of being re-implemented;
//! other platforms keep using the generic base class.
#if defined(__APPLE__)
typedef vtkCocoaRenderWindowInteractor IVtkDraw_InteractorBase;
#else
typedef vtkRenderWindowInteractor IVtkDraw_InteractorBase;
#endif

class vtkWin32RenderWindowInteractor;
typedef vtkSmartPointer<IVtkTools_ShapePicker> PSelector;

class IVtkDraw_Interactor : public IVtkDraw_InteractorBase
{
public:
  static IVtkDraw_Interactor* New();

  vtkTypeMacro(IVtkDraw_Interactor, IVtkDraw_InteractorBase)

    void Initialize() override;
  void Enable() override;

  void Start() override {}

  const PSelector& Selector() const { return mySelector; }

  void                              SetShapePicker(const PSelector& theSelector);
  void                              SetPipelines(const occ::handle<ShapePipelineMap>& thePipelines);
  void                              SetOCCWindow(const occ::handle<Aspect_Window>& theWindow);
  const occ::handle<Aspect_Window>& GetOCCWindow() const;

  //! Process highlighting
  void MoveTo(int theX, int theY);

  //! Process selection
  void OnSelection();

  bool IsEnabled() const;

//! ViewerMainLoop() and the X11 display handle only exist on platforms
//! driving the event loop from an X display; the Cocoa flavor gets its
//! events from the vtkCocoaGLView instead.
#if !defined(_WIN32) && !defined(__APPLE__)
  Display* GetDisplayId() const;
  int      ViewerMainLoop(int theArgNum, const char** theArgs);
#endif

protected:
  IVtkDraw_Interactor();
  ~IVtkDraw_Interactor() override;

#ifdef _WIN32
  friend LRESULT CALLBACK WndProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam);
  friend LRESULT CALLBACK ViewerWindowProc(HWND                 hwnd,
                                           UINT                 Msg,
                                           WPARAM               wParam,
                                           LPARAM               lParam,
                                           IVtkDraw_Interactor* theInteractor);

  void OnMouseMove(HWND wnd, UINT nFlags, int X, int Y);
  void OnRButtonDown(HWND wnd, UINT nFlags, int X, int Y, int repeat = 0);
  void OnRButtonUp(HWND wnd, UINT nFlags, int X, int Y);
  void OnMButtonDown(HWND wnd, UINT nFlags, int X, int Y, int repeat = 0);
  void OnMButtonUp(HWND wnd, UINT nFlags, int X, int Y);
  void OnLButtonDown(HWND wnd, UINT nFlags, int X, int Y, int repeat = 0);
  void OnLButtonUp(HWND wnd, UINT nFlags, int X, int Y);
  void OnSize(HWND wnd, UINT nType, int X, int Y);
  void OnTimer(HWND wnd, UINT nIDEvent);
  void OnMouseWheelForward(HWND wnd, UINT nFlags, int X, int Y);
  void OnMouseWheelBackward(HWND wnd, UINT nFlags, int X, int Y);
#elif defined(__APPLE__)
  //! Observer callback used to inject IVtk highlighting/selection into the
  //! events dispatched by VTK.
  //! Note that the Cocoa backend of VTK invokes vtkRenderWindowInteractor::
  //! InvokeEvent() directly from vtkCocoaGLView (without going through the
  //! virtual LeftButtonPressEvent()/MouseMoveEvent() helpers), so overriding
  //! those helpers would have no effect - an observer is required instead.
  static void OnVtkCocoaEvent(vtkObject*    theCaller,
                              unsigned long theEventId,
                              void*         theClientData,
                              void*         theCallData);
#else
  static void ProcessEvents(ClientData theData, int);
  void        GetMousePosition(int* theX, int* theY) override;
#endif

private:
  // copying is prohibited
  IVtkDraw_Interactor(const IVtkDraw_Interactor&) = delete;
  void operator=(const IVtkDraw_Interactor&)      = delete;

private:
#if defined(_WIN32)
  HWND myWindowId;
  int  myMouseInWindow;
#elif defined(__APPLE__)
  bool myIsLeftButtonPressed;

  //! Command object forwarding Vtk events to OnVtkCocoaEvent().
  vtkSmartPointer<vtkCallbackCommand> myEventCommand;
#else
  Window   myWindowId;
  Display* myDisplayId;
  bool     myIsLeftButtonPressed;
#endif

  PSelector                     mySelector;
  occ::handle<ShapePipelineMap> myPipelines;
  occ::handle<Aspect_Window>    myWindow;
};

#endif
