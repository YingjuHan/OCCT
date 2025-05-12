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

#ifndef Image_RWPixMap_HeaderFile
#define Image_RWPixMap_HeaderFile

#include <Image_PixMap.hxx>
#include <NCollection_Buffer.hxx>
#include <TCollection_AsciiString.hxx>

//! Abstract interface for loading/storing image pixmap from/into external file.
class Image_RWPixMap : public Standard_Transient
{
  DEFINE_STANDARD_RTTIEXT(Image_RWPixMap, Standard_Transient)
public:

  //! PNG image format identifier.
  static const char* IMAGE_TYPE_PNG;
  //! JPEG image format identifier.
  static const char* IMAGE_TYPE_JPG;
  //! GIF image format identifier.
  static const char* IMAGE_TYPE_GIF;
  //! TIFF image format identifier.
  static const char* IMAGE_TYPE_TIFF;
  //! BMP image format identifier.
  static const char* IMAGE_TYPE_BMP;
  //! WebP image format identifier.
  static const char* IMAGE_TYPE_WEBP;
  //! DDS image format identifier.
  static const char* IMAGE_TYPE_DDS;
  //! PPM image format identifier.
  static const char* IMAGE_TYPE_PPM;
  //! OpenEXR image format identifier.
  static const char* IMAGE_TYPE_EXR;
  //! HDR image format identifier.
  static const char* IMAGE_TYPE_HDR;
  //! PSD image format identifier.
  static const char* IMAGE_TYPE_PSD;
  //! ICO image format identifier.
  static const char* IMAGE_TYPE_ICO;

public:
  //! Default image library selector.
  Standard_EXPORT static const Handle(Image_RWPixMap)& DefaultSelector();

  //! Probe image file format from file name.
  Standard_EXPORT static TCollection_AsciiString CommonFormatFromName(const TCollection_AsciiString& theFileName);

  //! Probe image file format from one of the inputs in the following order:
  //! from @p theData data buffer when not NULL,
  //! or from @p theStream input stream when not NULL,
  //! or from @p theFileName file name to read.
  //! @param[in] theData     image buffer
  //! @param[in] theStream   input stream
  //! @param[in] theFileName file name
  Standard_EXPORT static TCollection_AsciiString CommonProbeFormat(const Handle(NCollection_Buffer)& theData,
                                                                   std::istream* theStream,
                                                                   const TCollection_AsciiString& theFileName);

public:

  //! Destructor
  Standard_EXPORT virtual ~Image_RWPixMap();

  //! Probe image file format from file name.
  virtual TCollection_AsciiString FormatFromName(const TCollection_AsciiString& theFileName) const
  {
    return CommonFormatFromName(theFileName);
  }

  //! Probe image file format from one of the inputs in the following order:
  //! from @p theData data buffer when not NULL,
  //! or from @p theStream input stream when not NULL,
  //! or from @p theFileName file name to read.
  //! @param[in] theData     image buffer
  //! @param[in] theStream   input stream
  //! @param[in] theFileName file name
  virtual TCollection_AsciiString ProbeFormat(const Handle(NCollection_Buffer)& theData,
                                              std::istream* theStream,
                                              const TCollection_AsciiString& theFileName) const
  {
    return CommonProbeFormat(theData, theStream, theFileName);
  }

  //! Return TRUE if the given image library is available.
  virtual bool IsAvailable() const { return true; }

  //! Return TRUE if the given image format could be read.
  virtual bool SupportsReading(const TCollection_AsciiString& theName) const = 0;

  //! Return TRUE if the given image format could be written.
  virtual bool SupportsWriting(const TCollection_AsciiString& theName) const = 0;

  //! Read image data from one of the inputs in the following order:
  //! from @p theData data buffer when not NULL,
  //! or from @p theStream input stream when not NULL,
  //! or from @p theFileName file name.
  //! @param[in,out] thePixmap   image to read into
  //! @param[in]     theData     image buffer
  //! @param[in]     theStream   input stream
  //! @param[in]     theFileName file name
  virtual bool Read(Image_PixMap& thePixmap,
                    const Handle(NCollection_Buffer)& theData,
                    std::istream* theStream,
                    const TCollection_AsciiString& theFileName) const = 0;

  //! Return default rows order used by underlying image library.
  virtual bool IsTopDownDefault() const { return true; }

  //! Initialize image plane with required dimensions.
  //! @param[in,out] thePixmap       image to initialize
  //! @param[in]     thePixelFormat  desired pixel format
  //!                               (nearest supported by library format will be actually used)
  //! @param[in]     theDims         image dimensions
  //! @param[in]     theSizeRowBytes desired stride in bytes
  //!                               (nearest alignment supported by library format will be actually used)
  Standard_EXPORT virtual bool InitTrash(Image_PixMap& thePixmap,
                                         Image_Format thePixelFormat,
                                         const NCollection_Vec3<Standard_Size>& theDims,
                                         const Standard_Size theSizeRowBytes) const;

  //! Write image data to file using file extension to determine compression format.
  //! @param[in] thePixmap image to save, initialized via Image_RWPixMap::InitTrash();
  //!                      if image was initialized by other means,
  //!                      the saving procedure may fail or imply additional conversion steps
  //! @param[in] theFileName output file name
  //! @param[in] theFormat   optional image format
  virtual bool Write(Image_PixMap& thePixmap,
                     const TCollection_AsciiString& theFileName,
                     const TCollection_AsciiString& theFormat) const = 0;

  //! Apply gamma correction to the image.
  //! @param[in,out] thePixmap image to modify
  //! @param[in]     theGamma  gamma value to use; a value of 1.0 leaves the image alone
  Standard_EXPORT virtual bool AdjustGamma(Image_PixMap& thePixmap,
                                           const Standard_Real theGammaCorr) const;

};

#endif // Image_RWPixMap_HeaderFile
