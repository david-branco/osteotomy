/*

Imebra community build 20151130-002

Imebra: a C++ Dicom library

Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015
by Paolo Brandoli/Binarno s.p.

All rights reserved.

This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 2 as published by
 the Free Software Foundation.

This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

-------------------

If you want to use Imebra commercially then you have to buy the commercial
 license available at http://imebra.com

After you buy the commercial license then you can use Imebra according
 to the terms described in the Imebra Commercial License Version 2.
A copy of the Imebra Commercial License Version 2 is available in the
 documentation pages.

Imebra is available at http://imebra.com

The author can be contacted by email at info@binarno.com or by mail at
 the following address:
 Binarno s.p., Paolo Brandoli
 Rakuseva 14
 1000 Ljubljana
 Slovenia



*/

/*! \file image.h
    \brief Declaration of the class image.

*/

#if !defined(imebraImage_A807A3CA_FA04_44f4_85D2_C7AA2FE103C4__INCLUDED_)
#define imebraImage_A807A3CA_FA04_44f4_85D2_C7AA2FE103C4__INCLUDED_

#include "../../base/include/baseObject.h"


///////////////////////////////////////////////////////////
//
// Everything is in the namespace puntoexe::imebra
//
///////////////////////////////////////////////////////////
namespace puntoexe
{

namespace imebra
{

    namespace handlers
    {
        class dataHandlerNumericBase;
    }

class palette;
class buffer;

/// \addtogroup group_image Image data
/// \brief The class image contains the data of one DICOM image.
///
/// The image's data includes:
/// - the image's size, in pixels
/// - the image's size, in millimeters
/// - the bit depth (bytes per color channel) and high
///   bit
/// - the color palette (if available)
/// - the pixels' data
///
/// An image can be obtained from a dataSet object by
///  calling dataSet::getImage(), or it can be initialized
///  with image::create().
///
/// Images can also be allocated by the transforms
///  by calling
///  transforms::transform::allocateOutputImage().
///
/// @{


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief Represents a single image of a dicom data set.
///
/// Images are embedded into the dicom structures
///  (represented by the dataSet class), stored in
///  a compressed format.
///
/// The class image represents a decompressed raw image,
///  extracted from a dicom structure using
///  dataSet::getImage().
///
/// image objects can also be created by the
///  application and stored into a dicom structure using
///  the function dataSet::setImage().
///
/// The image and its buffer share a common lock object:
///  this means that a lock to the image object will also
///  locks the image's buffer and viceversa.
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class image : public baseObject
{
public:

	///////////////////////////////////////////////////////////
	///
	/// \brief Define the number of allocated bytes per
	///        value.
	///
	///////////////////////////////////////////////////////////
	enum bitDepth
	{
			depthU8,    ///< unsigned integer, 1 byte
			depthS8,    ///< signed integer, 1 byte
			depthU16,   ///< unsigned integer, 2 bytes
			depthS16,   ///< signed integer, 2 bytes
			depthU32,   ///< unsigned integer, 4 bytes
			depthS32,   ///< signed integer, 4 bytes
            endOfDepths

	};

	// Constructor
	///////////////////////////////////////////////////////////
	image():
			m_rowLength(0),
			m_channelPixelSize(0),
			m_channelsNumber(0),
			m_imageDepth(depthU8),
			m_highBit(0),
			m_sizeX(0),
			m_sizeY(0),
			m_sizeMmX(0),
			m_sizeMmY(0){}


	/// \brief Create the image.
	///
	/// An image with the specified size (in pixels), bit depth
	///  and color space is allocated.
	/// The number of channels to allocate is automatically
	///  calculated using the colorSpace parameter.
	///
	/// @param sizeX    the image's width, in pixels.
	/// @param sizeY    the image's height, in pixels.
	/// @param depth    the size of a single color's component.
	/// @param colorSpace The color space as defined by the
	///                 DICOM standard.
	///                 Valid colorspace are:
	///                 - "RGB"
	///                 - "YBR_FULL"
	///                 - "YBR_PARTIAL"
	///                 - "YBR_RCT" (Not yet supported)
	///                 - "YBR_ICT" (Not yet supported)
	///                 - "PALETTE COLOR"
	///                 - "MONOCHROME2"
	///                 - "MONOCHROME1"
	/// @param highBit  the highest bit used for integer
	///                  values.
	/// @return         the data handler containing the image's
	///                  data
	///
	///////////////////////////////////////////////////////////
	ptr<handlers::dataHandlerNumericBase> create(
		const std::uint32_t sizeX,
		const std::uint32_t sizeY,
		const bitDepth depth,
		std::wstring colorSpace,
		const std::uint8_t  highBit);

	/// \brief Set the high bit.
	///
	/// @param highBit       the image's high bit
	///
	///////////////////////////////////////////////////////////
	void setHighBit(std::uint32_t highBit);

	/// \brief Set the palette for the image
	///
	/// @param imagePalette  the palette used in the image
	///
	///////////////////////////////////////////////////////////
	void setPalette(ptr<palette> imagePalette);

	/// \brief Retrieve the image's size, in millimeters.
	///
	/// The image's size in millimeters is automatically read
	///  from the dicom structure or can be set using
	///  setSizeMm().
	///
	/// @param pSizeX a pointer to the variable to fill with
	///               the image's width (in millimeters).
	/// @param pSizeY a pointer to the variable to fill with
	///               the image's height (in millimeters).
	///////////////////////////////////////////////////////////
	void getSizeMm(double* pSizeX, double* pSizeY);

	/// \brief Set the image's size, in millimeters.
	///
	/// @param sizeX the new image's width, in millimeters.
	/// @param sizeY the new image's height, in millimeters.
	///
	///////////////////////////////////////////////////////////
	void setSizeMm(const double sizeX, const double sizeY);

	/// \brief Get the image's size, in pixels.
	///
	/// @param pSizeX a pointer to the variable to fill with
	///               the image's width (in pixels).
	/// @param pSizeY a pointer to the variable to fill with
	///               the image's height (in pixels).
	///
	///////////////////////////////////////////////////////////
	void getSize(std::uint32_t* pSizeX, std::uint32_t* pSizeY);

	/// \brief Retrieve a data handler for managing the
	///        image's buffer
	///
	/// The retrieved data handler gives access to the image's
	///  buffer.
	/// The image's buffer stores the data in the following
	///  format:
	/// - when multiple channels are present, then the channels
	///   are ALWAYS interleaved
	/// - the channels are NEVER subsampled or oversampled.
	///   The subsampling/oversampling is done by the codecs
	///   when the image is stored or loaded from the dicom
	///   structure.
	/// - the first stored value represent the first channel of
	///   the top/left pixel.
	/// - each row is stored countiguously, from the top to the
	///   bottom.
	///
	/// @param bWrite   true if the application wants to write
	///                 into the buffer, false otherwise.
	/// @param pRowSize the function will fill the variable
	///                 pointed by this parameter with
	///                 the size of a single row, in bytes.
	/// @param pChannelPixelSize the function will fill the
	///                 variable pointed by this parameter with
	///                 the size of a single pixel,
	///                 in bytes.
	/// @param pChannelsNumber  the function will fill the
	///                 variable pointed by this parameter with
	///                 the number of channels per pixel.
	/// @return a pointer to the data handler for the image's
	///         buffer.
	///
	///////////////////////////////////////////////////////////
	ptr<handlers::dataHandlerNumericBase> getDataHandler(
		const bool bWrite,
		std::uint32_t* pRowSize,
		std::uint32_t* pChannelPixelSize,
		std::uint32_t* pChannelsNumber);

	/// \brief Get the image's color space (DICOM standard)
	///
	/// @return a string with the image's color space
	///
	///////////////////////////////////////////////////////////
	std::wstring getColorSpace();

	/// \brief Get the number of allocated channels.
	///
	/// @return the number of color channels in the image
	///
	///////////////////////////////////////////////////////////
	std::uint32_t getChannelsNumber();

	/// \brief Get the image's bit depth.
	///
	/// The bit depth indicates the number of bits used to
	///  store every single value.
	///
	/// @return the bit depth.
	///////////////////////////////////////////////////////////
	bitDepth getDepth();

	/// \brief Get the high bit.
	///
	/// @return the image's high bit
	///
	///////////////////////////////////////////////////////////
	std::uint32_t getHighBit();

        ptr<palette> getPalette();


protected:
	// Image's buffer
	///////////////////////////////////////////////////////////
	ptr<buffer> m_buffer;

	// Lenght of a buffer's row (in bytes)
	///////////////////////////////////////////////////////////
	std::uint32_t m_rowLength;

	// Length of a pixel's component (in bytes)
	///////////////////////////////////////////////////////////
	std::uint32_t m_channelPixelSize;

	// Number of channels
	///////////////////////////////////////////////////////////
	std::uint32_t  m_channelsNumber;

	// Color space
	///////////////////////////////////////////////////////////
	std::wstring m_colorSpace;

	// Depth (enum)
	///////////////////////////////////////////////////////////
	bitDepth  m_imageDepth;

	// High bit (not valid in float mode)
	///////////////////////////////////////////////////////////
	std::uint32_t m_highBit;

	// Image's size in pixels
	///////////////////////////////////////////////////////////
	std::uint32_t m_sizeX;
	std::uint32_t m_sizeY;

	// Image's size in millimeters
	///////////////////////////////////////////////////////////
	double m_sizeMmX;
	double m_sizeMmY;

	// Image's lut (only if the colorspace is PALETTECOLOR
	///////////////////////////////////////////////////////////
        ptr<palette> m_palette;

};


///////////////////////////////////////////////////////////
/// \brief This is the base class for the exceptions thrown
///         by the image class.
///
///////////////////////////////////////////////////////////
class imageException: public std::runtime_error
{
public:
	/// \brief Build a codec exception
	///
	/// @param message the message to store into the exception
	///
	///////////////////////////////////////////////////////////
	imageException(const std::string& message): std::runtime_error(message){}
};

///////////////////////////////////////////////////////////
/// \brief This exception is thrown when an unknown depth
///         is specified as a parameter.
///
///////////////////////////////////////////////////////////
class imageExceptionUnknownDepth: public imageException
{
public:
	imageExceptionUnknownDepth(const std::string& message): imageException(message){}
};

///////////////////////////////////////////////////////////
/// \brief This exception is thrown when an unknown color
///         space is specified in the function create().
///
///////////////////////////////////////////////////////////
class imageExceptionUnknownColorSpace: public imageException
{
public:
	imageExceptionUnknownColorSpace(const std::string& message): imageException(message){}
};

///////////////////////////////////////////////////////////
/// \brief This exception is thrown when an invalid size
///         in pixels is specified in the function
///         create().
///
///////////////////////////////////////////////////////////
class imageExceptionInvalidSize: public imageException
{
public:
	imageExceptionInvalidSize(const std::string& message): imageException(message){}
};

/// @}

} // namespace imebra

} // namespace puntoexe

#endif // !defined(imebraImage_A807A3CA_FA04_44f4_85D2_C7AA2FE103C4__INCLUDED_)
