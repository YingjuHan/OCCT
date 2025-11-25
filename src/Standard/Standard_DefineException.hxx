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

#ifndef _Standard_DefineException_HeaderFile
#define _Standard_DefineException_HeaderFile

#include <Standard_Type.hxx>

//! Defines an exception class \a C1 that inherits an exception class \a C2.
/*! \a C2 must be Standard_Failure or its ancestor.
    The macro defines empty constructor, copy constructor and static methods Raise() and NewInstance().
*/

#define DEFINE_STANDARD_EXCEPTION(C1,C2) \
 \
class C1 : public C2 { \
  void Throw () const Standard_OVERRIDE { throw *this; } \
public: \
  C1() : C2() {} \
  C1(Standard_CString theMessage) : C2(theMessage) {} \
  C1(Standard_CString theMessage, Standard_CString theStackTrace) \
  : C2 (theMessage, theStackTrace) {} \
  static void Raise(const Standard_CString theMessage = "") { \
    std::shared_ptr<C1> _E = std::make_shared<C1>(); \
    _E->Reraise(theMessage); \
  } \
  static void Raise(Standard_SStream& theMessage) { \
    std::shared_ptr<C1> _E = std::make_shared<C1>(); \
    _E->Reraise (theMessage); \
  } \
  static std::shared_ptr<C1> NewInstance(Standard_CString theMessage = "") { return std::make_shared<C1>(theMessage); } \
  static std::shared_ptr<C1> NewInstance(Standard_CString theMessage, Standard_CString theStackTrace) { return std::make_shared<C1>(theMessage, theStackTrace); } \
  virtual const char* ExceptionType() const Standard_OVERRIDE { return #C1; } \
};

//! Obsolete macro, kept for compatibility with old code
#define IMPLEMENT_STANDARD_EXCEPTION(C1) 

#endif
