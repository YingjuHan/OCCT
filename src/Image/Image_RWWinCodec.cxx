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

#if !defined(HAVE_FREEIMAGE) && defined(_WIN32)
  #define HAVE_WINCODEC
#endif

#if defined(HAVE_WINCODEC)
  #include <wincodec.h>
  // prevent warnings on MSVC10
  #include <Standard_WarningsDisable.hxx>
  #include <Standard_TypeDef.hxx>
  #include <Standard_WarningsRestore.hxx>
  #undef min
  #undef max
#endif

#include <Image_RWWinCodec.hxx>

#include <Image_RWPPM.hxx>
#include <Message.hxx>
#include <NCollection_Array1.hxx>
#include <Standard_NotImplemented.hxx>
#include <TCollection_ExtendedString.hxx>

namespace
{
#if defined(HAVE_WINCODEC)

  //! Return a zero GUID
  static GUID getNullGuid()
  {
    GUID aGuid = { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };
    return aGuid;
  }

  //! Sentry over IUnknown pointer.
  template<class T> class Image_ComPtr
  {
  public:
    //! Empty constructor.
    Image_ComPtr()
    : myPtr (nullptr) {}

    //! Destructor.
    ~Image_ComPtr()
    {
      Nullify();
    }

    //! Return TRUE if pointer is NULL.
    bool IsNull() const { return myPtr == nullptr; }

    //! Release the pointer.
    void Nullify()
    {
      if (myPtr != nullptr)
      {
        myPtr->Release();
        myPtr = nullptr;
      }
    }

    //! Return pointer for initialization.
    T*& ChangePtr()
    {
      Standard_ASSERT_RAISE (myPtr == nullptr, "Pointer cannot be initialized twice!");
      return myPtr;
    }

    //! Return pointer.
    T* get() { return myPtr; }

    //! Return pointer.
    T* operator->() { return get(); }

    //! Cast handle to contained type
    T& operator*() { return *get(); }

  private:
    T* myPtr;
  };

  //! Convert WIC GUID to Image_Format.
  static Image_Format convertFromWicFormat (const WICPixelFormatGUID& theFormat)
  {
    if (theFormat == GUID_WICPixelFormat32bppBGRA)
    {
      return Image_Format_BGRA;
    }
    else if (theFormat == GUID_WICPixelFormat32bppBGR)
    {
      return Image_Format_BGR32;
    }
    else if (theFormat == GUID_WICPixelFormat24bppRGB)
    {
      return Image_Format_RGB;
    }
    else if (theFormat == GUID_WICPixelFormat24bppBGR)
    {
      return Image_Format_BGR;
    }
    else if (theFormat == GUID_WICPixelFormat8bppGray)
    {
      return Image_Format_Gray;
    }
    else if (theFormat == GUID_WICPixelFormat16bppGray)
    {
      return Image_Format_Gray16;
    }
    return Image_Format_UNKNOWN;
  }

  //! Convert Image_Format to WIC GUID.
  static WICPixelFormatGUID convertToWicFormat (Image_Format theFormat)
  {
    switch (theFormat)
    {
      case Image_Format_BGRA:   return GUID_WICPixelFormat32bppBGRA;
      case Image_Format_BGR32:  return GUID_WICPixelFormat32bppBGR;
      case Image_Format_RGB:    return GUID_WICPixelFormat24bppRGB;
      case Image_Format_BGR:    return GUID_WICPixelFormat24bppBGR;
      case Image_Format_Gray:   return GUID_WICPixelFormat8bppGray;
      case Image_Format_Alpha:  return GUID_WICPixelFormat8bppGray; // GUID_WICPixelFormat8bppAlpha
      case Image_Format_Gray16: return GUID_WICPixelFormat16bppGray;
      case Image_Format_GrayF:  // GUID_WICPixelFormat32bppGrayFloat
      case Image_Format_AlphaF:
      case Image_Format_RGBAF:  // GUID_WICPixelFormat128bppRGBAFloat
      case Image_Format_RGBF:   // GUID_WICPixelFormat96bppRGBFloat
      case Image_Format_RGBA:   // GUID_WICPixelFormat32bppRGBA
      case Image_Format_RGB32:  // GUID_WICPixelFormat32bppRGB
      default:
        return getNullGuid();
    }
  }

#endif
}

IMPLEMENT_STANDARD_RTTIEXT(Image_RWWinCodec, Image_RWPixMap)

// ================================================================
// Function : Image_RWWinCodec
// Purpose  :
// ================================================================
Image_RWWinCodec::Image_RWWinCodec()
{
  //
}

// ================================================================
// Function : IsAvailable
// Purpose  :
// ================================================================
bool Image_RWWinCodec::IsAvailable() const
{
#ifdef HAVE_WINCODEC
  return true;
#else
  return false;
#endif
}

// ================================================================
// Function : SupportsReading
// Purpose  :
// ================================================================
bool Image_RWWinCodec::SupportsReading(const TCollection_AsciiString& theName) const
{
#ifdef HAVE_WINCODEC
  return theName == IMAGE_TYPE_BMP
      || theName == IMAGE_TYPE_PNG
      || theName == IMAGE_TYPE_JPG
      || theName == IMAGE_TYPE_TIFF
      || theName == IMAGE_TYPE_GIF
      || theName == IMAGE_TYPE_PPM;
#else
  (void)theName;
  return false;
#endif
}

// ================================================================
// Function : SupportsWriting
// Purpose  :
// ================================================================
bool Image_RWWinCodec::SupportsWriting(const TCollection_AsciiString& theName) const
{
#ifdef HAVE_WINCODEC
  return theName == IMAGE_TYPE_BMP
      || theName == IMAGE_TYPE_PNG
      || theName == IMAGE_TYPE_JPG
      || theName == IMAGE_TYPE_TIFF
      || theName == IMAGE_TYPE_GIF
      || theName == IMAGE_TYPE_PPM;
#else
  (void)theName;
  return false;
#endif
}

// ================================================================
// Function : InitTrash
// Purpose  :
// ================================================================
bool Image_RWWinCodec::InitTrash(Image_PixMap& thePixmap,
                                 Image_Format thePixelFormat,
                                 const NCollection_Vec3<Standard_Size>& theDims,
                                 const Standard_Size theSizeRowBytes) const
{
  thePixmap.Clear();

  Image_Format aFormat = thePixelFormat;
  switch (aFormat)
  {
    case Image_Format_RGB:
      aFormat = Image_Format_BGR;
      break;
    case Image_Format_RGB32:
      aFormat = Image_Format_BGR32;
      break;
    case Image_Format_RGBA:
      aFormat = Image_Format_BGRA;
      break;
    default:
      break;
  }

  if (!thePixmap.InitTrash3D(aFormat, theDims, theSizeRowBytes))
  {
    return false;
  }
  return true;
}

// ================================================================
// Function : Read
// Purpose  :
// ================================================================
bool Image_RWWinCodec::Read(Image_PixMap& thePixmap,
                            const Handle(NCollection_Buffer)& theData,
                            std::istream* theStream,
                            const TCollection_AsciiString& theFileName) const
{
  thePixmap.Clear();

  const TCollection_AsciiString aFormat = ProbeFormat(theData, theStream, theFileName);
  if (aFormat == IMAGE_TYPE_PPM)
  {
    Image_RWPPM aTool;
    return aTool.Read(thePixmap, theData, theStream, theFileName);
  }
#ifdef HAVE_WINCODEC
  Image_ComPtr<IWICImagingFactory> aWicImgFactory;
  CoInitializeEx(nullptr, COINIT_MULTITHREADED);
  if (CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&aWicImgFactory.ChangePtr())) != S_OK)
  {
    Message::SendFail() << "Error: cannot initialize WIC Imaging Factory";
    return false;
  }

  Image_ComPtr<IWICBitmapDecoder> aWicDecoder;
  Image_ComPtr<IWICStream> aWicStream;
  if (!theData.IsNull())
  {
    if (aWicImgFactory->CreateStream(&aWicStream.ChangePtr()) != S_OK
     || aWicStream->InitializeFromMemory((BYTE* )theData->ChangeData(), (DWORD )theData->Size()) != S_OK)
    {
      Message::SendFail() << "Error: cannot initialize WIC Stream";
      return false;
    }
    if (aWicImgFactory->CreateDecoderFromStream(aWicStream.get(), nullptr, WICDecodeMetadataCacheOnDemand, &aWicDecoder.ChangePtr()) != S_OK)
    {
      Message::SendFail() << "Error: cannot create WIC Image Decoder";
      return false;
    }
  }
  else if (theStream != nullptr)
  {
    // fallback copying stream data into transient buffer
    const std::streamoff aStart = theStream->tellg();
    theStream->seekg(0, std::ios::end);
    const Standard_Integer aLen = Standard_Integer(theStream->tellg() - aStart);
    theStream->seekg(aStart);
    if (aLen <= 0)
    {
      Message::SendFail() << "Error: empty stream";
      return false;
    }

    NCollection_Array1<Standard_Byte> aBuff(1, aLen);
    if (!theStream->read((char*)&aBuff.ChangeFirst(), aBuff.Size()))
    {
      Message::SendFail() << "Error: unable to read stream";
      return false;
    }

    if (aWicImgFactory->CreateStream(&aWicStream.ChangePtr()) != S_OK
     || aWicStream->InitializeFromMemory((BYTE* )&aBuff.ChangeFirst(), (DWORD )aBuff.Size()) != S_OK)
    {
      Message::SendFail() << "Error: cannot initialize WIC Stream";
      return false;
    }
    if (aWicImgFactory->CreateDecoderFromStream(aWicStream.get(), nullptr, WICDecodeMetadataCacheOnDemand, &aWicDecoder.ChangePtr()) != S_OK)
    {
      Message::SendFail() << "Error: cannot create WIC Image Decoder";
      return false;
    }
  }
  else
  {
    const TCollection_ExtendedString aFileNameW(theFileName);
    if (aWicImgFactory->CreateDecoderFromFilename(aFileNameW.ToWideString(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &aWicDecoder.ChangePtr()) != S_OK)
    {
      Message::SendFail() << "Error: cannot create WIC Image Decoder";
      return false;
    }
  }

  UINT aFrameCount = 0, aFrameSizeX = 0, aFrameSizeY = 0;
  WICPixelFormatGUID aWicPixelFormat = getNullGuid();
  Image_ComPtr<IWICBitmapFrameDecode> aWicFrameDecode;
  if (aWicDecoder->GetFrameCount(&aFrameCount) != S_OK
   || aFrameCount < 1
   || aWicDecoder->GetFrame(0, &aWicFrameDecode.ChangePtr()) != S_OK
   || aWicFrameDecode->GetSize(&aFrameSizeX, &aFrameSizeY) != S_OK
   || aWicFrameDecode->GetPixelFormat(&aWicPixelFormat))
  {
    Message::SendFail() << "Error: cannot get WIC Image Frame";
    return false;
  }

  Image_ComPtr<IWICFormatConverter> aWicConvertedFrame;
  Image_Format aPixelFormat = convertFromWicFormat(aWicPixelFormat);
  if (aPixelFormat == Image_Format_UNKNOWN)
  {
    aPixelFormat = Image_Format_RGB;
    if (aWicImgFactory->CreateFormatConverter(&aWicConvertedFrame.ChangePtr()) != S_OK
     || aWicConvertedFrame->Initialize(aWicFrameDecode.get(), convertToWicFormat(aPixelFormat), WICBitmapDitherTypeNone, nullptr, 0.0f, WICBitmapPaletteTypeCustom) != S_OK)
    {
      Message::SendFail ("Error: cannot convert WIC Image Frame to RGB format");
      return false;
    }
    aWicFrameDecode.Nullify();
  }

  if (!thePixmap.InitTrash(aPixelFormat, aFrameSizeX, aFrameSizeY))
  {
    Message::SendFail() << "Error: cannot initialize memory for image";
    return false;
  }

  IWICBitmapSource* aWicSrc = aWicFrameDecode.get();
  if (!aWicConvertedFrame.IsNull())
  {
    aWicSrc = aWicConvertedFrame.get();
  }

  if (thePixmap.IsTopDown())
  {
    if (aWicSrc->CopyPixels(nullptr, (UINT)thePixmap.SizeRowBytes(), (UINT)thePixmap.SizeBytes(), thePixmap.ChangeData()) != S_OK)
    {
      Message::SendFail() << "Error: cannot copy pixels from WIC Image";
      return false;
    }
  }
  else
  {
    for (Standard_Size aRow = 0; aRow < thePixmap.SizeY(); ++aRow)
    {
      WICRect aRect = {};
      aRect.X = 0;
      aRect.Y = (INT)aRow;
      aRect.Width = (INT)thePixmap.SizeX();
      aRect.Height = 1;
      if (aWicSrc->CopyPixels(&aRect, (UINT)thePixmap.SizeRowBytes(), (UINT)thePixmap.SizeRowBytes(), (BYTE*)thePixmap.Row(aRow)) != S_OK)
      {
        Message::SendFail() << "Error: cannot write pixels to WIC Frame";
        return false;
      }
    }
  }
  return true;
#else
  (void)theData;
  (void)theStream;
  (void)theFileName;
  Message::SendFail() << "Error: Image_RWWinCodec was disabled during build (HAVE_WINCODEC undefined)";
  return false;
#endif
}

// ================================================================
// Function : Write
// Purpose  :
// ================================================================
bool Image_RWWinCodec::Write(Image_PixMap& thePixmap,
                             const TCollection_AsciiString& theFileName,
                             const TCollection_AsciiString& theFormat) const
{
  if (thePixmap.IsEmpty() || theFileName.IsEmpty())
  {
    return false;
  }

  const TCollection_AsciiString aFormat = !theFormat.IsEmpty() ? theFormat : CommonFormatFromName(theFileName);
  if (aFormat.IsEmpty())
  {
    Message::SendFail() << "Error: Image_RWWinCodec, unknown image format";
    return false;
  }

  if (aFormat == IMAGE_TYPE_PPM)
  {
    Image_RWPPM aTool;
    return aTool.Write(thePixmap, theFileName, IMAGE_TYPE_PPM);
  }
#ifdef HAVE_WINCODEC
  GUID aFileFormat = getNullGuid();
  if (aFormat == IMAGE_TYPE_BMP)
  {
    aFileFormat = GUID_ContainerFormatBmp;
  }
  else if (aFormat == IMAGE_TYPE_PNG)
  {
    aFileFormat = GUID_ContainerFormatPng;
  }
  else if (aFormat == IMAGE_TYPE_JPG)
  {
    aFileFormat = GUID_ContainerFormatJpeg;
  }
  else if (aFormat == IMAGE_TYPE_TIFF)
  {
    aFileFormat = GUID_ContainerFormatTiff;
  }
  else if (aFormat == IMAGE_TYPE_GIF)
  {
    aFileFormat = GUID_ContainerFormatGif;
  }

  if (aFileFormat == getNullGuid())
  {
    Message::SendFail() << "Error: Image_RWWinCodec, unsupported image format";
    return false;
  }

  Image_ComPtr<IWICImagingFactory> aWicImgFactory;
  CoInitializeEx(nullptr, COINIT_MULTITHREADED);
  if (CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&aWicImgFactory.ChangePtr())) != S_OK)
  {
    Message::SendFail() << "Error: cannot initialize WIC Imaging Factory";
    return false;
  }

  Image_ComPtr<IWICStream> aWicFileStream;
  Image_ComPtr<IWICBitmapEncoder> aWicEncoder;
  const TCollection_ExtendedString aFileNameW(theFileName);
  if (aWicImgFactory->CreateStream(&aWicFileStream.ChangePtr()) != S_OK
   || aWicFileStream->InitializeFromFilename(aFileNameW.ToWideString(), GENERIC_WRITE) != S_OK)
  {
    Message::SendFail() << "Error: cannot create WIC File Stream";
    return false;
  }
  if (aWicImgFactory->CreateEncoder(aFileFormat, nullptr, &aWicEncoder.ChangePtr()) != S_OK
   || aWicEncoder->Initialize(aWicFileStream.get(), WICBitmapEncoderNoCache) != S_OK)
  {
    Message::SendFail() << "Error: cannot create WIC Encoder";
    return false;
  }

  const WICPixelFormatGUID aWicPixelFormat = convertToWicFormat(thePixmap.Format());
  if (aWicPixelFormat == getNullGuid())
  {
    Message::SendFail() << "Error: unsupported pixel format";
    return false;
  }

  WICPixelFormatGUID aWicPixelFormatRes = aWicPixelFormat;
  Image_ComPtr<IWICBitmapFrameEncode> aWicFrameEncode;
  if (aWicEncoder->CreateNewFrame(&aWicFrameEncode.ChangePtr(), nullptr) != S_OK
   || aWicFrameEncode->Initialize(nullptr) != S_OK
   || aWicFrameEncode->SetSize((UINT )thePixmap.SizeX(), (UINT )thePixmap.SizeY()) != S_OK
   || aWicFrameEncode->SetPixelFormat(&aWicPixelFormatRes) != S_OK)
  {
    Message::SendFail() << "Error: cannot create WIC Frame";
    return false;
  }

  if (aWicPixelFormatRes != aWicPixelFormat)
  {
    Message::SendFail() << "Error: pixel format is unsupported by image format";
    return false;
  }

  if (thePixmap.IsTopDown())
  {
    if (aWicFrameEncode->WritePixels((UINT )thePixmap.SizeY(), (UINT )thePixmap.SizeRowBytes(), (UINT )thePixmap.SizeBytes(), (BYTE* )thePixmap.Data()) != S_OK)
    {
      Message::SendFail() << "Error: cannot write pixels to WIC Frame";
      return false;
    }
  }
  else
  {
    for (Standard_Size aRow = 0; aRow < thePixmap.SizeY(); ++aRow)
    {
      if (aWicFrameEncode->WritePixels(1, (UINT )thePixmap.SizeRowBytes(), (UINT )thePixmap.SizeRowBytes(), (BYTE* )thePixmap.Row(aRow)) != S_OK)
      {
        Message::SendFail() << "Error: cannot write pixels to WIC Frame";
        return false;
      }
    }
  }

  if (aWicFrameEncode->Commit() != S_OK
   || aWicEncoder->Commit() != S_OK)
  {
    Message::SendFail() << "Error: cannot commit data to WIC Frame";
    return false;
  }
  if (aWicFileStream->Commit(STGC_DEFAULT) != S_OK)
  {
    //Message::SendFail() << "Error: cannot commit data to WIC File Stream";
    //return false;
  }
  return true;
#else
  Message::SendFail() << "Error: Image_RWWinCodec was disabled during build (HAVE_WINCODEC undefined)";
  return false;
#endif
}
