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

#include <Image_RWEmscripten.hxx>

#if defined(__EMSCRIPTEN__)
  #include <emscripten/emscripten.h>
#endif

#include <Image_RWPPM.hxx>
#include <Message.hxx>
#include <Standard_NotImplemented.hxx>

#ifdef __EMSCRIPTEN__
//! Owner of image data.
class Image_RWEmscripten::Owner : public NCollection_BaseAllocator
{
public:

  //! Initialize from existing image.
  Owner(char* theData) : myImgData(theData) {}

  //! Destructor.
  ~Owner()
  {
    if (myImgData != nullptr)
    {
      free((void*)myImgData);
      myImgData = nullptr;
    }
  }

  //! Dummy placeholder.
  virtual void* Allocate(const size_t) override { return nullptr; }

  //! Dummy placeholder.
  virtual void Free(void*) override {}

private:
  char* myImgData = nullptr;
};
#endif

IMPLEMENT_STANDARD_RTTIEXT(Image_RWEmscripten, Image_RWPixMap)

// ================================================================
// Function : Image_RWEmscripten
// Purpose  :
// ================================================================
Image_RWEmscripten::Image_RWEmscripten()
{
  //
}

// ================================================================
// Function : IsAvailable
// Purpose  :
// ================================================================
bool Image_RWEmscripten::IsAvailable() const
{
#if defined(__EMSCRIPTEN__)
  return true;
#else
  return false;
#endif
}

// ================================================================
// Function : SupportsReading
// Purpose  :
// ================================================================
bool Image_RWEmscripten::SupportsReading(const TCollection_AsciiString& theName) const
{
  (void)theName;
  return false;
}

// ================================================================
// Function : SupportsWriting
// Purpose  :
// ================================================================
bool Image_RWEmscripten::SupportsWriting(const TCollection_AsciiString& theName) const
{
  return theName == IMAGE_TYPE_PPM;
}

// ================================================================
// Function : Read
// Purpose  :
// ================================================================
bool Image_RWEmscripten::Read(Image_PixMap& thePixmap,
                              const Handle(NCollection_Buffer)& theData,
                              std::istream* theStream,
                              const TCollection_AsciiString& theFileName) const
{
  thePixmap.Clear();
  if (!theData.IsNull())
  {
    Message::SendFail() << "Error: Image_RWEmscripten doesn't support decoding of in-memory buffer";
    return false;
  }
  else if (theStream != nullptr)
  {
    Message::SendFail() << "Error: Image_RWEmscripten doesn't support decoding from stream";
    return false;
  }

#if defined(__EMSCRIPTEN__)
  int aSizeX = 0, aSizeY = 0;
  char* anImgData = emscripten_get_preloaded_image_data(theFileName.ToCString(), &aSizeX, &aSizeY);
  if (anImgData == nullptr)
  {
    Message::SendFail() << "Error: image '" << theFileName << "' is not preloaded";
    return false;
  }

  Handle(Image_RWEmscripten::Owner) anOwner = new Image_RWEmscripten::Owner(anImgData);
  thePixmap.InitWrapper3D(Image_Format_RGBA, (Standard_Byte* )anImgData,
                          NCollection_Vec3<Standard_Size>(aSizeX, aSizeY, 1), 0, anOwner);
  thePixmap.SetTopDown(true);
  return true;
#else
  (void)theFileName;
  Message::SendFail() << "Error: Image_RWEmscripten library is unavailable on this platform";
  return false;
#endif
}

// ================================================================
// Function : Write
// Purpose  :
// ================================================================
bool Image_RWEmscripten::Write(Image_PixMap& thePixmap,
                               const TCollection_AsciiString& theFileName,
                               const TCollection_AsciiString& theFormat) const
{
  if (thePixmap.IsEmpty() || theFileName.IsEmpty())
  {
    return false;
  }

  const TCollection_AsciiString aFormat = !theFormat.IsEmpty() ? theFormat : CommonFormatFromName(theFileName);
  if (aFormat == IMAGE_TYPE_PPM)
  {
    Image_RWPPM aTool;
    return aTool.Write(thePixmap, theFileName, IMAGE_TYPE_PPM);
  }

  Message::SendFail() << "Error: Image_RWEmscripten library doesn't support writing image files";
  return false;
}
