// Created on: 1995-01-27
// Created by: Jacques GOUSSARD
// Copyright (c) 1995-1999 Matra Datavision
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

#include <GeomInt_MyGradientbisOfTheComputeLineOfWLApprox.hxx>

#include <Standard_OutOfRange.hxx>
#include <StdFail_NotDone.hxx>
#include <GeomInt_TheMultiLineOfWLApprox.hxx>
#include <GeomInt_TheMultiLineToolOfWLApprox.hxx>
#include <GeomInt_ParLeastSquareOfMyGradientbisOfTheComputeLineOfWLApprox.hxx>
#include <GeomInt_ResConstraintOfMyGradientbisOfTheComputeLineOfWLApprox.hxx>
#include <GeomInt_ParFunctionOfMyGradientbisOfTheComputeLineOfWLApprox.hxx>
#include <GeomInt_Gradient_BFGSOfMyGradientbisOfTheComputeLineOfWLApprox.hxx>
#include <AppParCurves_MultiCurve.hxx>

#define MultiLine GeomInt_TheMultiLineOfWLApprox
#define MultiLine_hxx <GeomInt_TheMultiLineOfWLApprox.hxx>
#define ToolLine GeomInt_TheMultiLineToolOfWLApprox
#define ToolLine_hxx <GeomInt_TheMultiLineToolOfWLApprox.hxx>
#define AppParCurves_ParLeastSquare GeomInt_ParLeastSquareOfMyGradientbisOfTheComputeLineOfWLApprox
#define AppParCurves_ParLeastSquare_hxx                                                            \
  <GeomInt_ParLeastSquareOfMyGradientbisOfTheComputeLineOfWLApprox.hxx>
#define AppParCurves_ResConstraint GeomInt_ResConstraintOfMyGradientbisOfTheComputeLineOfWLApprox
#define AppParCurves_ResConstraint_hxx                                                             \
  <GeomInt_ResConstraintOfMyGradientbisOfTheComputeLineOfWLApprox.hxx>
#define AppParCurves_ParFunction GeomInt_ParFunctionOfMyGradientbisOfTheComputeLineOfWLApprox
#define AppParCurves_ParFunction_hxx                                                               \
  <GeomInt_ParFunctionOfMyGradientbisOfTheComputeLineOfWLApprox.hxx>
#define AppParCurves_Gradient_BFGS GeomInt_Gradient_BFGSOfMyGradientbisOfTheComputeLineOfWLApprox
#define AppParCurves_Gradient_BFGS_hxx                                                             \
  <GeomInt_Gradient_BFGSOfMyGradientbisOfTheComputeLineOfWLApprox.hxx>
#define AppParCurves_Gradient GeomInt_MyGradientbisOfTheComputeLineOfWLApprox
#define AppParCurves_Gradient_hxx <GeomInt_MyGradientbisOfTheComputeLineOfWLApprox.hxx>
#include <AppParCurves_Gradient.gxx>
