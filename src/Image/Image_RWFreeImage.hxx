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

#ifndef Image_RWFreeImage_HeaderFile
#define Image_RWFreeImage_HeaderFile

#include <Image_RWPixMap.hxx>

//! Class for loading/storing image pixmap from/into external file using FreeImage library.
//! Supported image formats: BMP, PPM, PNG, JPEG, TIFF, TGA, GIF, EXR and others (depending on FreeImage library building options).
class Image_RWFreeImage : public Image_RWPixMap
{
  DEFINE_STANDARD_RTTIEXT(Image_RWFreeImage, Image_RWPixMap)
public:

  //! Constructor.
  Standard_EXPORT Image_RWFreeImage();

  //! Return TRUE if the given image library is available.
  Standard_EXPORT virtual bool IsAvailable() const override;

  //! Probe image file format from file name.
  Standard_EXPORT virtual TCollection_AsciiString FormatFromName(const TCollection_AsciiString& theFileName) const override;

  //! Probe image file format from one of the inputs.
  Standard_EXPORT virtual TCollection_AsciiString ProbeFormat(const Handle(NCollection_Buffer)& theData,
                                                              std::istream* theStream,
                                                              const TCollection_AsciiString& theFileName) const override;

  //! Return TRUE if the given image format could be read.
  Standard_EXPORT virtual bool SupportsReading(const TCollection_AsciiString& theName) const override;

  //! Return TRUE if the given image format could be written.
  Standard_EXPORT virtual bool SupportsWriting(const TCollection_AsciiString& theName) const override;

  //! Read image data.
  Standard_EXPORT virtual bool Read(Image_PixMap& thePixmap,
                                    const Handle(NCollection_Buffer)& theData,
                                    std::istream* theStream,
                                    const TCollection_AsciiString& theFileName) const override;

  //! Return default rows order used by underlying image library.
  virtual bool IsTopDownDefault() const override { return false; }

  //! Initialize image plane with required dimensions.
  Standard_EXPORT virtual bool InitTrash(Image_PixMap& thePixmap,
                                         Image_Format thePixelFormat,
                                         const NCollection_Vec3<Standard_Size>& theDims,
                                         const Standard_Size theSizeRowBytes) const override;

  //! Write image data to file using file extension to determine compression format.
  Standard_EXPORT virtual bool Write(Image_PixMap& thePixmap,
                                     const TCollection_AsciiString& theFileName,
                                     const TCollection_AsciiString& theFormat) const override;

  //! Apply gamma correction to the image.
  Standard_EXPORT bool AdjustGamma (Image_PixMap& thePixmap,
                                    const Standard_Real theGammaCorr) const override;
private:
  class Owner;

};

#endif // Image_RWFreeImage_HeaderFile
