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

#ifdef HAVE_FREEIMAGE
  #include <FreeImage.h>
#endif

#include <Image_RWFreeImage.hxx>

#include <Message.hxx>
#include <OSD_FileSystem.hxx>
#include <Standard_NotImplemented.hxx>
#include <TCollection_ExtendedString.hxx>

#ifdef HAVE_FREEIMAGE
//! Owner of image data.
class Image_RWFreeImage::Owner : public NCollection_BaseAllocator
{
public:
  FIBITMAP* LibImage = nullptr;

public:

  //! Initialize from existing image.
  Owner(FIBITMAP* theLibImage) : LibImage(theLibImage) {}

  //! Destructor.
  ~Owner()
  {
    Reset();
  }

  //! Dummy placeholder.
  virtual void* Allocate(const size_t ) override { return nullptr; }

  //! Dummy placeholder.
  virtual void Free(void* ) override {}

  //! Initialize from existing image.
  void Init(FIBITMAP* theLibImage)
  {
    Reset();
    LibImage = theLibImage;
  }

  //! Reset image.
  void Reset()
  {
    if (LibImage != nullptr)
    {
      FreeImage_Unload(LibImage);
      LibImage = nullptr;
    }
  }

};
#endif

namespace
{
#ifdef HAVE_FREEIMAGE

  static Image_Format convertFromFreeFormat(FREE_IMAGE_TYPE       theFormatFI,
                                            FREE_IMAGE_COLOR_TYPE theColorTypeFI,
                                            unsigned              theBitsPerPixel)
  {
    switch (theFormatFI)
    {
      case FIT_RGBF:   return Image_Format_RGBF;
      case FIT_RGBAF:  return Image_Format_RGBAF;
      case FIT_FLOAT:  return Image_Format_GrayF;
      case FIT_INT16:
      case FIT_UINT16: return Image_Format_Gray16;
      case FIT_BITMAP:
      {
        switch (theColorTypeFI)
        {
          case FIC_MINISBLACK:
          {
            return Image_Format_Gray;
          }
          case FIC_RGB:
          {
            if (Image_PixMap::IsBigEndianHost())
            {
              return (theBitsPerPixel == 32) ? Image_Format_RGB32 : Image_Format_RGB;
            }
            else
            {
              return (theBitsPerPixel == 32) ? Image_Format_BGR32 : Image_Format_BGR;
            }
          }
          case FIC_RGBALPHA:
          {
            return Image_PixMap::IsBigEndianHost() ? Image_Format_RGBA : Image_Format_BGRA;
          }
          default:
            return Image_Format_UNKNOWN;
        }
      }
      default:
        return Image_Format_UNKNOWN;
    }
  }

  static FREE_IMAGE_TYPE convertToFreeFormat(Image_Format theFormat)
  {
    switch (theFormat)
    {
      case Image_Format_GrayF:
      case Image_Format_AlphaF:
        return FIT_FLOAT;
      case Image_Format_RGBAF:
        return FIT_RGBAF;
      case Image_Format_RGBF:
        return FIT_RGBF;
      case Image_Format_RGBA:
      case Image_Format_BGRA:
      case Image_Format_RGB32:
      case Image_Format_BGR32:
      case Image_Format_RGB:
      case Image_Format_BGR:
      case Image_Format_Gray:
      case Image_Format_Alpha:
        return FIT_BITMAP;
      case Image_Format_Gray16:
        return FIT_UINT16;
      default:
        return FIT_UNKNOWN;
    }
  }

  //! Return image format name.
  static const char* getImageFormatName(FREE_IMAGE_FORMAT theFormat)
  {
    switch (theFormat)
    {
      case FIF_BMP: return Image_RWPixMap::IMAGE_TYPE_BMP;
      case FIF_ICO: return Image_RWPixMap::IMAGE_TYPE_ICO;
      case FIF_JPEG: return Image_RWPixMap::IMAGE_TYPE_JPG;
      case FIF_JNG: return "jng";
      case FIF_KOALA: return "koala";
      case FIF_IFF: return "iff";
      case FIF_MNG: return "mng";
      case FIF_PBM: return "pbm";
      case FIF_PBMRAW: return "pbm";
      case FIF_PCD: return "pcd";
      case FIF_PCX: return "pcx";
      case FIF_PGM: return "pgm";
      case FIF_PGMRAW: return "pgm";
      case FIF_PNG: return Image_RWPixMap::IMAGE_TYPE_PNG;
      case FIF_PPM: return Image_RWPixMap::IMAGE_TYPE_PPM;
      case FIF_PPMRAW: return Image_RWPixMap::IMAGE_TYPE_PPM;
      case FIF_RAS: return "ras";
      case FIF_TARGA: return "tga";
      case FIF_TIFF: return Image_RWPixMap::IMAGE_TYPE_TIFF;
      case FIF_WBMP: return "wbmp";
      case FIF_PSD: return Image_RWPixMap::IMAGE_TYPE_PSD;
      case FIF_CUT: return "cut";
      case FIF_XBM: return "xbm";
      case FIF_XPM: return "xpm";
      case FIF_DDS: return Image_RWPixMap::IMAGE_TYPE_DDS;
      case FIF_GIF: return Image_RWPixMap::IMAGE_TYPE_GIF;
      case FIF_HDR: return Image_RWPixMap::IMAGE_TYPE_HDR;
      case FIF_FAXG3: return "faxg3";
      case FIF_SGI: return "sgi";
      case FIF_EXR: return Image_RWPixMap::IMAGE_TYPE_EXR;
      case FIF_J2K: return "j2k";
      case FIF_JP2: return "jp2";
      case FIF_PFM: return "pfm";
      case FIF_PICT: return "pict";
      case FIF_RAW: return "raw";
      case FIF_WEBP: return Image_RWPixMap::IMAGE_TYPE_WEBP;
      case FIF_JXR: return "jxr";
      default: return "";
    }
  }

  //! Wrapper for accessing C++ stream from FreeImage.
  class Image_FreeImageStream
  {
  public:
    //! Construct wrapper over input stream.
    Image_FreeImageStream(std::istream* theStream)
    : myIStream(theStream),
      myOStream(nullptr),
      myInitPos(theStream != nullptr ? theStream->tellg() : std::streampos(0))
    {
    }

    //! Get io object.
    FreeImageIO GetFiIO() const
    {
      FreeImageIO anIo = {};
      if (myIStream != nullptr)
      {
        anIo.read_proc = readProc;
        anIo.seek_proc = seekProc;
        anIo.tell_proc = tellProc;
      }
      if (myOStream != nullptr)
      {
        anIo.write_proc = writeProc;
      }
      return anIo;
    }
  public:
    //! Simulate fread().
    static unsigned int DLL_CALLCONV readProc(void* theBuffer, unsigned int theSize, unsigned int theCount, fi_handle theHandle)
    {
      Image_FreeImageStream* aThis = (Image_FreeImageStream*)theHandle;
      if (aThis->myIStream == nullptr)
      {
        return 0;
      }

      if (!aThis->myIStream->read((char*)theBuffer, std::streamsize(theSize) * std::streamsize(theCount)))
      {
        //aThis->myIStream->clear();
      }
      const std::streamsize aNbRead = aThis->myIStream->gcount();
      return (unsigned int)(aNbRead / theSize);
    }

    //! Simulate fwrite().
    static unsigned int DLL_CALLCONV writeProc(void* theBuffer, unsigned int theSize, unsigned int theCount, fi_handle theHandle)
    {
      Image_FreeImageStream* aThis = (Image_FreeImageStream*)theHandle;
      if (aThis->myOStream != nullptr
       && aThis->myOStream->write((const char*)theBuffer, std::streamsize(theSize) * std::streamsize(theCount)))
      {
        return theCount;
      }
      return 0;
    }

    //! Simulate fseek().
    static int DLL_CALLCONV seekProc(fi_handle theHandle, long theOffset, int theOrigin)
    {
      Image_FreeImageStream* aThis = (Image_FreeImageStream*)theHandle;
      if (aThis->myIStream == nullptr)
      {
        return -1;
      }

      bool isSeekDone = false;
      switch (theOrigin)
      {
        case SEEK_SET:
          if (aThis->myIStream->seekg((std::streamoff)aThis->myInitPos + theOffset, std::ios::beg))
          {
            isSeekDone = true;
          }
          break;
        case SEEK_CUR:
          if (aThis->myIStream->seekg(theOffset, std::ios::cur))
          {
            isSeekDone = true;
          }
          break;
        case SEEK_END:
          if (aThis->myIStream->seekg(theOffset, std::ios::end))
          {
            isSeekDone = true;
          }
          break;
      }
      return isSeekDone ? 0 : -1;
    }

    //! Simulate ftell().
    static long DLL_CALLCONV tellProc(fi_handle theHandle)
    {
      Image_FreeImageStream* aThis = (Image_FreeImageStream*)theHandle;
      const long aPos = aThis->myIStream != NULL ? (long)(aThis->myIStream->tellg() - aThis->myInitPos) : 0;
      return aPos;
    }
  private:
    std::istream*  myIStream = nullptr;
    std::ostream*  myOStream = nullptr;
    std::streampos myInitPos = 0;
  };

#endif
}

IMPLEMENT_STANDARD_RTTIEXT(Image_RWFreeImage, Image_RWPixMap)

// ================================================================
// Function : Image_RWFreeImage
// Purpose  :
// ================================================================
Image_RWFreeImage::Image_RWFreeImage()
{
  //
}

// ================================================================
// Function : IsAvailable
// Purpose  :
// ================================================================
bool Image_RWFreeImage::IsAvailable() const
{
#ifdef HAVE_FREEIMAGE
  return true;
#else
  return false;
#endif
}

// ================================================================
// Function : ProbeFormat
// Purpose  :
// ================================================================
TCollection_AsciiString Image_RWFreeImage::FormatFromName(const TCollection_AsciiString& theFileName) const
{
#ifdef HAVE_FREEIMAGE
  FREE_IMAGE_FORMAT aFormat = FreeImage_GetFIFFromFilename(theFileName.ToCString());
  return getImageFormatName(aFormat);
#else
  (void)theFileName;
  return "";
#endif
}

// ================================================================
// Function : ProbeFormat
// Purpose  :
// ================================================================
TCollection_AsciiString Image_RWFreeImage::ProbeFormat(const Handle(NCollection_Buffer)& theData,
                                                       std::istream* theStream,
                                                       const TCollection_AsciiString& theFileName) const
{
#ifdef HAVE_FREEIMAGE
  FREE_IMAGE_FORMAT aFormat = FIF_UNKNOWN;
  if (!theData.IsNull())
  {
    FIMEMORY* aFiMem = FreeImage_OpenMemory((BYTE*)theData->ChangeData(), (DWORD)theData->Size());
    aFormat = FreeImage_GetFileTypeFromMemory(aFiMem, 0);
    if (aFiMem != nullptr)
    {
      FreeImage_CloseMemory(aFiMem);
    }
  }
  else if (theStream != nullptr)
  {
    Image_FreeImageStream aStream(theStream);
    FreeImageIO aFiIO = aStream.GetFiIO();
    aFormat = FreeImage_GetFileTypeFromHandle(&aFiIO, &aStream, 0);
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
    Image_FreeImageStream aStream(aFileIn.get());
    FreeImageIO aFiIO = aStream.GetFiIO();
    aFormat = FreeImage_GetFileTypeFromHandle(&aFiIO, &aStream, 0);
  }
  return getImageFormatName(aFormat);
#else
  (void)theData;
  (void)theStream;
  (void)theFileName;
  return "";
#endif
}

// ================================================================
// Function : SupportsReading
// Purpose  :
// ================================================================
bool Image_RWFreeImage::SupportsReading(const TCollection_AsciiString& theName) const
{
#ifdef HAVE_FREEIMAGE
  FREE_IMAGE_FORMAT aFormat = FreeImage_GetFIFFromFilename(theName.ToCString());
  return aFormat != FIF_UNKNOWN && FreeImage_FIFSupportsReading(aFormat);
#else
  (void)theName;
  return false;
#endif
}

// ================================================================
// Function : SupportsWriting
// Purpose  :
// ================================================================
bool Image_RWFreeImage::SupportsWriting(const TCollection_AsciiString& theName) const
{
#ifdef HAVE_FREEIMAGE
  FREE_IMAGE_FORMAT aFormat = FreeImage_GetFIFFromFilename(theName.ToCString());
  return aFormat != FIF_UNKNOWN && FreeImage_FIFSupportsWriting(aFormat);
#else
  (void)theName;
  return false;
#endif
}

// ================================================================
// Function : InitTrash
// Purpose  :
// ================================================================
bool Image_RWFreeImage::InitTrash(Image_PixMap& thePixmap,
                                  Image_Format thePixelFormat,
                                  const NCollection_Vec3<Standard_Size>& theDims,
                                  const Standard_Size theSizeRowBytes) const
{
  thePixmap.Clear();
  if (theDims.z() != 1)
  {
    return thePixmap.InitTrash3D(thePixelFormat, theDims, theSizeRowBytes);
  }

#ifdef HAVE_FREEIMAGE
  FREE_IMAGE_TYPE aFormatFI = convertToFreeFormat(thePixelFormat);
  int aBitsPerPixel = (int)Image_PixMap::SizePixelBytes(thePixelFormat) * 8;
  if (aFormatFI == FIT_UNKNOWN)
  {
    aFormatFI = FIT_BITMAP;
    aBitsPerPixel = 24;
  }

  FIBITMAP* anImage = FreeImage_AllocateT(aFormatFI, (int)theDims.x(), (int)theDims.y(), aBitsPerPixel);
  if (anImage == nullptr)
  {
    return false;
  }

  Handle(Image_RWFreeImage::Owner) anOwner = new Image_RWFreeImage::Owner(anImage);
  Image_Format aFormat = convertFromFreeFormat(FreeImage_GetImageType(anImage),
                                               FreeImage_GetColorType(anImage),
                                               FreeImage_GetBPP(anImage));
  if (thePixelFormat == Image_Format_BGR32
   || thePixelFormat == Image_Format_RGB32)
  {
    //FreeImage_SetTransparent(anImage, FALSE);
    aFormat = (aFormat == Image_Format_BGRA) ? Image_Format_BGR32 : Image_Format_RGB32;
  }

  NCollection_Vec3<Standard_Size> aDims(FreeImage_GetWidth(anImage), FreeImage_GetHeight(anImage), 1);
  thePixmap.InitWrapper3D(aFormat, FreeImage_GetBits(anImage), aDims, FreeImage_GetPitch(anImage), anOwner);
  thePixmap.SetTopDown(false);
  return true;
#else
  return thePixmap.InitTrash3D(thePixelFormat, theDims, theSizeRowBytes);
#endif
}

// ================================================================
// Function : Read
// Purpose  :
// ================================================================
bool Image_RWFreeImage::Read(Image_PixMap& thePixmap,
                             const Handle(NCollection_Buffer)& theData,
                             std::istream* theStream,
                             const TCollection_AsciiString& theFileName) const
{
  thePixmap.Clear();
#ifdef HAVE_FREEIMAGE
#ifdef _WIN32
  const TCollection_ExtendedString aFileNameW(theFileName);
#endif
  FREE_IMAGE_FORMAT aFIF = FIF_UNKNOWN;
  FIMEMORY* aFiMem = nullptr;
  Image_FreeImageStream aStream(theStream);
  FreeImageIO aFiIO = aStream.GetFiIO();
  if (!theData.IsNull())
  {
    aFiMem = FreeImage_OpenMemory((BYTE*)theData->ChangeData(), (DWORD)theData->Size());
    aFIF = FreeImage_GetFileTypeFromMemory(aFiMem, 0);
  }
  else if (theStream != nullptr)
  {
    aFIF = FreeImage_GetFileTypeFromHandle(&aFiIO, &aStream, 0);
  }

  if (aFIF == FIF_UNKNOWN)
  {
    // no signature? try to guess the file format from the file extension
  #ifdef _WIN32
    aFIF = FreeImage_GetFileTypeU(aFileNameW.ToWideString(), 0);
  #else
    aFIF = FreeImage_GetFIFFromFilename(theFileName.ToCString());
  #endif
  }

  if ((aFIF == FIF_UNKNOWN) || !FreeImage_FIFSupportsReading(aFIF))
  {
    Message::SendFail() << "Error: image '" << theFileName << "' has unsupported file format";
    if (aFiMem != nullptr)
    {
      FreeImage_CloseMemory(aFiMem);
    }
    return false;
  }

  int aLoadFlags = 0;
  if (aFIF == FIF_GIF)
  {
    // 'Play' the GIF to generate each frame (as 32bpp) instead of returning raw frame data when loading
    aLoadFlags = GIF_PLAYBACK;
  }
  else if (aFIF == FIF_ICO)
  {
    // convert to 32bpp and create an alpha channel from the AND-mask when loading
    aLoadFlags = ICO_MAKEALPHA;
  }

  FIBITMAP* anImage = nullptr;
  if (!theData.IsNull())
  {
    anImage = FreeImage_LoadFromMemory(aFIF, aFiMem, aLoadFlags);
    FreeImage_CloseMemory(aFiMem);
    aFiMem = nullptr;
  }
  else if (theStream != nullptr)
  {
    anImage = FreeImage_LoadFromHandle(aFIF, &aFiIO, &aStream, aLoadFlags);
  }
  else
  {
  #ifdef _WIN32
    anImage = FreeImage_LoadU(aFIF, aFileNameW.ToWideString(), aLoadFlags);
  #else
    anImage = FreeImage_Load(aFIF, theFileName.ToCString(), aLoadFlags);
  #endif
  }
  if (anImage == nullptr)
  {
    Message::SendFail() << "Error: image file '" << theFileName << "' is missing or invalid";
    return false;
  }

  Handle(Image_RWFreeImage::Owner) anOwner = new Image_RWFreeImage::Owner(anImage);

  Image_Format aFormat = Image_Format_UNKNOWN;
  if (FreeImage_GetBPP(anImage) == 1)
  {
    FIBITMAP* aTmpImage = FreeImage_ConvertTo8Bits(anImage);
    anOwner->Init(aTmpImage);
    anImage = aTmpImage;
  }
  if (anImage != nullptr)
  {
    aFormat = convertFromFreeFormat(FreeImage_GetImageType(anImage),
                                    FreeImage_GetColorType(anImage),
                                    FreeImage_GetBPP(anImage));
    if (aFormat == Image_Format_UNKNOWN)
    {
      FIBITMAP* aTmpImage = FreeImage_ConvertTo24Bits(anImage);
      anOwner->Init(aTmpImage);
      anImage = aTmpImage;
      if (anImage != nullptr)
      {
        aFormat = convertFromFreeFormat(FreeImage_GetImageType(anImage),
                                        FreeImage_GetColorType(anImage),
                                        FreeImage_GetBPP(anImage));
      }
    }
  }
  if (aFormat == Image_Format_UNKNOWN)
  {
    Message::SendFail() << "Error: image '" << theFileName << "' has unsupported pixel format";
    return false;
  }

  NCollection_Vec3<Standard_Size> aDims(FreeImage_GetWidth(anImage), FreeImage_GetHeight(anImage), 1);
  thePixmap.InitWrapper3D(aFormat, FreeImage_GetBits(anImage), aDims, FreeImage_GetPitch(anImage), anOwner);
  thePixmap.SetTopDown(false);
  return true;
#else
  (void)theData;
  (void)theStream;
  (void)theFileName;
  Message::SendFail() << "Error: image library was disabled during build (HAVE_FREEIMAGE undefined)";
  return false;
#endif
}

// ================================================================
// Function : Write
// Purpose  :
// ================================================================
bool Image_RWFreeImage::Write(Image_PixMap& thePixmap,
                              const TCollection_AsciiString& theFileName,
                              const TCollection_AsciiString& theFormat) const
{
  if (thePixmap.IsEmpty() || theFileName.IsEmpty())
  {
    return false;
  }

#ifdef HAVE_FREEIMAGE
  FREE_IMAGE_TYPE aFormatFI = convertToFreeFormat(thePixmap.Format());
  int aBitsPerPixel = (int)Image_PixMap::SizePixelBytes(thePixmap.Format()) * 8;
  if (aFormatFI == FIT_UNKNOWN)
  {
    return false;
  }

  unsigned red_mask = 0, green_mask = 0, blue_mask = 0;
  FIBITMAP* anImageToDump = FreeImage_ConvertFromRawBitsEx(FALSE, (BYTE*)thePixmap.ChangeData(), aFormatFI,
                                                           int(thePixmap.SizeX()), int(thePixmap.SizeY()),
                                                           int(thePixmap.SizeRowBytes()),
                                                           aBitsPerPixel, red_mask, green_mask, blue_mask, thePixmap.IsTopDown());
  if (anImageToDump == nullptr)
  {
    return false;
  }

  Handle(Image_RWFreeImage::Owner) anOwner = new Image_RWFreeImage::Owner(anImageToDump);

  FREE_IMAGE_FORMAT anImageFormat = FIF_UNKNOWN;
  if (!theFormat.IsEmpty())
  {
    anImageFormat = FreeImage_GetFIFFromFilename(theFormat.ToCString());
  }
#ifdef _WIN32
  const TCollection_ExtendedString aFileNameW(theFileName.ToCString(), Standard_True);
#endif
  if (anImageFormat == FIF_UNKNOWN)
  {
  #ifdef _WIN32
    anImageFormat = FreeImage_GetFIFFromFilenameU(aFileNameW.ToWideString());
  #else
    anImageFormat = FreeImage_GetFIFFromFilename(theFileName.ToCString());
  #endif
  }
  if (anImageFormat == FIF_UNKNOWN)
  {
    Message::SendFail() << "Image_RWFreeImage::Save(), unsupported image format";
    return false;
  }

  // FreeImage doesn't provide flexible format conversion API
  // so we should perform multiple conversions in some cases!
  switch (anImageFormat)
  {
    case FIF_PNG:
    case FIF_BMP:
    {
      if (thePixmap.Format() == Image_Format_BGR32
       || thePixmap.Format() == Image_Format_RGB32)
      {
        // stupid FreeImage treats reserved byte as alpha if some bytes not set to 0xFF
        for (Standard_Size aRow = 0; aRow < thePixmap.SizeY(); ++aRow)
        {
          for (Standard_Size aCol = 0; aCol < thePixmap.SizeX(); ++aCol)
          {
            thePixmap.ChangeRawValue(aRow, aCol)[3] = 0xFF;
          }
        }
      }
      else if (FreeImage_GetImageType(anOwner->LibImage) != FIT_BITMAP)
      {
        anImageToDump = FreeImage_ConvertToType(anOwner->LibImage, FIT_BITMAP);
      }
      break;
    }
    case FIF_GIF:
    {
      FIBITMAP* aTmpBitmap = anOwner->LibImage;
      if (FreeImage_GetImageType(anOwner->LibImage) != FIT_BITMAP)
      {
        aTmpBitmap = FreeImage_ConvertToType(anOwner->LibImage, FIT_BITMAP);
        if (aTmpBitmap == nullptr)
        {
          return false;
        }
      }

      if (FreeImage_GetBPP(aTmpBitmap) != 24)
      {
        FIBITMAP* aTmpBitmap24 = FreeImage_ConvertTo24Bits(aTmpBitmap);
        if (aTmpBitmap != anOwner->LibImage)
        {
          FreeImage_Unload(aTmpBitmap);
        }
        if (aTmpBitmap24 == nullptr)
        {
          return false;
        }
        aTmpBitmap = aTmpBitmap24;
      }

      // need conversion to image with palette (requires 24bit bitmap)
      anImageToDump = FreeImage_ColorQuantize(aTmpBitmap, FIQ_NNQUANT);
      if (aTmpBitmap != anOwner->LibImage)
      {
        FreeImage_Unload(aTmpBitmap);
      }
      break;
    }
    case FIF_HDR:
    case FIF_EXR:
    {
      if (thePixmap.Format() == Image_Format_Gray
       || thePixmap.Format() == Image_Format_Alpha)
      {
        anImageToDump = FreeImage_ConvertToType(anOwner->LibImage, FIT_FLOAT);
      }
      else if (thePixmap.Format() == Image_Format_RGBA
            || thePixmap.Format() == Image_Format_BGRA)
      {
        anImageToDump = FreeImage_ConvertToType(anOwner->LibImage, FIT_RGBAF);
      }
      else
      {
        FREE_IMAGE_TYPE aImgTypeFI = FreeImage_GetImageType(anOwner->LibImage);
        if (aImgTypeFI != FIT_RGBF
         && aImgTypeFI != FIT_RGBAF
         && aImgTypeFI != FIT_FLOAT)
        {
          anImageToDump = FreeImage_ConvertToType(anOwner->LibImage, FIT_RGBF);
        }
      }
      break;
    }
    default:
    {
      if (FreeImage_GetImageType(anOwner->LibImage) != FIT_BITMAP)
      {
        anImageToDump = FreeImage_ConvertToType(anOwner->LibImage, FIT_BITMAP);
        if (anImageToDump == nullptr)
        {
          return false;
        }
      }

      if (FreeImage_GetBPP(anImageToDump) != 24)
      {
        FIBITMAP* aTmpBitmap24 = FreeImage_ConvertTo24Bits(anImageToDump);
        if (anImageToDump != anOwner->LibImage)
        {
          FreeImage_Unload(anImageToDump);
        }
        if (aTmpBitmap24 == nullptr)
        {
          return false;
        }
        anImageToDump = aTmpBitmap24;
      }
      break;
    }
  }

  if (anImageToDump == nullptr)
  {
    return false;
  }

#ifdef _WIN32
  bool isSaved = (FreeImage_SaveU(anImageFormat, anImageToDump, aFileNameW.ToWideString()) != FALSE);
#else
  bool isSaved = (FreeImage_Save(anImageFormat, anImageToDump, theFileName.ToCString()) != FALSE);
#endif
  if (anImageToDump != anOwner->LibImage)
  {
    FreeImage_Unload(anImageToDump);
  }
  return isSaved;
#else
  (void)theFormat;
  Message::SendFail() << "Error: image library was disabled during build (HAVE_FREEIMAGE undefined)";
  return false;
#endif
}

// =======================================================================
// function : AdjustGamma
// purpose  :
// =======================================================================
bool Image_RWFreeImage::AdjustGamma(Image_PixMap& thePixmap,
                                    const Standard_Real theGammaCorr) const
{
  if (thePixmap.IsEmpty())
  {
    return false;
  }

#ifdef HAVE_FREEIMAGE
  FREE_IMAGE_TYPE aFormatFI = convertToFreeFormat(thePixmap.Format());
  int aBitsPerPixel = (int)Image_PixMap::SizePixelBytes(thePixmap.Format()) * 8;
  if (aFormatFI == FIT_UNKNOWN)
  {
    Message::SendFail() << "Error: gamma cannot be adjusted for external pixmap";
    return false;
  }

  unsigned red_mask = 0, green_mask = 0, blue_mask = 0;
  FIBITMAP* aTmpImage = FreeImage_ConvertFromRawBitsEx(FALSE, (BYTE*)thePixmap.ChangeData(), aFormatFI,
                                                       int(thePixmap.SizeX()), int(thePixmap.SizeY()),
                                                       int(thePixmap.SizeRowBytes()),
                                                       aBitsPerPixel, red_mask, green_mask, blue_mask, thePixmap.IsTopDown());
  if (aTmpImage == nullptr)
  {
    Message::SendFail() << "Error: gamma cannot be adjusted for external pixmap";
    return false;
  }

  Handle(Image_RWFreeImage::Owner) anOwner = new Image_RWFreeImage::Owner(aTmpImage);
  return FreeImage_AdjustGamma(aTmpImage, theGammaCorr) != FALSE;
#else
  (void)theGammaCorr;
  return false;
#endif
}
