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

#ifndef Image_RWPPM_HeaderFile
#define Image_RWPPM_HeaderFile

#include <Image_RWPixMap.hxx>

//! Class for storing image pixmap into PPM file. The reading functionality is not implemented.
class Image_RWPPM : public Image_RWPixMap
{
  DEFINE_STANDARD_RTTIEXT(Image_RWPPM, Image_RWPixMap)
public:

  //! Constructor.
  Standard_EXPORT Image_RWPPM();

  //! Destructor
  Standard_EXPORT virtual ~Image_RWPPM();

  //! Return TRUE if the given image format could be read.
  Standard_EXPORT virtual bool SupportsReading(const TCollection_AsciiString& theName) const override;

  //! Return TRUE if the given image format could be written.
  Standard_EXPORT virtual bool SupportsWriting(const TCollection_AsciiString& theName) const override;

  //! Read image data from memory buffer or from file.
  Standard_EXPORT virtual bool Read(Image_PixMap& thePixmap,
                                    const Handle(NCollection_Buffer)& theData,
                                    std::istream* theStream,
                                    const TCollection_AsciiString& theFileName) const override;

  //! Write image data to file using file extension to determine compression format.
  Standard_EXPORT virtual bool Write(Image_PixMap& thePixmap,
                                     const TCollection_AsciiString& theFileName,
                                     const TCollection_AsciiString& theFormat) const override;

private:

  //! PPM format variations.
  enum PPMFormat
  {
    PPMFormat_UNKNOWN = 0,
    PPMFormat_P1_AsciiBitmap = 1, //!< ascii bitmap
    PPMFormat_P2_AsciiGreymap,    //!< ascii greymap
    PPMFormat_P3_AsciiPixmap,     //!< ascii pixmap
    PPMFormat_P4_RawBitmap,       //!< raw bitmap
    PPMFormat_P5_RawGreymap,      //!< raw greymap
    PPMFormat_P6_RawPixmap,       //!< raw pixmap
  };

};

#endif // Image_RWPPM_HeaderFile
