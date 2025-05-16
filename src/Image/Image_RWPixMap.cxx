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

#include <Image_RWPixMap.hxx>

#include <Image_RWEmscripten.hxx>
#include <Image_RWFreeImage.hxx>
#include <Image_RWPixMapSelector.hxx>
#include <Image_RWPNG.hxx>
#include <Image_RWPPM.hxx>
#include <Image_RWWinCodec.hxx>
#include <Message.hxx>
#include <OSD_FileSystem.hxx>

#include <algorithm>

const char* Image_RWPixMap::IMAGE_TYPE_PNG = "png";
const char* Image_RWPixMap::IMAGE_TYPE_JPG = "jpg";
const char* Image_RWPixMap::IMAGE_TYPE_GIF = "gif";
const char* Image_RWPixMap::IMAGE_TYPE_TIFF = "tiff";
const char* Image_RWPixMap::IMAGE_TYPE_BMP = "bmp";
const char* Image_RWPixMap::IMAGE_TYPE_WEBP = "webp";
const char* Image_RWPixMap::IMAGE_TYPE_DDS = "dds";
const char* Image_RWPixMap::IMAGE_TYPE_PPM = "ppm";
const char* Image_RWPixMap::IMAGE_TYPE_EXR = "exr";
const char* Image_RWPixMap::IMAGE_TYPE_HDR = "hdr";
const char* Image_RWPixMap::IMAGE_TYPE_PSD = "psd";
const char* Image_RWPixMap::IMAGE_TYPE_ICO = "ico";

IMPLEMENT_STANDARD_RTTIEXT(Image_RWPixMap, Standard_Transient)

//=======================================================================
// function : createDefaultSelector
// purpose :
//=======================================================================
static Handle(Image_RWPixMapSelector) createDefaultSelector()
{
  Handle(Image_RWPixMapSelector) aSystem = new Image_RWPixMapSelector();

  Handle(Image_RWFreeImage) aFreeImg = new Image_RWFreeImage();
  if (aFreeImg->IsAvailable())
  {
    aSystem->AddLibrary(aFreeImg, true);
  }

  // platform-specific libraries
#if defined(_WIN32)
  Handle(Image_RWWinCodec) aWinImg = new Image_RWWinCodec();
  if (!aFreeImg->IsAvailable() && aWinImg->IsAvailable())
  {
    aSystem->AddLibrary(aWinImg, true);
  }
#elif defined(__EMSCRIPTEN__)
  Handle(Image_RWEmscripten) aWasmImg = new Image_RWEmscripten();
  if (aWasmImg->IsAvailable())
  {
    aSystem->AddLibrary(aWasmImg, true);
  }
#endif
  if (!aSystem->Libraries().IsEmpty())
  {
    // prefer multi-format image library
    return aSystem;
  }

  // use per-format libraries only when multi-format library is disabled
  Handle(Image_RWPNG) aPngImg = new Image_RWPNG();
  if (aPngImg->IsAvailable())
  {
    aSystem->AddLibrary(aPngImg, false);
  }

  Handle(Image_RWPPM) aPpmImg = new Image_RWPPM();
  if (aPpmImg->IsAvailable())
  {
    aSystem->AddLibrary(aPpmImg, false);
  }
  return aSystem;
}

//=======================================================================
// function : DefaultSelector
// purpose :
//=======================================================================
const Handle(Image_RWPixMap)& Image_RWPixMap::DefaultSelector()
{
  static const Handle(Image_RWPixMapSelector) aDefSystem = createDefaultSelector();
  return aDefSystem;
}

// ================================================================
// Function : ~Image_RWPixMap
// Purpose  :
// ================================================================
Image_RWPixMap::~Image_RWPixMap()
{
  //
}

// =======================================================================
// function : CommonFormatFromName
// purpose  :
// =======================================================================
TCollection_AsciiString Image_RWPixMap::CommonFormatFromName(const TCollection_AsciiString& theFileName)
{
  TCollection_AsciiString aName(theFileName);
  aName.LowerCase();
  if (aName.EndsWith(".png") || aName.EndsWith(".pns"))
  {
    return IMAGE_TYPE_PNG;
  }
  else if (aName.EndsWith(".jpg") || aName.EndsWith(".jpeg") || aName.EndsWith(".jpe") || aName.EndsWith(".jps") || aName.EndsWith(".mpo"))
  {
    return IMAGE_TYPE_JPG;
  }
  else if (aName.EndsWith(".gif"))
  {
    return IMAGE_TYPE_GIF;
  }
  else if (aName.EndsWith(".webp") || aName.EndsWith(".webpll"))
  {
    return IMAGE_TYPE_WEBP;
  }
  else if (aName.EndsWith(".tiff") || aName.EndsWith(".tif"))
  {
    return IMAGE_TYPE_TIFF;
  }
  else if (aName.EndsWith(".exr"))
  {
    return IMAGE_TYPE_EXR;
  }
  else if (aName.EndsWith(".psd"))
  {
    return IMAGE_TYPE_PSD;
  }
  else if (aName.EndsWith(".ico"))
  {
    return IMAGE_TYPE_ICO;
  }
  else if (aName.EndsWith(".bmp"))
  {
    return IMAGE_TYPE_BMP;
  }
  else if (aName.EndsWith(".hdr"))
  {
    return IMAGE_TYPE_HDR;
  }
  else if (aName.EndsWith(".dds"))
  {
    return IMAGE_TYPE_DDS;
  }
  else if (aName.EndsWith(".ppm"))
  {
    return IMAGE_TYPE_PPM;
  }
  return "";
}

// =======================================================================
// function : CommonProbeFormat
// purpose  :
// =======================================================================
TCollection_AsciiString Image_RWPixMap::CommonProbeFormat(const Handle(NCollection_Buffer)& theData,
                                                          std::istream* theStream,
                                                          const TCollection_AsciiString& theFileName)
{
  static const Standard_Size THE_PROBE_SIZE = 20;
  char aBuffer[THE_PROBE_SIZE] = {};
  if (!theData.IsNull())
  {
    memcpy(aBuffer, theData->Data(), theData->Size() < THE_PROBE_SIZE ? theData->Size() : THE_PROBE_SIZE);
  }
  else if (theStream != nullptr)
  {
    const std::streamoff aStart = theStream->tellg();
    std::streamsize aNbRead = theStream->read(aBuffer, THE_PROBE_SIZE).gcount();
    (void)aNbRead;
    if (!*theStream)
    {
      Message::SendFail() << "Error: unable to read image file '" << theFileName << "'";
      return TCollection_AsciiString();
    }
    theStream->seekg(aStart);
  }
  else
  {
    const Handle(OSD_FileSystem)& aFileSystem = OSD_FileSystem::DefaultFileSystem();
    std::shared_ptr<std::istream> aFileIn = aFileSystem->OpenIStream(theFileName, std::ios::in | std::ios::binary);
    if (aFileIn.get() == nullptr)
    {
      Message::SendFail() << "Error: Unable to open file '" << theFileName << "'";
      return TCollection_AsciiString();
    }
    if (!aFileIn->read(aBuffer, THE_PROBE_SIZE))
    {
      Message::SendFail() << "Error: unable to read image file '" << theFileName << "'";
      return false;
    }
  }

  if (memcmp(aBuffer, "\x89" "PNG\r\n" "\x1A" "\n", 8) == 0)
  {
    return IMAGE_TYPE_PNG;
  }
  else if (memcmp(aBuffer, "\xFF\xD8\xFF", 3) == 0)
  {
    return IMAGE_TYPE_JPG;
  }
  else if (memcmp(aBuffer, "GIF87a", 6) == 0
        || memcmp(aBuffer, "GIF89a", 6) == 0)
  {
    return IMAGE_TYPE_GIF;
  }
  else if (memcmp(aBuffer, "II\x2A\x00", 4) == 0
        || memcmp(aBuffer, "MM\x00\x2A", 4) == 0)
  {
    return IMAGE_TYPE_TIFF;
  }
  else if (memcmp(aBuffer, "BM", 2) == 0)
  {
    return IMAGE_TYPE_BMP;
  }
  else if (memcmp(aBuffer, "RIFF", 4) == 0
        && memcmp(aBuffer + 8, "WEBP", 4) == 0)
  {
    return IMAGE_TYPE_WEBP;
  }
  else if (memcmp(aBuffer, "DDS ", 4) == 0)
  {
    return IMAGE_TYPE_DDS;
  }
  else if (memcmp(aBuffer, "P1\n", 3) == 0
        || memcmp(aBuffer, "P2\n", 3) == 0
        || memcmp(aBuffer, "P3\n", 3) == 0
        || memcmp(aBuffer, "P4\n", 3) == 0
        || memcmp(aBuffer, "P5\n", 3) == 0
        || memcmp(aBuffer, "P6\n", 3) == 0)
  {
    return IMAGE_TYPE_PPM;
  }
  return "";
}

// =======================================================================
// function : AdjustGamma
// purpose  :
// =======================================================================
bool Image_RWPixMap::AdjustGamma(Image_PixMap& thePixmap,
                                 const Standard_Real theGammaCorr) const
{
  if (thePixmap.IsEmpty())
  {
    return false;
  }

  (void)theGammaCorr;
  Message::SendFail() << "Error: AdjustGamma() is not implemented";
  return false;
}

// ================================================================
// Function : InitTrash
// Purpose  :
// ================================================================
bool Image_RWPixMap::InitTrash(Image_PixMap& thePixmap,
                               Image_Format thePixelFormat,
                               const NCollection_Vec3<Standard_Size>& theDims,
                               const Standard_Size theSizeRowBytes) const
{
  return thePixmap.InitTrash3D(thePixelFormat, theDims, theSizeRowBytes);
}
