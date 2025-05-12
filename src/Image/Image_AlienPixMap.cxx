// Created on: 2010-09-16
// Created by: KGV
// Copyright (c) 2010-2014 OPEN CASCADE SAS
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

#include <Image_AlienPixMap.hxx>

#include <Message.hxx>

IMPLEMENT_STANDARD_RTTIEXT(Image_AlienPixMap,Image_PixMap)

// =======================================================================
// function : Image_AlienPixMap
// purpose  :
// =======================================================================
Image_AlienPixMap::Image_AlienPixMap()
{
  myLibImage = Image_RWPixMap::DefaultSelector();
  SetTopDown (myLibImage->IsTopDownDefault());
}

// =======================================================================
// function : ~Image_AlienPixMap
// purpose  :
// =======================================================================
Image_AlienPixMap::~Image_AlienPixMap()
{
  //
}

// =======================================================================
// function : InitTrash
// purpose  :
// =======================================================================
bool Image_AlienPixMap::InitTrash (Image_Format        thePixelFormat,
                                   const Standard_Size theSizeX,
                                   const Standard_Size theSizeY,
                                   const Standard_Size theSizeRowBytes)
{
  return myLibImage->InitTrash(*this, thePixelFormat, NCollection_Vec3<Standard_Size>(theSizeX, theSizeY, 1), theSizeRowBytes);
}

// =======================================================================
// function : IsTopDownDefault
// purpose  :
// =======================================================================
bool Image_AlienPixMap::IsTopDownDefault()
{
  return Image_RWPixMap::DefaultSelector()->IsTopDownDefault();
}

// =======================================================================
// function : Load
// purpose  :
// =======================================================================
bool Image_AlienPixMap::Load (const Standard_Byte* theData,
                              Standard_Size theLength,
                              const TCollection_AsciiString& theImagePath)
{
  Handle(NCollection_Buffer) aBuffer;
  if (theData != nullptr)
  {
    aBuffer = new NCollection_Buffer(nullptr, theLength, const_cast<Standard_Byte*>(theData));
  }
  return myLibImage->Read(*this, aBuffer, nullptr, theImagePath);
}

// =======================================================================
// function : Load
// purpose  :
// =======================================================================
bool Image_AlienPixMap::Load (std::istream& theStream,
                              const TCollection_AsciiString& theFileName)
{
  return myLibImage->Read(*this, Handle(NCollection_Buffer)(), &theStream, theFileName);
}

// =======================================================================
// function : Save
// purpose  :
// =======================================================================
bool Image_AlienPixMap::Save (const TCollection_AsciiString& theFileName)
{
  return myLibImage->Write(*this, theFileName, "");
}

// =======================================================================
// function : AdjustGamma
// purpose  :
// =======================================================================
bool Image_AlienPixMap::AdjustGamma (const Standard_Real theGammaCorr)
{
  return myLibImage->AdjustGamma(*this, theGammaCorr);
}
