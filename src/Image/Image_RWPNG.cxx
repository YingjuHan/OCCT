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

#include <Image_RWPNG.hxx>

#include <Message.hxx>
#include <OSD_File.hxx>
#include <OSD_FileSystem.hxx>
#include <Standard_ArrayStreamBuffer.hxx>
#include <Standard_ErrorHandler.hxx>
#include <Standard_NotImplemented.hxx>

#include <memory>
#include <vector>

#ifdef HAVE_LIBPNG
#include <png.h>

// suppress warnings related to setjmp()
#if defined(_MSC_VER)
#pragma warning(disable:4611) // interaction between '_setjmp' and C++ object destruction is non-portable
#endif

//! Auxiliary PNG context.
class Image_RWPNG::PngContext
{
public:
  //! Empty constructor.
  PngContext()
  {
    //
  }

  //! Initialize PNG reader.
  bool InitReader(const Handle(NCollection_Buffer)& theBuffer,
                  std::istream* theStream,
                  const TCollection_AsciiString& theFileName)
  {
    Release();
    myIsWriter = false;

    if (!theBuffer.IsNull())
    {
      myArrBuffer.reset(new Standard_ArrayStreamBuffer((const char*)theBuffer->Data(), theBuffer->Size()));
      myIStream.reset(new std::istream(myArrBuffer.get()));
      myIStreamPtr = myIStream.get();
    }
    else if (theStream != nullptr)
    {
      myIStreamPtr = theStream;
    }
    else
    {
      const Handle(OSD_FileSystem)& aFileSystem = OSD_FileSystem::DefaultFileSystem();
      myIStream = aFileSystem->OpenIStream(theFileName, std::ios::in | std::ios::binary);
      if (myIStream.get() == nullptr)
      {
        Message::SendFail() << "Error: Unable to open file '" << theFileName << "'";
        return false;
      }
      myIStreamPtr = myIStream.get();
    }

    myPngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, this, &onPngError, &onPngWarning);
    if (myPngPtr == nullptr)
    {
      return false;
    }

    myPngInfo = png_create_info_struct(myPngPtr);
    if (myPngInfo == nullptr)
    {
      png_destroy_read_struct(&myPngPtr, (png_info**)nullptr, (png_infopp)nullptr);
      return false;
    }

    myPngEndInfo = png_create_info_struct(myPngPtr);
    if (myPngEndInfo == nullptr)
    {
      png_destroy_read_struct(&myPngPtr, &myPngInfo, (png_infopp)nullptr);
      return false;
    }

    png_set_mem_fn(myPngPtr, this, &onPngMalloc, &onPngFree);
    png_set_read_fn(myPngPtr, myIStreamPtr, &onPngReadData);
    return true;
  }

  //! Initialize PNG writer.
  bool InitWriter(const TCollection_AsciiString& theFileName)
  {
    Release();
    myIsWriter = true;
    myFileName = theFileName;

    const Handle(OSD_FileSystem)& aFileSystem = OSD_FileSystem::DefaultFileSystem();
    myOStream = aFileSystem->OpenOStream(theFileName, std::ios::out | std::ios::binary);
    if (myOStream.get() == nullptr)
    {
      Message::SendFail() << "Error: Unable to open file '" << theFileName << "'";
      return false;
    }

    myPngPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING, this, &onPngError, &onPngWarning);
    if (myPngPtr == nullptr)
    {
      return false;
    }

    myPngInfo = png_create_info_struct(myPngPtr);
    if (myPngInfo == nullptr)
    {
      png_destroy_write_struct(&myPngPtr, (png_info**)nullptr);
      return false;
    }

    png_set_mem_fn(myPngPtr, this, &onPngMalloc, &onPngFree);
    png_set_write_fn(myPngPtr, myOStream.get(), &onPngWriteData, &onPngFlushData);
    return true;
  }

  //! Destructor.
  ~PngContext()
  {
    Release();
  }

  //! Release resources.
  bool Release()
  {
    if (myPngPtr != nullptr)
    {
      if (myIsWriter)
      {
        png_destroy_write_struct(&myPngPtr, &myPngInfo);
      }
      else
      {
        png_destroy_read_struct(&myPngPtr, &myPngInfo, &myPngEndInfo);
      }
    }

    myIStream.reset();
    myArrBuffer.reset();
    myIStreamPtr = nullptr;
    if (myOStream.get() != nullptr)
    {
      myOStream->flush();
      if (!myOStream->good())
      {
        ++myNbErrors;
        Message::SendFail() << "Error: Unable to write file '" << myFileName << "'";
      }
      myOStream.reset();

      if (myNbErrors != 0)
      {
        // remove incomplete file
        OSD_Path aFilePath(myFileName);
        OSD_File(aFilePath).Remove();
      }
    }

    return myNbErrors == 0;
  }

  //! Access PNG struct.
  png_struct* PngStruct() { return myPngPtr; }

  //! Access PNG info.
  png_info* PngInfo() { return myPngInfo; }

  //! Access PNG end info.
  png_info* PngEndInfo() { return myPngEndInfo; }

  //! Return number of errors.
  int NbErrors() const { return myNbErrors; }

public:
  //! Allocation memory block.
  static void* onPngMalloc(png_struct*, png_alloc_size_t theSize)
  {
    return Standard::Allocate(theSize);
  }

  //! Free memory pointer.
  static void onPngFree(png_struct*, void* thePtr)
  {
    Standard::Free(thePtr);
  }

  //! Handle error message.
  static void onPngError(png_struct* thePng, const char* theMsg)
  {
    Message::SendFail() << "libpng error: " << (theMsg != nullptr ? theMsg : "undefined");
    PngContext* aThis = static_cast<PngContext*>(png_get_error_ptr(thePng));
    ++aThis->myNbErrors;
  }

  //! Handle warning message.
  static void onPngWarning(png_struct* thePng, const char* theMsg)
  {
    Message::SendWarning() << "libpng warning: " << (theMsg != nullptr ? theMsg : "undefined");
    PngContext* aThis = static_cast<PngContext*>(png_get_error_ptr(thePng));
    (void)aThis;
  }

  //! Read data from file stream.
  static void onPngReadData(png_struct* thePng, png_byte* theData, size_t theLength)
  {
    std::istream* aStream = static_cast<std::istream*>(png_get_io_ptr(thePng));
    aStream->read((char*)theData, theLength);
    if (!aStream->good())
    {
      png_error(thePng, "unable to read from stream");
    }
  }

  //! Write data into file stream.
  static void onPngWriteData(png_struct* thePng, png_byte* theData, size_t theLength)
  {
    std::ostream* aStream = static_cast<std::ostream*>(png_get_io_ptr(thePng));
    aStream->write((const char*)theData, theLength);
  }

  //! Flush data with output file stream.
  static void onPngFlushData(png_struct* thePng)
  {
    std::ostream* aStream = static_cast<std::ostream*>(png_get_io_ptr(thePng));
    aStream->flush();
  }

private:
  png_struct* myPngPtr = nullptr;
  png_info*   myPngInfo = nullptr;
  png_info*   myPngEndInfo = nullptr;
  int         myNbErrors = 0;
  bool        myIsWriter = false;

  TCollection_AsciiString myFileName;
  std::shared_ptr<std::ostream> myOStream;

  std::istream* myIStreamPtr = nullptr;
  std::shared_ptr<std::istream> myIStream;
  std::shared_ptr<Standard_ArrayStreamBuffer> myArrBuffer;
};
#endif

IMPLEMENT_STANDARD_RTTIEXT(Image_RWPNG, Image_RWPixMap)

// ================================================================
// Function : Image_RWPNG
// Purpose  :
// ================================================================
Image_RWPNG::Image_RWPNG()
{
  //
}

// ================================================================
// Function : IsAvailable
// Purpose  :
// ================================================================
bool Image_RWPNG::IsAvailable() const
{
#ifdef HAVE_LIBPNG
  return true;
#else
  return false;
#endif
}

// ================================================================
// Function : SupportsReading
// Purpose  :
// ================================================================
bool Image_RWPNG::SupportsReading(const TCollection_AsciiString& theName) const
{
#ifdef HAVE_LIBPNG
  return theName == IMAGE_TYPE_PNG;
#else
  (void)theName;
  return false;
#endif
}

// ================================================================
// Function : SupportsWriting
// Purpose  :
// ================================================================
bool Image_RWPNG::SupportsWriting(const TCollection_AsciiString& theName) const
{
#ifdef HAVE_LIBPNG
  return theName == IMAGE_TYPE_PNG;
#else
  (void)theName;
  return false;
#endif
}

// ================================================================
// Function : Read
// Purpose  :
// ================================================================
bool Image_RWPNG::Read(Image_PixMap& thePixmap,
                       const Handle(NCollection_Buffer)& theBuffer,
                       std::istream* theStream,
                       const TCollection_AsciiString& theFileName) const
{
  thePixmap.Clear();
#ifdef HAVE_LIBPNG
  PngContext aPngTool;
  if (!aPngTool.InitReader(theBuffer, theStream, theFileName))
  {
    return false;
  }

  if (setjmp(png_jmpbuf(aPngTool.PngStruct())))
  {
    aPngTool.Release();
    return false;
  }

  png_read_info(aPngTool.PngStruct(), aPngTool.PngInfo());
  png_uint_32 aPngWidth = 0, aPngHeight = 0;
  int aPngBitDepth = 0;
  int aPngColorType = 0;
  int aPngInterlaceType = 0;
  png_get_IHDR(aPngTool.PngStruct(), aPngTool.PngInfo(), &aPngWidth, &aPngHeight, &aPngBitDepth, &aPngColorType,
               &aPngInterlaceType, nullptr, nullptr);

  Image_Format aFormat = Image_Format_RGB;
  if (aPngColorType == PNG_COLOR_TYPE_GRAY
   || aPngColorType == PNG_COLOR_TYPE_GRAY_ALPHA)
  {
    aFormat = Image_Format_Gray;
  }
  else if (aPngColorType == PNG_COLOR_TYPE_RGB
        || aPngColorType == PNG_COLOR_TYPE_PALETTE)
  {
    aFormat = Image_Format_RGB;
  }
  else if (aPngColorType == PNG_COLOR_TYPE_RGBA)
  {
    aFormat = Image_Format_RGBA;
  }

  if (aPngColorType == PNG_COLOR_TYPE_PALETTE
   || aPngBitDepth < 8
   || png_get_valid(aPngTool.PngStruct(), aPngTool.PngInfo(), PNG_INFO_tRNS))
  {
    png_set_expand(aPngTool.PngStruct());
  }
  if (aPngBitDepth == 16)
  {
    png_set_strip_16(aPngTool.PngStruct());
  }
  if (aPngColorType == PNG_COLOR_TYPE_GRAY_ALPHA)
  {
    png_set_strip_alpha(aPngTool.PngStruct());
  }

  if (!thePixmap.InitZero(aFormat, aPngWidth, aPngHeight))
  {
    Message::SendFail() << "Error: unable to allocate PNG pixmap '" << theFileName << "'";
    return false;
  }

  if (thePixmap.Format() == Image_Format_BGR
   || thePixmap.Format() == Image_Format_BGRA
   || thePixmap.Format() == Image_Format_BGR32)
  {
    png_set_bgr(aPngTool.PngStruct());
  }
  if (thePixmap.Format() == Image_Format_Gray16
  && !Image_PixMap::IsBigEndianHost())
  {
    png_set_swap(aPngTool.PngStruct());
  }

  if (aPngInterlaceType == PNG_INTERLACE_NONE)
  {
    for (png_uint_32 aRow = 0; aRow < aPngHeight; ++aRow)
    {
      png_byte* aRowsPtr[1] = {thePixmap.ChangeRow(aRow) };
      png_read_rows(aPngTool.PngStruct(), aRowsPtr, nullptr, 1);
    }
  }
  else
  {
    std::vector<png_byte*> aRowsPtr(aPngHeight);
    for (png_uint_32 aRow = 0; aRow < aPngHeight; ++aRow)
    {
      aRowsPtr[aRow] = thePixmap.ChangeRow(aRow);
    }
    png_set_rows(aPngTool.PngStruct(), aPngTool.PngInfo(), aRowsPtr.data());
    png_read_image(aPngTool.PngStruct(), aRowsPtr.data());
  }
  png_read_end(aPngTool.PngStruct(), aPngTool.PngEndInfo());
  return aPngTool.Release();
#else
  (void)theBuffer;
  (void)theStream;
  (void)theFileName;
  Message::SendFail() << "Error: Image_RWPNG image loading is not implemented";
  return false;
#endif
}

// ================================================================
// Function : Write
// Purpose  :
// ================================================================
bool Image_RWPNG::Write(Image_PixMap& thePixmap,
                        const TCollection_AsciiString& theFileName,
                        const TCollection_AsciiString& theFormat) const
{
  if (thePixmap.IsEmpty() || theFileName.IsEmpty())
  {
    return false;
  }

#ifdef HAVE_LIBPNG
  const TCollection_AsciiString aFormat = !theFormat.IsEmpty() ? theFormat : CommonFormatFromName(theFileName);
  if (aFormat != IMAGE_TYPE_PNG)
  {
    Message::SendWarning() << "Warning, PNG image will be written into '" << theFileName << "'";
  }

  PngContext aPngTool;
  if (!aPngTool.InitWriter(theFileName))
  {
    return false;
  }

  int aPngColor = PNG_COLOR_TYPE_RGB;
  int aPngBitDepth = 8;
  Image_PixMap aTmp;
  switch (thePixmap.Format())
  {
    case Image_Format_Alpha:
    case Image_Format_Gray:
      aPngColor = PNG_COLOR_TYPE_GRAY;
      aPngBitDepth = 8;
      break;
    case Image_Format_Gray16:
      aPngColor = PNG_COLOR_TYPE_GRAY;
      aPngBitDepth = 16;
      if (!Image_PixMap::IsBigEndianHost())
      {
        png_set_swap(aPngTool.PngStruct());
      }
      break;
    case Image_Format_RGB:
    case Image_Format_RGB32:
      aPngColor = PNG_COLOR_TYPE_RGB;
      aPngBitDepth = 8;
      break;
    case Image_Format_RGBA:
      aPngColor = PNG_COLOR_TYPE_RGBA;
      aPngBitDepth = 8;
      break;
    case Image_Format_BGRA:
      aPngColor = PNG_COLOR_TYPE_RGBA;
      aPngBitDepth = 8;
      png_set_bgr(aPngTool.PngStruct()); // need to swap components
      break;
    case Image_Format_BGR:
    case Image_Format_BGR32:
      aPngColor = PNG_COLOR_TYPE_RGB;
      aPngBitDepth = 8;
      png_set_bgr(aPngTool.PngStruct()); // need to swap components
      break;
    case Image_Format_AlphaF:
    case Image_Format_GrayF:
    case Image_Format_GrayF_half:
      aPngColor = PNG_COLOR_TYPE_GRAY;
      aPngBitDepth = 8;
      aTmp.InitZero(Image_Format_Gray, thePixmap.SizeX(), 1);
      break;
    case Image_Format_RGBAF:
    case Image_Format_RGBAF_half:
      aPngColor = PNG_COLOR_TYPE_RGBA;
      aPngBitDepth = 8;
      aTmp.InitZero(Image_Format_RGBA, thePixmap.SizeX(), 1);
      break;
    case Image_Format_RGBF:
    case Image_Format_RGF_half:
      aPngColor = PNG_COLOR_TYPE_RGB;
      aPngBitDepth = 8;
      aTmp.InitZero(Image_Format_RGB, thePixmap.SizeX(), 1);
      break;
    default:
      // fallback using slowest conversion for unknown formats
      aPngColor = PNG_COLOR_TYPE_RGB;
      aPngBitDepth = 8;
      aTmp.InitZero(Image_Format_RGB, thePixmap.SizeX(), 1);
      break;
  }

  if (setjmp(png_jmpbuf(aPngTool.PngStruct())))
  {
    aPngTool.Release();
    return false;
  }

  const png_uint_32 aPngWidth  = (png_uint_32)thePixmap.SizeX();
  const png_uint_32 aPngHeight = (png_uint_32)thePixmap.SizeY();

  //png_set_compression_level(aPngTool.PngStruct(), 9); // 9 for best compression, 0 for no compression
  png_set_IHDR(aPngTool.PngStruct(), aPngTool.PngInfo(),
               aPngWidth, aPngHeight, aPngBitDepth, aPngColor,
               PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
  png_write_info(aPngTool.PngStruct(), aPngTool.PngInfo());
  if (aTmp.IsEmpty()
   && (thePixmap.Format() == Image_Format_BGR32
    || thePixmap.Format() == Image_Format_RGB32))
  {
    // strip extra component
    png_set_filler(aPngTool.PngStruct(), 0, PNG_FILLER_AFTER);
  }

  for (Standard_Size aRow = 0; aRow < thePixmap.SizeY(); ++aRow)
  {
    if (aTmp.IsEmpty())
    {
      png_write_row(aPngTool.PngStruct(), thePixmap.Row(aRow));
    }
    else
    {
      aTmp.FillRowFrom(0, thePixmap, aRow);
      png_write_row(aPngTool.PngStruct(), aTmp.Row(0));
    }
  }

  png_write_end(aPngTool.PngStruct(), aPngTool.PngInfo());
  return aPngTool.Release();
#else
  (void)theFormat;
  Message::SendFail() << "Error: image library was disabled during build (HAVE_LIBPNG undefined)";
  return false;
#endif
}
