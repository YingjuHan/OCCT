// Created on: 1993-01-21
// Created by: Remi LEQUETTE
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

#ifndef _TopoDS_Iterator_HeaderFile
#define _TopoDS_Iterator_HeaderFile

#include <Standard_NoSuchObject.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_ListIteratorOfListOfShape.hxx>
#include <TopAbs_Orientation.hxx>
#include <TopLoc_Location.hxx>

//! Iterates on the underlying shape underlying a given
//! TopoDS_Shape object, providing access to its
//! component sub-shapes. Each component shape is
//! returned as a TopoDS_Shape with an orientation,
//! and a compound of the original values and the relative values.
class TopoDS_Iterator 
{
public:

  DEFINE_STANDARD_ALLOC

  //! Creates an empty Iterator.
  TopoDS_Iterator() : myOrientation(TopAbs_FORWARD) {}

  //! Creates an Iterator on <S> sub-shapes.
  //! Note:
  //! - If cumOri is true, the function composes all
  //! sub-shapes with the orientation of S.
  //! - If cumLoc is true, the function multiplies all
  //! sub-shapes by the location of S, i.e. it applies to
  //! each sub-shape the transformation that is associated with S.
  TopoDS_Iterator (const TopoDS_Shape& S,
                   const Standard_Boolean cumOri = Standard_True,
                   const Standard_Boolean cumLoc = Standard_True)
  {
    Initialize (S, cumOri,cumLoc);
  }

  //! Initializes this iterator with shape S.
  //! Note:
  //! - If cumOri is true, the function composes all
  //! sub-shapes with the orientation of S.
  //! - If cumLoc is true, the function multiplies all
  //! sub-shapes by the location of S, i.e. it applies to
  //! each sub-shape the transformation that is associated with S.
  Standard_EXPORT void Initialize (const TopoDS_Shape& S, const Standard_Boolean cumOri = Standard_True, const Standard_Boolean cumLoc = Standard_True);
  
  //! Returns true if there is another sub-shape in the
  //! shape which this iterator is scanning.
  Standard_Boolean More() const { return myShapes.More(); }

  //! Moves on to the next sub-shape in the shape which
  //! this iterator is scanning.
  //! Exceptions
  //! Standard_NoMoreObject if there are no more sub-shapes in the shape.
  Standard_EXPORT void Next();
  
  //! Returns the current sub-shape in the shape which
  //! this iterator is scanning.
  //! Exceptions
  //! Standard_NoSuchObject if there is no current sub-shape.
  const TopoDS_Shape& Value() const
  {
    Standard_NoSuchObject_Raise_if(!More(),"TopoDS_Iterator::Value");  
    return myShape;
  }

public:

  //! Copy constructor.
  TopoDS_Iterator(const TopoDS_Iterator& theOther) = default;

  //! Move constructor.
  TopoDS_Iterator(TopoDS_Iterator&& theOther) = default;

  //! Assginment operator.
  TopoDS_Iterator& operator=(const TopoDS_Iterator& theOther) = default;

  //! Move operator.
  TopoDS_Iterator& operator=(TopoDS_Iterator&& theOther) = default;

  //! Returns the current subshape.
  const TopoDS_Shape& operator*() const { return Value(); }

  //! Returns pointer to the current subshape.
  const TopoDS_Shape* operator->() const { return &Value(); }

  //! Move iterator to next subshape and return new position.
  TopoDS_Iterator& operator++()
  {
    Next();
    return *this;
  }

  //! Move iterator to next subshape and return previous position.
  TopoDS_Iterator operator++(int)
  {
    TopoDS_Iterator aCopy(*this);
    Next();
    return aCopy;
  }

  //! Equality operator.
  bool operator==(const TopoDS_Iterator& theOther) const { return myShapes == theOther.myShapes; }

  //! Non-equality operator.
  bool operator!=(const TopoDS_Iterator& theOther) const { return !(myShapes == theOther.myShapes); }

public:
  //! Wrapper for range-based loops implementing only minimal set of operations.
  class StlIterator
  {
  public:
    const TopoDS_Shape& operator*() const { return myIter->Value(); }
    const TopoDS_Shape* operator->() const { return &myIter->Value(); }
    StlIterator& operator++()
    {
      myIter->Next();
      if (!myIter->More())
        myIter = nullptr; // equal to end()

      return *this;
    }
    bool operator==(const StlIterator& theOther) const { return myIter == theOther.myIter; }
    bool operator!=(const StlIterator& theOther) const { return myIter != theOther.myIter; }
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = TopoDS_Shape;
    using difference_type = ptrdiff_t;
    using pointer = const TopoDS_Shape*;
    using reference = const TopoDS_Shape&;
  private:
    friend class TopoDS_Iterator;
    StlIterator(TopoDS_Iterator* theIter) : myIter((theIter != nullptr && theIter->More()) ? theIter : nullptr) {}
  private:
    TopoDS_Iterator* myIter = nullptr;
  };

  //! Returns iterator pointing to this for range-based loop.
  //! TopoDS_Iterator will be modified during the loop.
  StlIterator begin() { return StlIterator(this); }

  //! Returns iterator pointing to nothing for range-based loop.
  StlIterator end() { return StlIterator(nullptr); }

public: //! @name types for std::iterator
  using iterator_category = std::forward_iterator_tag;
  using value_type = TopoDS_Shape;
  using difference_type = ptrdiff_t;
  using pointer = const TopoDS_Shape*;
  using reference = const TopoDS_Shape&;

private:

  TopoDS_Shape myShape;
  TopoDS_ListIteratorOfListOfShape myShapes;
  TopAbs_Orientation myOrientation;
  TopLoc_Location myLocation;

};

#endif // _TopoDS_Iterator_HeaderFile
