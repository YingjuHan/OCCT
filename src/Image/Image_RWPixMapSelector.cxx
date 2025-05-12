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

#include <Image_RWPixMapSelector.hxx>

#include <Message.hxx>

IMPLEMENT_STANDARD_RTTIEXT(Image_RWPixMapSelector, Image_RWPixMap)

// ================================================================
// Function : Image_RWPixMapSelector
// Purpose  :
// ================================================================
Image_RWPixMapSelector::Image_RWPixMapSelector()
{
  //
}

// ================================================================
// Function : ~Image_RWPixMapSelector
// Purpose  :
// ================================================================
Image_RWPixMapSelector::~Image_RWPixMapSelector()
{
  //
}

//=======================================================================
// function : AddLibrary
// purpose :
//=======================================================================
void Image_RWPixMapSelector::AddLibrary(const Handle(Image_RWPixMap)& theLibrary, bool theIsPreferred)
{
  myLibraries.Remove(theLibrary); // avoid duplicates
  if (theIsPreferred)
  {
    myLibraries.Prepend(theLibrary);
  }
  else
  {
    myLibraries.Append(theLibrary);
  }
}

//=======================================================================
// function : RemoveLibrary
// purpose :
//=======================================================================
void Image_RWPixMapSelector::RemoveLibrary(const Handle(Image_RWPixMap)& theLibrary)
{
  myLibraries.Remove(theLibrary);
}

// ================================================================
// Function : FormatFromName
// Purpose  :
// ================================================================
TCollection_AsciiString Image_RWPixMapSelector::FormatFromName(const TCollection_AsciiString& theFileName) const
{
  return !myLibraries.IsEmpty() ? myLibraries.First()->FormatFromName(theFileName) : Image_RWPixMap::CommonFormatFromName(theFileName);
}

// ================================================================
// Function : ProbeFormat
// Purpose  :
// ================================================================
TCollection_AsciiString Image_RWPixMapSelector::ProbeFormat(const Handle(NCollection_Buffer)& theData,
                                                            std::istream* theStream,
                                                            const TCollection_AsciiString& theFileName) const
{
  return !myLibraries.IsEmpty() ? myLibraries.First()->ProbeFormat(theData, theStream, theFileName) : Image_RWPixMap::CommonProbeFormat(theData, theStream, theFileName);
}

// ================================================================
// Function : SupportsReading
// Purpose  :
// ================================================================
bool Image_RWPixMapSelector::SupportsReading(const TCollection_AsciiString& theName) const
{
  for (const Handle(Image_RWPixMap)& aLibIter : myLibraries)
  {
    if (aLibIter->SupportsReading(theName))
    {
      return true;
    }
  }
  return false;
}

// ================================================================
// Function : SupportsWriting
// Purpose  :
// ================================================================
bool Image_RWPixMapSelector::SupportsWriting(const TCollection_AsciiString& theName) const
{
  for (const Handle(Image_RWPixMap)& aLibIter : myLibraries)
  {
    if (aLibIter->SupportsWriting(theName))
    {
      return true;
    }
  }
  return false;
}

// ================================================================
// Function : Read
// Purpose  :
// ================================================================
bool Image_RWPixMapSelector::Read(Image_PixMap& thePixmap,
                                  const Handle(NCollection_Buffer)& theData,
                                  std::istream* theStream,
                                  const TCollection_AsciiString& theFileName) const
{
  // probe file format first if there are several readers to choose from
  const TCollection_AsciiString aFormat = myLibraries.Size() > 1 ? ProbeFormat(theData, theStream, theFileName) : "";
  if (!aFormat.IsEmpty())
  {
    for (const Handle(Image_RWPixMap)& aLibIter : myLibraries)
    {
      if (aLibIter->SupportsReading(aFormat))
      {
        return aLibIter->Read(thePixmap, theData, theStream, theFileName);
      }
    }
  }

  return !myLibraries.IsEmpty() && myLibraries.First()->Read(thePixmap, theData, theStream, theFileName);
}

// ================================================================
// Function : Write
// Purpose  :
// ================================================================
bool Image_RWPixMapSelector::Write(Image_PixMap& thePixmap,
                                   const TCollection_AsciiString& theFileName,
                                   const TCollection_AsciiString& theFormat) const
{
  const TCollection_AsciiString aFormat = !theFormat.IsEmpty() ? theFormat : FormatFromName(theFileName);
  if (!aFormat.IsEmpty())
  {
    for (const Handle(Image_RWPixMap)& aLibIter : myLibraries)
    {
      if (aLibIter->SupportsWriting(aFormat))
      {
        return aLibIter->Write(thePixmap, theFileName, aFormat);
      }
    }
  }

  return !myLibraries.IsEmpty() && myLibraries.First()->Write(thePixmap, theFileName, theFormat);
}

// =======================================================================
// function : IsTopDownDefault
// purpose  :
// =======================================================================
bool Image_RWPixMapSelector::IsTopDownDefault() const
{
  return !myLibraries.IsEmpty() && myLibraries.First()->IsTopDownDefault();
}

// =======================================================================
// function : AdjustGamma
// purpose  :
// =======================================================================
bool Image_RWPixMapSelector::AdjustGamma(Image_PixMap& thePixmap,
                                         const Standard_Real theGammaCorr) const
{
  return !myLibraries.IsEmpty() && myLibraries.First()->AdjustGamma(thePixmap, theGammaCorr);
}

// ================================================================
// Function : InitTrash
// Purpose  :
// ================================================================
bool Image_RWPixMapSelector::InitTrash(Image_PixMap& thePixmap,
                                       Image_Format thePixelFormat,
                                       const NCollection_Vec3<Standard_Size>& theDims,
                                       const Standard_Size theSizeRowBytes) const
{
  return !myLibraries.IsEmpty() && myLibraries.First()->InitTrash(thePixmap, thePixelFormat, theDims, theSizeRowBytes);
}
