// Created on: 2012-07-18
// Created by: Kirill GAVRILOV
// Copyright (c) 2012-2014 OPEN CASCADE SAS
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

#ifndef Image_AlienPixMap_HeaderFile
#define Image_AlienPixMap_HeaderFile

#include <Image_RWPixMap.hxx>

//! Image class that support file reading/writing operations using auxiliary image library.
//! The list of supported image formats depends on OCCT building options and enabled 3rd-party dependencies:
//! - FreeImage library is handled by Image_RWFreeImage.
//!   When enabled (USE_FREEIMAGE=ON in CMake), the wide range of image formats will be supported including
//!   BMP, PNG, JPEG, TIFF, GIF, TGA, EXR, PPM.
//! - Windows Imaging Component (WIC) is handled by Image_RWWinCodec.
//!   When available (Windows platform only, built without FreeImage), the minimal set of image formats will be supported including
//!   BMP, PNG, JPEG, TIFF, GIF.
//! - Emscripten SDK is handled by Image_RWEmscripten.
//!   When available (WebAssembly target), the minimal set of image formats will be supported (for which Browser implements readers) including
//!   BMP, PNG, JPEG.
//! - When no 3rd-party image library is available, this class will be able to export images in PPM format only, with no import possible.
class Image_AlienPixMap : public Image_PixMap
{
  DEFINE_STANDARD_RTTIEXT(Image_AlienPixMap, Image_PixMap)
public:

  //! Return default rows order used by underlying image library.
  Standard_EXPORT static bool IsTopDownDefault();
public:

  //! Empty constructor.
  Standard_EXPORT Image_AlienPixMap();

  //! Destructor
  Standard_EXPORT virtual ~Image_AlienPixMap();

  //! Read image data from file.
  bool Load (const TCollection_AsciiString& theFileName)
  {
    return Load (NULL, 0, theFileName);
  }

  //! Read image data from stream.
  Standard_EXPORT bool Load (std::istream& theStream,
                             const TCollection_AsciiString& theFileName);

  //! Read image data from memory buffer.
  //! @param theData     memory pointer to read from;
  //!                    when NULL, function will attempt to open theFileName file
  //! @param theLength   memory buffer length
  //! @param theFileName optional file name
  Standard_EXPORT bool Load (const Standard_Byte* theData,
                             Standard_Size theLength,
                             const TCollection_AsciiString& theFileName);

  //! Write image data to file using file extension to determine compression format.
  Standard_EXPORT bool Save (const TCollection_AsciiString& theFileName);

  //! Initialize image plane with required dimensions.
  //! thePixelFormat - if specified pixel format doesn't supported by image library
  //!                  than nearest supported will be used instead!
  //! theSizeRowBytes - may be ignored by this class and required alignment will be used instead!
  Standard_EXPORT virtual bool InitTrash (Image_Format        thePixelFormat,
                                          const Standard_Size theSizeX,
                                          const Standard_Size theSizeY,
                                          const Standard_Size theSizeRowBytes = 0) Standard_OVERRIDE;

  //! Performs gamma correction on image.
  //! theGamma - gamma value to use; a value of 1.0 leaves the image alone
  Standard_EXPORT bool AdjustGamma (const Standard_Real theGammaCorr);

private:

  Handle(Image_RWPixMap) myLibImage;

private:

  //! Copying allowed only within Handles
  Image_AlienPixMap            (const Image_AlienPixMap& );
  Image_AlienPixMap& operator= (const Image_AlienPixMap& );

};

DEFINE_STANDARD_HANDLE(Image_AlienPixMap, Image_PixMap)

#endif // _Image_AlienPixMap_H__
