// Created on: 2014-05-22
// Created by: Varvara POSKONINA
// Copyright (c) 2005-2014 OPEN CASCADE SAS
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

#include <SelectMgr_SelectingVolumeManager.hxx>

#include <Graphic3d_SequenceOfHClipPlane.hxx>
#include <SelectMgr_AxisIntersector.hxx>
#include <SelectMgr_RectangularFrustum.hxx>
#include <SelectMgr_TriangularFrustumSet.hxx>

#include <BVH_Tools.hxx>
#include <Standard_Dump.hxx>

//=======================================================================
// function : SelectMgr_SelectingVolumeManager
// purpose  : Creates instances of all available selecting volume types
//=======================================================================
SelectMgr_SelectingVolumeManager::SelectMgr_SelectingVolumeManager()
: myActiveSelectingVolume (NULL),
  myToAllowOverlap (Standard_False)
{
}

//=======================================================================
// function : ~SelectMgr_SelectingVolumeManager
// purpose  :
//=======================================================================
SelectMgr_SelectingVolumeManager::~SelectMgr_SelectingVolumeManager()
{
  //
}

//=======================================================================
// function : ScaleAndTransform
// purpose  : IMPORTANT: Scaling makes sense only for frustum built on a single point!
//            Note that this method does not perform any checks on type of the frustum.
//
//            Returns a copy of the frustum resized according to the scale factor given
//            and transforms it using the matrix given.
//            There are no default parameters, but in case if:
//                - transformation only is needed: @theScaleFactor must be initialized
//                  as any negative value;
//                - scale only is needed: @theTrsf must be set to gp_Identity.
//            Builder is an optional argument that represents corresponding settings for
//            re-constructing transformed frustum from scratch. Can be null if reconstruction
//            is not needed furthermore in the code.
//=======================================================================
SelectMgr_SelectingVolumeManager SelectMgr_SelectingVolumeManager::ScaleAndTransform (const Standard_Integer theScaleFactor,
                                                                                      const gp_GTrsf& theTrsf,
                                                                                      const Handle(SelectMgr_FrustumBuilder)& theBuilder) const
{
  SelectMgr_SelectingVolumeManager aMgr;
  if (myActiveSelectingVolume.IsNull())
  {
    return aMgr;
  }

  aMgr.myActiveSelectingVolume = myActiveSelectingVolume->ScaleAndTransform (theScaleFactor, theTrsf, theBuilder);
  aMgr.myToAllowOverlap = myToAllowOverlap;
  aMgr.myViewClipPlanes = myViewClipPlanes;
  aMgr.myObjectClipPlanes = myObjectClipPlanes;
  aMgr.myViewClipRange = myViewClipRange;

  return aMgr;
}

//=======================================================================
// function : Camera
// purpose  :
//=======================================================================
const Handle(Graphic3d_Camera)& SelectMgr_SelectingVolumeManager::Camera() const
{
  if (myActiveSelectingVolume.IsNull())
  {
    static const Handle(Graphic3d_Camera) anEmptyCamera;
    return anEmptyCamera;
  }
  return myActiveSelectingVolume->Camera();
}

//=======================================================================
// function : SetCamera
// purpose  :
//=======================================================================
void SelectMgr_SelectingVolumeManager::SetCamera (const Handle(Graphic3d_Camera)& theCamera)
{
  Standard_ASSERT_RAISE(!myActiveSelectingVolume.IsNull(),
    "SelectMgr_SelectingVolumeManager::SetCamera() should be called after initialization of selection volume ");
  myActiveSelectingVolume->SetCamera (theCamera);
}

//=======================================================================
// function : SetWindowSize
// purpose  : Updates window size in all selecting volumes
//=======================================================================
void SelectMgr_SelectingVolumeManager::SetWindowSize (const Standard_Integer theWidth,
                                                      const Standard_Integer theHeight)
{
  Standard_ASSERT_RAISE(!myActiveSelectingVolume.IsNull(),
    "SelectMgr_SelectingVolumeManager::SetWindowSize() should be called after initialization of selection volume ");
  myActiveSelectingVolume->SetWindowSize (theWidth, theHeight);
}

//=======================================================================
// function : SetCamera
// purpose  : Updates viewport in all selecting volumes
//=======================================================================
void SelectMgr_SelectingVolumeManager::SetViewport (const Standard_Real theX,
                                                    const Standard_Real theY,
                                                    const Standard_Real theWidth,
                                                    const Standard_Real theHeight)
{
  Standard_ASSERT_RAISE(!myActiveSelectingVolume.IsNull(),
    "SelectMgr_SelectingVolumeManager::SetViewport() should be called after initialization of selection volume ");
  myActiveSelectingVolume->SetViewport (theX, theY, theWidth, theHeight);
}

//=======================================================================
// function : SetPixelTolerance
// purpose  : Updates pixel tolerance in all selecting volumes
//=======================================================================
void SelectMgr_SelectingVolumeManager::SetPixelTolerance (const Standard_Integer theTolerance)
{
  Standard_ASSERT_RAISE(!myActiveSelectingVolume.IsNull(),
    "SelectMgr_SelectingVolumeManager::SetPixelTolerance() should be called after initialization of selection volume ");
  myActiveSelectingVolume->SetPixelTolerance (theTolerance);
}

//=======================================================================
// function : InitPointSelectingVolume
// purpose  :
//=======================================================================
void SelectMgr_SelectingVolumeManager::InitPointSelectingVolume (const gp_Pnt2d& thePoint)
{
  Handle(SelectMgr_RectangularFrustum) aPntVolume = Handle(SelectMgr_RectangularFrustum)::DownCast(myActiveSelectingVolume);
  if (aPntVolume.IsNull())
  {
    aPntVolume = new SelectMgr_RectangularFrustum();
  }
  aPntVolume->Init (thePoint);
  myActiveSelectingVolume = aPntVolume;
}

//=======================================================================
// function : InitBoxSelectingVolume
// purpose  :
//=======================================================================
void SelectMgr_SelectingVolumeManager::InitBoxSelectingVolume (const gp_Pnt2d& theMinPt,
                                                               const gp_Pnt2d& theMaxPt)
{
  Handle(SelectMgr_RectangularFrustum) aBoxVolume = Handle(SelectMgr_RectangularFrustum)::DownCast(myActiveSelectingVolume);
  if (aBoxVolume.IsNull())
  {
    aBoxVolume = new SelectMgr_RectangularFrustum();
  }
  aBoxVolume->Init (theMinPt, theMaxPt);
  myActiveSelectingVolume = aBoxVolume;
}

//=======================================================================
// function : InitPolylineSelectingVolume
// purpose  :
//=======================================================================
void SelectMgr_SelectingVolumeManager::InitPolylineSelectingVolume (const TColgp_Array1OfPnt2d& thePoints)
{
  Handle(SelectMgr_TriangularFrustumSet) aPolylineVolume = Handle(SelectMgr_TriangularFrustumSet)::DownCast(myActiveSelectingVolume);
  if (aPolylineVolume.IsNull())
  {
    aPolylineVolume = new SelectMgr_TriangularFrustumSet();
  }
  aPolylineVolume->Init (thePoints);
  myActiveSelectingVolume = aPolylineVolume;
  aPolylineVolume->SetAllowOverlapDetection (IsOverlapAllowed());
}

//=======================================================================
// function : InitAxisSelectingVolume
// purpose  :
//=======================================================================
void SelectMgr_SelectingVolumeManager::InitAxisSelectingVolume (const gp_Ax1& theAxis)
{
  Handle(SelectMgr_AxisIntersector) anAxisVolume = Handle(SelectMgr_AxisIntersector)::DownCast(myActiveSelectingVolume);
  if (anAxisVolume.IsNull())
  {
    anAxisVolume = new SelectMgr_AxisIntersector();
  }
  anAxisVolume->Init (theAxis);
  myActiveSelectingVolume = anAxisVolume;
}

//=======================================================================
// function : BuildSelectingVolume
// purpose  :
//=======================================================================
void SelectMgr_SelectingVolumeManager::BuildSelectingVolume()
{
  Standard_ASSERT_RAISE (!myActiveSelectingVolume.IsNull(),
    "SelectMgr_SelectingVolumeManager::BuildSelectingVolume() should be called after initialization of active selection volume.");
  myActiveSelectingVolume->Build();
}

// =======================================================================
// function : DetectedPoint
// purpose  : Calculates the point on a view ray that was detected during
//            the run of selection algo by given depth. Is valid for point
//            selection only
// =======================================================================
gp_Pnt SelectMgr_SelectingVolumeManager::DetectedPoint (const Standard_Real theDepth) const
{
  Standard_ASSERT_RAISE(!myActiveSelectingVolume.IsNull(),
    "SelectMgr_SelectingVolumeManager::DetectedPoint() should be called after initialization of selection volume");
  return myActiveSelectingVolume->DetectedPoint (theDepth);
}

//=======================================================================
// function : GetVertices
// purpose  :
//=======================================================================
const gp_Pnt* SelectMgr_SelectingVolumeManager::GetVertices() const
{
  if (myActiveSelectingVolume.IsNull())
  {
    return NULL;
  }
  const SelectMgr_RectangularFrustum* aRectFrustum =
    static_cast<const SelectMgr_RectangularFrustum*> (myActiveSelectingVolume.get());
  if (aRectFrustum == NULL)
  {
    return NULL;
  }
  return aRectFrustum->GetVertices();
}

//=======================================================================
// function : GetPlanes
// purpose  :
//=======================================================================
void SelectMgr_SelectingVolumeManager::GetPlanes (NCollection_Vector<SelectMgr_Vec4>& thePlaneEquations) const
{
  if (myActiveSelectingVolume.IsNull())
  {
    thePlaneEquations.Clear();
    return;
  }
  return myActiveSelectingVolume->GetPlanes (thePlaneEquations);
}

//=======================================================================
// function : SetViewClipping
// purpose  :
//=======================================================================
void SelectMgr_SelectingVolumeManager::SetViewClipping (const Handle(Graphic3d_SequenceOfHClipPlane)& theViewPlanes,
                                                        const Handle(Graphic3d_SequenceOfHClipPlane)& theObjPlanes,
                                                        const SelectMgr_SelectingVolumeManager* theWorldSelMgr)
{
  myViewClipPlanes   = theViewPlanes;
  myObjectClipPlanes = theObjPlanes;
  if (GetActiveSelectionType() != SelectMgr_SelectionType_Point)
  {
    return;
  }

  const SelectMgr_SelectingVolumeManager* aWorldSelMgr = theWorldSelMgr != NULL ? theWorldSelMgr : this;
  myViewClipRange.SetVoid();
  if (!theViewPlanes.IsNull()
   && !theViewPlanes->IsEmpty())
  {
    myViewClipRange.AddClippingPlanes (*theViewPlanes,
      gp_Ax1(aWorldSelMgr->myActiveSelectingVolume->GetNearPnt(),
             aWorldSelMgr->myActiveSelectingVolume->GetViewRayDirection()));
  }
  if (!theObjPlanes.IsNull()
   && !theObjPlanes->IsEmpty())
  {
    myViewClipRange.AddClippingPlanes (*theObjPlanes,
      gp_Ax1(aWorldSelMgr->myActiveSelectingVolume->GetNearPnt(),
             aWorldSelMgr->myActiveSelectingVolume->GetViewRayDirection()));
  }
}

//=======================================================================
// function : SetViewClipping
// purpose  :
//=======================================================================
void SelectMgr_SelectingVolumeManager::SetViewClipping (const SelectMgr_SelectingVolumeManager& theOther)
{
  myViewClipPlanes   = theOther.myViewClipPlanes;
  myObjectClipPlanes = theOther.myObjectClipPlanes;
  myViewClipRange    = theOther.myViewClipRange;
}

//=======================================================================
//function : DumpJson
//purpose  : 
//=======================================================================
void SelectMgr_SelectingVolumeManager::DumpJson (Standard_OStream& theOStream, Standard_Integer theDepth) const 
{
  OCCT_DUMP_CLASS_BEGIN (theOStream, SelectMgr_SelectingVolumeManager)

  OCCT_DUMP_FIELD_VALUE_POINTER (theOStream, myActiveSelectingVolume.get())
  OCCT_DUMP_FIELD_VALUE_POINTER (theOStream, myViewClipPlanes.get())
  OCCT_DUMP_FIELD_VALUE_POINTER (theOStream, myObjectClipPlanes.get())

  OCCT_DUMP_FIELD_VALUES_DUMPED (theOStream, theDepth, &myViewClipRange)
  OCCT_DUMP_FIELD_VALUE_NUMERICAL (theOStream, myToAllowOverlap)
}
