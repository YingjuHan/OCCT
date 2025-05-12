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

#include <Image_RWPPM.hxx>

#include <Message.hxx>
#include <OSD_OpenFile.hxx>
#include <Standard_NotImplemented.hxx>

IMPLEMENT_STANDARD_RTTIEXT(Image_RWPPM, Image_RWPixMap)

// ================================================================
// Function : Image_RWPPM
// Purpose  :
// ================================================================
Image_RWPPM::Image_RWPPM()
{
  //
}

// ================================================================
// Function : ~Image_RWPPM
// Purpose  :
// ================================================================
Image_RWPPM::~Image_RWPPM()
{
  //
}

// ================================================================
// Function : SupportsReading
// Purpose  :
// ================================================================
bool Image_RWPPM::SupportsReading(const TCollection_AsciiString& theName) const
{
  (void)theName;
  return false;
}

// ================================================================
// Function : SupportsWriting
// Purpose  :
// ================================================================
bool Image_RWPPM::SupportsWriting(const TCollection_AsciiString& theName) const
{
  return theName == IMAGE_TYPE_PPM;
}

// ================================================================
// Function : Read
// Purpose  :
// ================================================================
bool Image_RWPPM::Read(Image_PixMap& ,
                       const Handle(NCollection_Buffer)& ,
                       std::istream* ,
                       const TCollection_AsciiString& ) const
{
  Message::SendFail() << "Error: Image_RWPPM image loading is not implemented";
  return false;
}

// ================================================================
// Function : Write
// Purpose  :
// ================================================================
bool Image_RWPPM::Write(Image_PixMap& thePixmap,
                        const TCollection_AsciiString& theFileName,
                        const TCollection_AsciiString& theFormat) const
{
  if (thePixmap.IsEmpty() || theFileName.IsEmpty())
  {
    return false;
  }

  const TCollection_AsciiString aFormat = !theFormat.IsEmpty() ? theFormat : CommonFormatFromName(theFileName);
  if (aFormat != IMAGE_TYPE_PPM)
  {
    Message::SendWarning() << "Warning, PPM image will be written into '" << theFileName << "'";
  }

  // Open file
  FILE* aFile = OSD_OpenFile(theFileName.ToCString(), "wb");
  if (aFile == nullptr)
  {
    return false;
  }

  // Write header
  fprintf(aFile, "P6\n%d %d\n255\n", (int )thePixmap.SizeX(), (int )thePixmap.SizeY());

  // Write pixel data
  Standard_Byte aByte;
  for (Standard_Size aRow = 0; aRow < thePixmap.SizeY(); ++aRow)
  {
    for (Standard_Size aCol = 0; aCol < thePixmap.SizeX(); ++aCol)
    {
      // extremely SLOW but universal (implemented for all supported pixel formats)
      const Quantity_ColorRGBA aColor = thePixmap.PixelColor((Standard_Integer )aCol, (Standard_Integer )aRow);
      aByte = Standard_Byte(aColor.GetRGB().Red()   * 255.0); fwrite(&aByte, 1, 1, aFile);
      aByte = Standard_Byte(aColor.GetRGB().Green() * 255.0); fwrite(&aByte, 1, 1, aFile);
      aByte = Standard_Byte(aColor.GetRGB().Blue()  * 255.0); fwrite(&aByte, 1, 1, aFile);
    }
  }

  // Close file
  fclose(aFile);
  return true;
}
