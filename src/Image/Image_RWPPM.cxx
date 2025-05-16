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
#include <FSD_BinaryFile.hxx>
#include <OSD_FileSystem.hxx>
#include <Standard_ArrayStreamBuffer.hxx>
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
  return theName == IMAGE_TYPE_PPM;
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
bool Image_RWPPM::Read(Image_PixMap& thePixmap,
                       const Handle(NCollection_Buffer)& theBuffer,
                       std::istream* theStream,
                       const TCollection_AsciiString& theFileName) const
{
  std::istream* aStream = nullptr;
  std::shared_ptr<std::istream> aFile;
  std::shared_ptr<Standard_ArrayStreamBuffer> anArrBuffer;
  if (!theBuffer.IsNull())
  {
    anArrBuffer.reset(new Standard_ArrayStreamBuffer((const char*)theBuffer->Data(), theBuffer->Size()));
    aFile.reset(new std::istream(anArrBuffer.get()));
    aStream = aFile.get();
  }
  else if (theStream != nullptr)
  {
    aStream = theStream;
  }
  else
  {
    const Handle(OSD_FileSystem)& aFileSystem = OSD_FileSystem::DefaultFileSystem();
    aFile = aFileSystem->OpenIStream(theFileName, std::ios::in | std::ios::binary);
    if (aFile.get() == nullptr)
    {
      Message::SendFail() << "Error: Unable to open file '" << theFileName << "'";
      return false;
    }
    aStream = aFile.get();
  }

  PPMFormat aFormat = PPMFormat_UNKNOWN;
  NCollection_Vec2<int> aDims;
  int aMaxVal = 0;

  std::string aMagic;
  std::getline(*aStream, aMagic);
  if (aMagic == "P1")
  {
    aFormat = PPMFormat_P1_AsciiBitmap;
  }
  else if (aMagic == "P2")
  {
    aFormat = PPMFormat_P2_AsciiGreymap;
  }
  else if (aMagic == "P3")
  {
    aFormat = PPMFormat_P3_AsciiPixmap;
  }
  else if (aMagic == "P4")
  {
    aFormat = PPMFormat_P4_RawBitmap;
  }
  else if (aMagic == "P5")
  {
    aFormat = PPMFormat_P5_RawGreymap;
  }
  else if (aMagic == "P6")
  {
    aFormat = PPMFormat_P6_RawPixmap;
  }
  else
  {
    Message::SendFail() << "Error: invalid PPM header in file '" << theFileName << "'";
    return false;
  }

  std::string aDimsStr;
  for (; std::getline(*aStream, aDimsStr);)
  {
    if (aDimsStr[0] == '#')
    {
      continue;
    }

    const char* aPos = aDimsStr.c_str();
    char* aNext = nullptr;
    aDims[0] = int(strtol(aPos, &aNext, 10));

    aPos = aNext;
    aDims[1] = int(strtol(aPos, &aNext, 10));
    break;
  }
  if (aDims.x() <= 0 || aDims.y() <= 0)
  {
    Message::SendFail() << "Error: invalid dimensions '" << aDimsStr << "' in PPM header in file '" << theFileName << "'";
    return false;
  }

  std::string aMaxValStr;
  for (; std::getline(*aStream, aMaxValStr);)
  {
    if (aMaxValStr[0] == '#')
    {
      continue;
    }

    char* aNext = nullptr;
    aMaxVal = int(strtol(aMaxValStr.c_str(), &aNext, 10));
    break;
  }
  if (aMaxVal < 2 || aMaxVal > 65535)
  {
    Message::SendFail() << "Error: invalid max value '" << aMaxValStr << "' in PPM header in file '" << theFileName << "'";
    return false;
  }

  Image_Format aPixFormat = Image_Format_UNKNOWN;
  switch (aFormat)
  {
    case PPMFormat_UNKNOWN:
    {
      break;
    }
    case PPMFormat_P1_AsciiBitmap:
    case PPMFormat_P2_AsciiGreymap:
    case PPMFormat_P3_AsciiPixmap:
    {
      Message::SendFail() << "Error: ASCII PPM format is unsupported while reading file '" << theFileName << "'";
      return false;
    }
    case PPMFormat_P4_RawBitmap:
    {
      Message::SendFail() << "Error: P4 PPM format is unsupported while reading file '" << theFileName << "'";
      return false;
    }
    case PPMFormat_P5_RawGreymap:
    {
      aPixFormat = aMaxVal > 255 ? Image_Format_Gray16 : Image_Format_Gray;
      break;
    }
    case PPMFormat_P6_RawPixmap:
    {
      aPixFormat = aMaxVal > 255 ? Image_Format_RGBF : Image_Format_RGB;
      break;
    }
  }

  if (!thePixmap.InitZero(aPixFormat, aDims.x(), aDims.y()))
  {
    Message::SendFail() << "Error: unable to allocate PPM pixmap '" << theFileName << "'";
    return false;
  }

  const Standard_Size aStride = thePixmap.SizeX() * thePixmap.SizePixelBytes();
  for (int aRow = 0; aRow < aDims.y(); ++aRow)
  {
    if (aPixFormat == Image_Format_RGBF)
    {
      // convert 16-bit RGB to floating-point values
      NCollection_Vec3<Standard_ExtCharacter> anRgb16;
      for (int aCol = 0; aCol < aDims.x(); ++aCol)
      {
        aStream->read((char*)anRgb16.ChangeData(), sizeof(anRgb16));
        if (!Image_PixMap::IsBigEndianHost())
        {
          anRgb16.r() = FSD_BinaryFile::InverseExtChar(anRgb16.r());
          anRgb16.g() = FSD_BinaryFile::InverseExtChar(anRgb16.g());
          anRgb16.b() = FSD_BinaryFile::InverseExtChar(anRgb16.b());
        }

        NCollection_Vec3<float>& aPixel = thePixmap.ChangeValue<NCollection_Vec3<float>>(aRow, aCol);
        aPixel = NCollection_Vec3<float>(anRgb16.r()) / 65535.0f;
      }
    }
    else
    {
      aStream->read((char*)thePixmap.ChangeRow(aRow), aStride);

      if (aPixFormat == Image_Format_Gray16 && !Image_PixMap::IsBigEndianHost())
      {
        // reverse bytes while reading 16-bit grayscale
        for (int aCol = 0; aCol < aDims.x(); ++aCol)
        {
          Standard_ExtCharacter& aPixel = thePixmap.ChangeValue<Standard_ExtCharacter>(aRow, aCol);
          aPixel = FSD_BinaryFile::InverseExtChar(aPixel);
        }
      }
    }
    if (!aStream->good())
    {
      Message::SendFail() << "Error: unable to read PPM data from '" << theFileName << "'";
      return false;
    }
  }
  return true;
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
  const Handle(OSD_FileSystem)& aFileSystem = OSD_FileSystem::DefaultFileSystem();
  std::shared_ptr<std::ostream> aFile = aFileSystem->OpenOStream(theFileName, std::ios::out | std::ios::binary);
  if (aFile.get() == nullptr)
  {
    Message::SendFail() << "Error: Unable to open file '" << theFileName << "'";
    return false;
  }

  // Write header
  const bool isGray = thePixmap.Format() == Image_Format_Gray
                   || thePixmap.Format() == Image_Format_GrayF
                   || thePixmap.Format() == Image_Format_GrayF_half
                   || thePixmap.Format() == Image_Format_Gray16
                   || thePixmap.Format() == Image_Format_Alpha
                   || thePixmap.Format() == Image_Format_AlphaF;
  const bool isWide = thePixmap.Format() == Image_Format_Gray16;

  TCollection_AsciiString aHeader = TCollection_AsciiString()
    + (isGray ? "P5\n" : "P6\n")
    + "# Image stored by OpenCASCADE framework\n"
    + int(thePixmap.SizeX()) + " " + int(thePixmap.SizeY()) + "\n"
    + (isWide ? "65535\n" : "255\n");
  aFile->write(aHeader.ToCString(), aHeader.Length());

  Image_PixMap aTmp;
  if (thePixmap.Format() == Image_Format_Gray
   || thePixmap.Format() == Image_Format_Alpha
   || thePixmap.Format() == Image_Format_RGB)
  {
    // could passed through without conversion
  }
  else if (isGray)
  {
    aTmp.InitZero(isWide ? Image_Format_Gray16 : Image_Format_Gray, thePixmap.SizeX(), 1);
  }
  else
  {
    aTmp.InitZero(Image_Format_RGB, thePixmap.SizeX(), 1);
  }

  // Write pixel data
  const Standard_Size aStride = !aTmp.IsEmpty()
                               ? aTmp.SizeX() * aTmp.SizePixelBytes()
                               : thePixmap.SizeX() * thePixmap.SizePixelBytes();
  for (Standard_Size aRow = 0; aRow < thePixmap.SizeY(); ++aRow)
  {
    if (!aTmp.IsEmpty())
    {
      aTmp.FillRowFrom(0, thePixmap, aRow);
      if (isWide && !Image_PixMap::IsBigEndianHost())
      {
        for (Standard_Size aCol = 0; aCol < aTmp.SizeX(); ++aCol)
        {
          Standard_ExtCharacter& aPixel = aTmp.ChangeValue<Standard_ExtCharacter>(0, aCol);
          aPixel = FSD_BinaryFile::InverseExtChar(aPixel);
        }
      }
      aFile->write((const char*)aTmp.Data(), aStride);
    }
    else
    {
      aFile->write((const char*)thePixmap.Row(aRow), aStride);
    }
  }

  aFile->flush();
  if (!aFile->good())
  {
    Message::SendFail() << "Error: Unable to write file '" << theFileName << "'";
    return false;
  }
  return true;
}
