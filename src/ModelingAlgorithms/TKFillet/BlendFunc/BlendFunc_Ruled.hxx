// Created on: 1993-12-02
// Created by: Jacques GOUSSARD
// Copyright (c) 1993-1999 Matra Datavision
// Copyright (c) 1999-2014 OPEN CASCADE SAS
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

#ifndef _BlendFunc_Ruled_HeaderFile
#define _BlendFunc_Ruled_HeaderFile

#include <Adaptor3d_Surface.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <gp_Vec2d.hxx>
#include <Blend_Function.hxx>
#include <math_Vector.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TColgp_Array1OfVec.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <GeomAbs_Shape.hxx>
#include <TColStd_Array1OfInteger.hxx>
#include <TColgp_Array1OfPnt2d.hxx>
#include <TColgp_Array1OfVec2d.hxx>

class math_Matrix;
class Blend_Point;
class gp_Ax1;

class BlendFunc_Ruled : public Blend_Function
{
public:
  DEFINE_STANDARD_ALLOC

  Standard_EXPORT BlendFunc_Ruled(const Handle(Adaptor3d_Surface)& S1,
                                  const Handle(Adaptor3d_Surface)& S2,
                                  const Handle(Adaptor3d_Curve)&   C);

  //! returns the number of equations of the function.
  Standard_EXPORT Standard_Integer NbEquations() const Standard_OVERRIDE;

  //! computes the values <F> of the Functions for the
  //! variable <X>.
  //! Returns True if the computation was done successfully,
  //! False otherwise.
  Standard_EXPORT Standard_Boolean Value(const math_Vector& X, math_Vector& F) Standard_OVERRIDE;

  //! returns the values <D> of the derivatives for the
  //! variable <X>.
  //! Returns True if the computation was done successfully,
  //! False otherwise.
  Standard_EXPORT Standard_Boolean Derivatives(const math_Vector& X,
                                               math_Matrix&       D) Standard_OVERRIDE;

  //! returns the values <F> of the functions and the derivatives
  //! <D> for the variable <X>.
  //! Returns True if the computation was done successfully,
  //! False otherwise.
  Standard_EXPORT Standard_Boolean Values(const math_Vector& X,
                                          math_Vector&       F,
                                          math_Matrix&       D) Standard_OVERRIDE;

  Standard_EXPORT void Set(const Standard_Real Param) Standard_OVERRIDE;

  Standard_EXPORT void Set(const Standard_Real First, const Standard_Real Last) Standard_OVERRIDE;

  Standard_EXPORT void GetTolerance(math_Vector&        Tolerance,
                                    const Standard_Real Tol) const Standard_OVERRIDE;

  Standard_EXPORT void GetBounds(math_Vector& InfBound,
                                 math_Vector& SupBound) const Standard_OVERRIDE;

  Standard_EXPORT Standard_Boolean IsSolution(const math_Vector&  Sol,
                                              const Standard_Real Tol) Standard_OVERRIDE;

  //! Returns   the    minimal  Distance  between   two
  //! extremities of calculated sections.
  Standard_EXPORT Standard_Real GetMinimalDistance() const Standard_OVERRIDE;

  Standard_EXPORT const gp_Pnt& PointOnS1() const Standard_OVERRIDE;

  Standard_EXPORT const gp_Pnt& PointOnS2() const Standard_OVERRIDE;

  Standard_EXPORT Standard_Boolean IsTangencyPoint() const Standard_OVERRIDE;

  Standard_EXPORT const gp_Vec& TangentOnS1() const Standard_OVERRIDE;

  Standard_EXPORT const gp_Vec2d& Tangent2dOnS1() const Standard_OVERRIDE;

  Standard_EXPORT const gp_Vec& TangentOnS2() const Standard_OVERRIDE;

  Standard_EXPORT const gp_Vec2d& Tangent2dOnS2() const Standard_OVERRIDE;

  //! Returns the tangent vector at the section,
  //! at the beginning and the end of the section, and
  //! returns the normal (of the surfaces) at
  //! these points.
  Standard_EXPORT void Tangent(const Standard_Real U1,
                               const Standard_Real V1,
                               const Standard_Real U2,
                               const Standard_Real V2,
                               gp_Vec&             TgFirst,
                               gp_Vec&             TgLast,
                               gp_Vec&             NormFirst,
                               gp_Vec&             NormLast) const Standard_OVERRIDE;

  Standard_EXPORT Standard_Boolean GetSection(const Standard_Real Param,
                                              const Standard_Real U1,
                                              const Standard_Real V1,
                                              const Standard_Real U2,
                                              const Standard_Real V2,
                                              TColgp_Array1OfPnt& tabP,
                                              TColgp_Array1OfVec& tabV);

  //! Returns False
  Standard_EXPORT Standard_Boolean IsRational() const Standard_OVERRIDE;

  //! Returns the length of the maximum section
  Standard_EXPORT Standard_Real GetSectionSize() const Standard_OVERRIDE;

  //! Compute the minimal value of weight for each poles
  //! of all sections.
  Standard_EXPORT void GetMinimalWeight(TColStd_Array1OfReal& Weigths) const Standard_OVERRIDE;

  //! Returns  the number  of  intervals for  continuity
  //! <S>. May be one if Continuity(me) >= <S>
  Standard_EXPORT Standard_Integer NbIntervals(const GeomAbs_Shape S) const Standard_OVERRIDE;

  //! Stores in <T> the  parameters bounding the intervals
  //! of continuity <S>.
  //!
  //! The array must provide  enough room to  accommodate
  //! for the parameters. i.e. T.Length() > NbIntervals()
  //! raises
  //! OutOfRange from Standard
  Standard_EXPORT void Intervals(TColStd_Array1OfReal& T,
                                 const GeomAbs_Shape   S) const Standard_OVERRIDE;

  Standard_EXPORT void GetShape(Standard_Integer& NbPoles,
                                Standard_Integer& NbKnots,
                                Standard_Integer& Degree,
                                Standard_Integer& NbPoles2d) Standard_OVERRIDE;

  //! Returns the tolerance to reach in approximation
  //! to respect
  //! BoundTol error at the Boundary
  //! AngleTol tangent error at the Boundary
  //! SurfTol error inside the surface.
  Standard_EXPORT void GetTolerance(const Standard_Real BoundTol,
                                    const Standard_Real SurfTol,
                                    const Standard_Real AngleTol,
                                    math_Vector&        Tol3d,
                                    math_Vector&        Tol1D) const Standard_OVERRIDE;

  Standard_EXPORT void Knots(TColStd_Array1OfReal& TKnots) Standard_OVERRIDE;

  Standard_EXPORT void Mults(TColStd_Array1OfInteger& TMults) Standard_OVERRIDE;

  //! Used for the first and last section
  Standard_EXPORT Standard_Boolean Section(const Blend_Point&    P,
                                           TColgp_Array1OfPnt&   Poles,
                                           TColgp_Array1OfVec&   DPoles,
                                           TColgp_Array1OfVec&   D2Poles,
                                           TColgp_Array1OfPnt2d& Poles2d,
                                           TColgp_Array1OfVec2d& DPoles2d,
                                           TColgp_Array1OfVec2d& D2Poles2d,
                                           TColStd_Array1OfReal& Weigths,
                                           TColStd_Array1OfReal& DWeigths,
                                           TColStd_Array1OfReal& D2Weigths) Standard_OVERRIDE;

  //! Used for the first and last section
  Standard_EXPORT Standard_Boolean Section(const Blend_Point&    P,
                                           TColgp_Array1OfPnt&   Poles,
                                           TColgp_Array1OfVec&   DPoles,
                                           TColgp_Array1OfPnt2d& Poles2d,
                                           TColgp_Array1OfVec2d& DPoles2d,
                                           TColStd_Array1OfReal& Weigths,
                                           TColStd_Array1OfReal& DWeigths) Standard_OVERRIDE;

  Standard_EXPORT void Section(const Blend_Point&    P,
                               TColgp_Array1OfPnt&   Poles,
                               TColgp_Array1OfPnt2d& Poles2d,
                               TColStd_Array1OfReal& Weigths) Standard_OVERRIDE;

  Standard_EXPORT gp_Ax1 AxeRot(const Standard_Real Prm);

  Standard_EXPORT void Resolution(const Standard_Integer IC2d,
                                  const Standard_Real    Tol,
                                  Standard_Real&         TolU,
                                  Standard_Real&         TolV) const Standard_OVERRIDE;

protected:
private:
  Handle(Adaptor3d_Surface) surf1;
  Handle(Adaptor3d_Surface) surf2;
  Handle(Adaptor3d_Curve)   curv;
  gp_Pnt                    pts1;
  gp_Pnt                    pts2;
  Standard_Boolean          istangent;
  gp_Vec                    tg1;
  gp_Vec2d                  tg12d;
  gp_Vec                    tg2;
  gp_Vec2d                  tg22d;
  gp_Pnt                    ptgui;
  gp_Vec                    d1gui;
  gp_Vec                    d2gui;
  gp_Vec                    nplan;
  Standard_Real             normtg;
  Standard_Real             theD;
  Standard_Real             distmin;
};

#endif // _BlendFunc_Ruled_HeaderFile
