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

/*! \file image.cpp
    \brief Implementation of the class image.

*/

#include "../../base/include/exception.h"
#include "../include/image.h"
#include "../include/colorTransformsFactory.h"

namespace puntoexe
{

namespace imebra
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// image
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Create an image with the specified size, colorspace and
//  bit depth
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
ptr<handlers::dataHandlerNumericBase> image::create(
						const std::uint32_t sizeX, 
						const std::uint32_t sizeY, 
						const bitDepth depth, 
						std::wstring inputColorSpace, 
						const std::uint8_t highBit)
{
	PUNTOEXE_FUNCTION_START(L"image::create");

	lockObject lockAccess(this);

	if(sizeX == 0 || sizeY == 0)
	{
		PUNTOEXE_THROW(imageExceptionInvalidSize, "An invalid image's size has been specified");
	}

	// Normalize the color space (remove _420 & _422 and
	//  make it uppercase).
	///////////////////////////////////////////////////////////
	m_colorSpace=transforms::colorTransforms::colorTransformsFactory::normalizeColorSpace(inputColorSpace);

	// Find the number of channels to allocate
	///////////////////////////////////////////////////////////
	m_channelsNumber = transforms::colorTransforms::colorTransformsFactory::getNumberOfChannels(inputColorSpace);
	if(m_channelsNumber == 0)
	{
		PUNTOEXE_THROW(imageExceptionUnknownColorSpace, "Cannot recognize the specified color space");
	}

	// Find the datatype to use to allocate the
	//  buffer (datatypes are in Dicom standard, plus SB
	//  for signed bytes).
	///////////////////////////////////////////////////////////
	m_channelPixelSize = 0;
	std::uint8_t defaultHighBit = 0;

	std::string bufferDataType;

	switch(depth)
	{
	case depthU8:
		bufferDataType = "OB";
		defaultHighBit=7;
		break;
	case depthS8:
		bufferDataType = "SB";
		defaultHighBit=7;
		break;
	case depthU16:
		bufferDataType = "US";
		defaultHighBit=15;
		break;
	case depthS16:
		bufferDataType = "SS";
		defaultHighBit=15;
		break;
	case depthU32:
		bufferDataType = "UL";
		defaultHighBit=31;
		break;
	case depthS32:
		bufferDataType = "SL";
		defaultHighBit=31;
		break;
	default:
		PUNTOEXE_THROW(imageExceptionUnknownDepth, "Unknown depth");
	}

	// Adjust the high bit value
	///////////////////////////////////////////////////////////
	if(highBit == 0 || highBit>defaultHighBit)
		m_highBit=defaultHighBit;
	else
		m_highBit=highBit;

	// If a valid buffer with the same data type is already
	//  allocated then use it.
	///////////////////////////////////////////////////////////
	if(m_buffer == 0 || !(m_buffer->isReferencedOnce()) )
	{
		ptr<buffer> tempBuffer(new buffer(this, bufferDataType));
		m_buffer = tempBuffer;
	}

	m_sizeX = m_sizeY = 0;
	
	ptr<handlers::dataHandler> imageHandler(m_buffer->getDataHandler(true, sizeX * sizeY * (std::uint32_t)m_channelsNumber) );
	if(imageHandler != 0)
	{
		m_rowLength = m_channelsNumber*sizeX;
		
		imageHandler->setSize(m_rowLength*sizeY);
		m_channelPixelSize = imageHandler->getUnitSize();

		// Set the attributes
		///////////////////////////////////////////////////////////
		m_imageDepth=depth;
		m_sizeX=sizeX;
		m_sizeY=sizeY;
	}

	return ptr<handlers::dataHandlerNumericBase>(dynamic_cast<handlers::dataHandlerNumericBase*>(imageHandler.get()) );

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the depth
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void image::setHighBit(std::uint32_t highBit)
{
	PUNTOEXE_FUNCTION_START(L"image::setHighBit");

	lockObject lockAccess(this);

	m_highBit = highBit;

	PUNTOEXE_FUNCTION_END();
}

void image::setPalette(ptr<palette> imagePalette)
{
	PUNTOEXE_FUNCTION_START(L"image::getPalette");

	lockObject lockAccess(this);

	m_palette = imagePalette;

	PUNTOEXE_FUNCTION_END();
}



///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Retrieve an handler to the image's buffer.
// The image's components are interleaved.
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
ptr<handlers::dataHandlerNumericBase> image::getDataHandler(const bool bWrite, std::uint32_t* pRowSize, std::uint32_t* pChannelPixelSize, std::uint32_t* pChannelsNumber)
{
	PUNTOEXE_FUNCTION_START(L"image::getDataHandler");

	lockObject lockAccess(this);

	if(m_buffer == 0)
	{
		return ptr<handlers::dataHandlerNumericBase>(0);
	}

	*pRowSize=m_rowLength;
	*pChannelPixelSize=m_channelPixelSize;
	*pChannelsNumber=m_channelsNumber;

	ptr<handlers::dataHandler> imageHandler(m_buffer->getDataHandler(bWrite, m_sizeX * m_sizeY * m_channelsNumber));

	return ptr<handlers::dataHandlerNumericBase>(dynamic_cast<handlers::dataHandlerNumericBase*>(imageHandler.get()) );

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Return the bit depth
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
image::bitDepth image::getDepth()
{
	PUNTOEXE_FUNCTION_START(L"image::getDepth");

	lockObject lockAccess(this);
	return m_imageDepth;

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Return the high bit
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t image::getHighBit()
{
	PUNTOEXE_FUNCTION_START(L"image::getHighBit");

	lockObject lockAccess(this);
	return m_highBit;

	PUNTOEXE_FUNCTION_END();
}

ptr<palette> image::getPalette()
{
	PUNTOEXE_FUNCTION_START(L"image::getPalette");

	lockObject lockAccess(this);

	return m_palette;

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Returns the colorspace
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::wstring image::getColorSpace()
{
	PUNTOEXE_FUNCTION_START(L"image::getColorSpace");

	lockObject lockAccess(this);
	return m_colorSpace;

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Returns the numbero of allocated channels
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t image::getChannelsNumber()
{
	PUNTOEXE_FUNCTION_START(L"image::getChannelsNumber");

	lockObject lockAccess(this);
	return m_channelsNumber;

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Returns the image's size in pixels
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void image::getSize(std::uint32_t* pSizeX, std::uint32_t* pSizeY)
{
	PUNTOEXE_FUNCTION_START(L"image::getSize");

	lockObject lockAccess(this);

	if(pSizeX)
		*pSizeX=m_sizeX;

	if(pSizeY)
		*pSizeY=m_sizeY;

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Returns the image's size in millimeters
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void image::getSizeMm(double* pSizeMmX, double* pSizeMmY)
{
	PUNTOEXE_FUNCTION_START(L"image::getSizeMm");

	lockObject lockAccess(this);

	if(pSizeMmX)
		*pSizeMmX=m_sizeMmX;

	if(pSizeMmY)
		*pSizeMmY=m_sizeMmY;

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the image's size in millimeters
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void image::setSizeMm(const double sizeMmX, const double sizeMmY)
{
	PUNTOEXE_FUNCTION_START(L"image::setSizeMm");

	lockObject lockAccess(this);

	if(sizeMmX)
		m_sizeMmX=sizeMmX;

	if(sizeMmY)
		m_sizeMmY=sizeMmY;

	PUNTOEXE_FUNCTION_END();
}

} // namespace imebra

} // namespace puntoexe
